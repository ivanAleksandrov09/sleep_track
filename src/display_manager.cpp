#include "display_manager.h"
#include <stdio.h>

DisplayManager::DisplayManager()
    : _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1)
{}

bool DisplayManager::begin() {
    if (!_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
        return false;
    }
    _display.clearDisplay();
    _display.display();
    return true;
}

// ─── Master render ────────────────────────────────────────────────────────────

void DisplayManager::render(const DisplayData& data) {
    _display.clearDisplay();

    switch (data.state) {
        case STATE_IDLE:         _renderIdle(data);        break;
        case STATE_TRACKING:     _renderTracking(data);    break;
        case STATE_CONFIRM_STOP: _renderConfirmStop(data); break;
        case STATE_STOPPED:      _renderStopped(data);     break;
    }

    _display.display();
}

// ─── Screen layouts ───────────────────────────────────────────────────────────

void DisplayManager::_renderIdle(const DisplayData& data) {
    // Left half: history, Right half: current temp/humidity
    _drawHistory(data.history, 0, 0, 62, SCREEN_HEIGHT);

    // Vertical divider
    _display.drawFastVLine(63, 0, SCREEN_HEIGHT, SSD1306_WHITE);

    _drawCurrentStats(data, 65, 0, SCREEN_WIDTH - 65, SCREEN_HEIGHT);
}

void DisplayManager::_renderTracking(const DisplayData& data) {
    // Top: timer + phase
    _drawTimer(data.elapsedMs, 0, 0);
    _drawPhase(data.phaseName, 80, 0);

    // Divider
    _display.drawFastHLine(0, 16, SCREEN_WIDTH, SSD1306_WHITE);

    // Bottom: temp + humidity
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);

    char buf[24];
    snprintf(buf, sizeof(buf), "%.1fC  %.0f%%", data.temp, data.humidity);
    _display.setCursor(0, 20);
    _display.print(buf);
}

void DisplayManager::_renderConfirmStop(const DisplayData& data) {
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);

    _display.setCursor(0, 0);
    _display.println("Stop sleep?");

    char dur[16];
    _formatDuration(data.elapsedMs, dur, sizeof(dur));
    _display.setCursor(0, 16);
    _display.print("Duration: ");
    _display.println(dur);

    _display.setCursor(0, 40);
    _display.println("Press again to");
    _display.println("confirm");
}

void DisplayManager::_renderStopped(const DisplayData& data) {
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);

    _display.setCursor(20, 4);
    _display.println("Sleep saved!");

    _display.setTextSize(2);
    _display.setCursor(30, 24);
    _display.print(data.lastScore);
    _display.println("/100");
}

// ─── Drawing helpers ──────────────────────────────────────────────────────────

void DisplayManager::_drawHistory(const SleepSession* history, int x, int y, int w, int h) {
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(x, y);
    _display.println("Nights:");

    for (int i = 0; i < HISTORY_COUNT; i++) {
        int rowY = y + 10 + i * 13;
        if (!history[i].valid) {
            _display.setCursor(x, rowY);
            _display.println("--");
            continue;
        }
        char buf[16];
        snprintf(buf, sizeof(buf), "%dh %d/100",
            (int)(history[i].durationMs / 3600000UL),
            history[i].score);
        _display.setCursor(x, rowY);
        _display.print(buf);
    }
}

void DisplayManager::_drawCurrentStats(const DisplayData& data, int x, int y, int w, int h) {
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);

    char buf[16];

    snprintf(buf, sizeof(buf), "%.1f C", data.temp);
    _display.setCursor(x, y + 10);
    _display.print(buf);

    snprintf(buf, sizeof(buf), "%.0f%%", data.humidity);
    _display.setCursor(x, y + 28);
    _display.print(buf);
}

void DisplayManager::_drawTimer(unsigned long ms, int x, int y) {
    char buf[12];
    _formatDuration(ms, buf, sizeof(buf));
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(x, y);
    _display.print(buf);
}

void DisplayManager::_drawPhase(const char* phase, int x, int y) {
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(x, y);
    _display.print(phase);
}

void DisplayManager::_formatDuration(unsigned long ms, char* buf, size_t bufLen) {
    unsigned long totalSec = ms / 1000;
    unsigned long h = totalSec / 3600;
    unsigned long m = (totalSec % 3600) / 60;
    unsigned long s = totalSec % 60;
    snprintf(buf, bufLen, "%02lu:%02lu:%02lu", h, m, s);
}
