#include "sleep_session.h"
#include <Preferences.h>
#include <math.h>

static Preferences prefs;

static unsigned long scaleSleepTime(unsigned long elapsedMs)
{
#if DEBUG_MODE
    unsigned long long scaled = (unsigned long long)elapsedMs * DEBUG_SLEEP_TIME_SCALE;
    return scaled > 0xFFFFFFFFULL ? 0xFFFFFFFFUL : (unsigned long)scaled;
#else
    return elapsedMs;
#endif
}

SleepTracker::SleepTracker()
    : _state(STATE_IDLE), _sleepStartMs(0), _confirmStartMs(0), _stoppedStartMs(0)
{
    for (int i = 0; i < HISTORY_COUNT; i++)
    {
        _history[i].valid = false;
    }
}

void SleepTracker::begin()
{
    _loadHistory();
}

void SleepTracker::update()
{
    if (_state == STATE_STOPPED && millis() - _stoppedStartMs > STOPPED_SCREEN_MS)
    {
        _state = STATE_IDLE;
        _stoppedStartMs = 0;
    }
}

void SleepTracker::startSleep()
{
    _sleepStartMs = millis();
    _state = STATE_TRACKING;
}

void SleepTracker::stopSleep(float avgTemp, float avgHumidity)
{
    if (_state != STATE_TRACKING && _state != STATE_CONFIRM_STOP)
        return;

    if (_state == STATE_TRACKING)
    {
        // First press: enter confirm state
        _confirmStartMs = millis();
        _state = STATE_CONFIRM_STOP;
        return;
    }

    // Second press (or confirm window still open): finalize
    unsigned long durationMs = scaleSleepTime(millis() - _sleepStartMs);
    int score = calculateScore(durationMs, avgTemp, avgHumidity);

    SleepSession session;
    session.startTime = _sleepStartMs;
    session.durationMs = durationMs;
    session.avgTemp = avgTemp;
    session.avgHumidity = avgHumidity;
    session.score = score;
    session.valid = true;

    _shiftHistory(session);
    _saveHistory();

    _stoppedStartMs = millis();
    _state = STATE_STOPPED;
}

void SleepTracker::cancelStop()
{
    if (_state == STATE_CONFIRM_STOP)
    {
        _state = STATE_TRACKING;
    }
}

unsigned long SleepTracker::getElapsedMs() const
{
    if (_state == STATE_IDLE || _state == STATE_STOPPED)
        return 0;
    return scaleSleepTime(millis() - _sleepStartMs);
}

SleepPhase SleepTracker::getCurrentPhase() const
{
    unsigned long mins = getElapsedMs() / 60000UL;
    // Simple time-based model cycling every 90 min
    unsigned long cycle = mins % 90;
    if (mins < 5)
        return PHASE_AWAKE;
    if (cycle < 30)
        return PHASE_LIGHT;
    if (cycle < 60)
        return PHASE_DEEP;
    return PHASE_REM;
}

const char *SleepTracker::getPhaseName() const
{
    switch (getCurrentPhase())
    {
    case PHASE_AWAKE:
        return "Awake";
    case PHASE_LIGHT:
        return "Light";
    case PHASE_DEEP:
        return "Deep";
    case PHASE_REM:
        return "REM";
    default:
        return "---";
    }
}

SleepSession SleepTracker::getHistory(int index) const
{
    if (index < 0 || index >= HISTORY_COUNT)
    {
        SleepSession empty;
        empty.valid = false;
        return empty;
    }
    return _history[index];
}

int SleepTracker::calculateScore(unsigned long durationMs, float avgTemp, float avgHumidity)
{
    float hours = durationMs / 3600000.0f;

    // Duration score (0–70): peaks at IDEAL_SLEEP_HOURS, falls off on both sides
    float durationDiff = fabs(hours - IDEAL_SLEEP_HOURS);
    float durationScore = max(0.0f, (float)SCORE_DURATION_MAX * (1.0f - durationDiff / 4.0f));

    // Comfort score (0–30): based on temp and humidity proximity to ideal
    float tempDiff = fabs(avgTemp - IDEAL_TEMP) / 5.0f;         // normalize over 5 deg range
    float humDiff = fabs(avgHumidity - IDEAL_HUMIDITY) / 20.0f; // normalize over 20% range
    float comfortScore = (float)SCORE_COMFORT_MAX * max(0.0f, 1.0f - (tempDiff + humDiff) / 2.0f);

    return (int)(durationScore + comfortScore);
}

void SleepTracker::_shiftHistory(SleepSession newSession)
{
    // Shift older entries back, newest at index 0
    for (int i = HISTORY_COUNT - 1; i > 0; i--)
    {
        _history[i] = _history[i - 1];
    }
    _history[0] = newSession;
}

void SleepTracker::_saveHistory()
{
    prefs.begin(PREFS_NAMESPACE, false);
    prefs.putBytes("history", _history, sizeof(_history));
    prefs.end();
}

void SleepTracker::_loadHistory()
{
    prefs.begin(PREFS_NAMESPACE, true);
    if (prefs.isKey("history"))
    {
        prefs.getBytes("history", _history, sizeof(_history));
    }
    prefs.end();
}
