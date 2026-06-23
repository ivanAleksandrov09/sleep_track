#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "sleep_session.h"

// All data the display needs in one place
struct DisplayData {
    // Current sensor
    float temp;
    float humidity;

    // Time
    unsigned long elapsedMs;   // sleep timer
    unsigned long epochMs;     // millis() at boot offset — or RTC if you add one

    // State
    SleepState state;
    const char* phaseName;

    // History (pointer to array of HISTORY_COUNT)
    const SleepSession* history;

    // For confirm screen
    int lastScore;
};

class DisplayManager {
public:
    DisplayManager();
    bool begin();  // returns false if display init fails

    // Master render — call every loop()
    void render(const DisplayData& data);

private:
    Adafruit_SSD1306 _display;

    // Sub-renderers
    void _renderIdle(const DisplayData& data);
    void _renderTracking(const DisplayData& data);
    void _renderConfirmStop(const DisplayData& data);
    void _renderStopped(const DisplayData& data);

    // Reusable drawing helpers
    void _drawHistory(const SleepSession* history, int x, int y, int w, int h);
    void _drawCurrentStats(const DisplayData& data, int x, int y, int w, int h);
    void _drawTimer(unsigned long ms, int x, int y);
    void _drawPhase(const char* phase, int x, int y);

    // Format helpers
    void _formatDuration(unsigned long ms, char* buf, size_t bufLen);
};
