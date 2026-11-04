#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "config.h"
#include "hardware_pins.h"

class SensorManager {
public:
  SensorManager() 
    : _available(false), _lastReadTime(0),
      _temperature(0.0f), _humidity(0.0f), _pressure(0.0f), _altitude(0.0f) {}

  bool begin() {
    Serial.println("[Sensor] Scanning for BME280 on I2C bus...");
    
    // Attempt primary address (0x76)
    if (_bme.begin(I2C_ADDR_BME_PRIMARY)) {
      _available = true;
      Serial.printf("[Sensor] BME280 detected at address 0x%02X.\n", I2C_ADDR_BME_PRIMARY);
      read();
      return true;
    }

    // Attempt fallback address (0x77)
    if (_bme.begin(I2C_ADDR_BME_FALLBACK)) {
      _available = true;
      Serial.printf("[Sensor] BME280 detected at address 0x%02X.\n", I2C_ADDR_BME_FALLBACK);
      read();
      return true;
    }

    _available = false;
    Serial.println("[Sensor] Warning: No BME280 sensor found. Continuing in simulation/standby mode.");
    return false;
  }

  bool update(bool force = false) {
    unsigned long now = millis();
    if (force || (now - _lastReadTime >= SENSOR_READ_INTERVAL_MS)) {
      _lastReadTime = now;
      return read();
    }
    return false;
  }

  bool read() {
    if (_available) {
      _temperature = _bme.readTemperature();
      _humidity    = _bme.readHumidity();
      _pressure    = _bme.readPressure() / 100.0f; // Convert Pa to hPa
      _altitude    = _bme.readAltitude(SEALEVELPRESSURE_HPA);
      return true;
    } else {
      // In case hardware sensor is absent, provide dummy baseline data so UI still renders smoothly
      _temperature = 24.5f;
      _humidity    = 45.0f;
      _pressure    = 1013.25f;
      _altitude    = 120.0f;
      return false;
    }
  }

  bool isAvailable() const { return _available; }
  float getTemperature() const { return _temperature; }
  float getHumidity() const { return _humidity; }
  float getPressure() const { return _pressure; }
  float getAltitude() const { return _altitude; }

private:
  Adafruit_BME280 _bme;
  bool _available;
  unsigned long _lastReadTime;
  float _temperature;
  float _humidity;
  float _pressure;
  float _altitude;
};

extern SensorManager sensorMgr;
