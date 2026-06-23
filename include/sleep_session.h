#pragma once
#include <Arduino.h>
#include "config.h"

enum SleepState
{
    STATE_IDLE,
    STATE_TRACKING,
    STATE_CONFIRM_STOP,
    STATE_STOPPED
};

enum SleepPhase
{
    PHASE_AWAKE,
    PHASE_LIGHT,
    PHASE_DEEP,
    PHASE_REM
};

struct SleepSession
{
    unsigned long startTime;  // millis() when sleep began
    unsigned long durationMs; // filled on stop
    float avgTemp;
    float avgHumidity;
    int score;  // 0–100
    bool valid; // false = empty slot
};

class SleepTracker
{
public:
    SleepTracker();

    void begin();
    void update();
    void startSleep();
    void stopSleep(float avgTemp, float avgHumidity);
    void cancelStop();

    SleepState getState() const { return _state; }
    unsigned long getElapsedMs() const;
    SleepPhase getCurrentPhase() const;
    const char *getPhaseName() const;

    // History
    SleepSession getHistory(int index) const; // 0 = most recent

    // Score calculation (public so you can preview it)
    static int calculateScore(unsigned long durationMs, float avgTemp, float avgHumidity);

private:
    SleepState _state;
    unsigned long _sleepStartMs;
    unsigned long _confirmStartMs;
    unsigned long _stoppedStartMs;

    SleepSession _history[HISTORY_COUNT];

    void _shiftHistory(SleepSession newSession);
    void _saveHistory();
    void _loadHistory();
};
