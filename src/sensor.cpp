#include "sensor.h"

SensorManager::SensorManager()
    : _dht(DHT_PIN, DHT11), _lastReadMs(0),
      _temp(0), _humidity(0),
      _tempSum(0), _humSum(0), _sampleCount(0)
{}

void SensorManager::begin() {
    _dht.begin();
}

void SensorManager::update() {
    if (millis() - _lastReadMs < SENSOR_READ_INTERVAL_MS) return;
    _lastReadMs = millis();

    float t = _dht.readTemperature();
    float h = _dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
        _temp = t;
        _humidity = h;
        _tempSum += t;
        _humSum  += h;
        _sampleCount++;
    }
}

float SensorManager::getAvgTemp() const {
    if (_sampleCount == 0) return _temp;
    return _tempSum / _sampleCount;
}

float SensorManager::getAvgHumidity() const {
    if (_sampleCount == 0) return _humidity;
    return _humSum / _sampleCount;
}

void SensorManager::resetAverages() {
    _tempSum     = 0;
    _humSum      = 0;
    _sampleCount = 0;
}
