#include <Wire.h>
#include "config.h"
#include "sleep_session.h"
#include "sensor.h"
#include "display_manager.h"

// ─── Globals ──────────────────────────────────────────────────────────────────

SleepTracker  tracker;
SensorManager sensor;
DisplayManager display;

// Button state
int lastSBState = LOW;
int lastRSState = LOW;
unsigned long lastSBPress = 0;
unsigned long lastRSPress = 0;

// ─── Setup ────────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);

    if (!display.begin()) {
        Serial.println(F("Display init failed"));
        for (;;);
    }

    sensor.begin();
    tracker.begin();

    pinMode(BUTTON_STOPBEGIN, INPUT);
    pinMode(BUTTON_RESET, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
}

// ─── Loop ─────────────────────────────────────────────────────────────────────

void loop() {
    sensor.update();

    handleButtons();
    checkConfirmTimeout();

    // Build display data
    DisplayData data;
    data.temp       = sensor.getTemp();
    data.humidity   = sensor.getHumidity();
    data.elapsedMs  = tracker.getElapsedMs();
    data.epochMs    = millis();
    data.state      = tracker.getState();
    data.phaseName  = tracker.getPhaseName();
    data.lastScore  = tracker.getHistory(0).score;

    // Fill history array
    static SleepSession historyBuf[HISTORY_COUNT];
    for (int i = 0; i < HISTORY_COUNT; i++) {
        historyBuf[i] = tracker.getHistory(i);
    }
    data.history = historyBuf;

    display.render(data);

    delay(100); // ~10fps is plenty for this UI
}

// ─── Button handling ──────────────────────────────────────────────────────────

void handleButtons() {
    int sbState = digitalRead(BUTTON_STOPBEGIN);
    int rsState = digitalRead(BUTTON_RESET);

    // STOPBEGIN button
    if (sbState == HIGH && lastSBState == LOW &&
        millis() - lastSBPress > BTN_DEBOUNCE_MS)
    {
        lastSBPress = millis();
        onStopBeginPress();
    }

    // RESET button — only useful in IDLE to clear history (optional)
    if (rsState == HIGH && lastRSState == LOW &&
        millis() - lastRSPress > BTN_DEBOUNCE_MS)
    {
        lastRSPress = millis();
        onResetPress();
    }

    lastSBState = sbState;
    lastRSState = rsState;
}

void onStopBeginPress() {
    SleepState state = tracker.getState();

    if (state == STATE_IDLE || state == STATE_STOPPED) {
        sensor.resetAverages();
        tracker.startSleep();
    }
    else if (state == STATE_TRACKING) {
        tracker.stopSleep(sensor.getAvgTemp(), sensor.getAvgHumidity());
        // stopSleep with STATE_TRACKING moves to STATE_CONFIRM_STOP
    }
    else if (state == STATE_CONFIRM_STOP) {
        tracker.stopSleep(sensor.getAvgTemp(), sensor.getAvgHumidity());
        // Second call finalizes — buzzer feedback
        beep(200);
    }
}

void onResetPress() {
    // Cancel a pending confirm-stop
    tracker.cancelStop();
}

// If confirm window expires, cancel stop
void checkConfirmTimeout() {
    // The tracker doesn't hold the confirm timestamp publicly,
    // so we track it here and call cancelStop if needed.
    static unsigned long confirmStarted = 0;

    if (tracker.getState() == STATE_CONFIRM_STOP) {
        if (confirmStarted == 0) confirmStarted = millis();
        if (millis() - confirmStarted > BTN_CONFIRM_WINDOW_MS) {
            tracker.cancelStop();
            confirmStarted = 0;
        }
    } else {
        confirmStarted = 0;
    }
}

// ─── Buzzer ───────────────────────────────────────────────────────────────────

void beep(int durationMs) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(durationMs);
    digitalWrite(BUZZER_PIN, LOW);
}
