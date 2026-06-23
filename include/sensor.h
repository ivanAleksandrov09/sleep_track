#pragma once
#include <Arduino.h>
#include <DHT.h>
#include "config.h"

class SensorManager {
public:
    SensorManager();
    void begin();

    // Call every loop() — only reads hardware every SENSOR_READ_INTERVAL_MS
    void update();

    float getTemp() const     { return _temp; }
    float getHumidity() const { return _humidity; }

    // Running averages since last reset (for scoring)
    float getAvgTemp() const;
    float getAvgHumidity() const;
    void resetAverages();

private:
    DHT _dht;
    unsigned long _lastReadMs;

    float _temp;
    float _humidity;

    // Running average state
    float _tempSum;
    float _humSum;
    unsigned long _sampleCount;
};
