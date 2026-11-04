#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "hardware_pins.h"

class DisplayManager {
public:
  DisplayManager() : _lcd(nullptr), _available(false), _lastCycleTime(0), _currentScreen(0) {}

  ~DisplayManager() {
    if (_lcd) delete _lcd;
  }

  bool begin() {
    Serial.println("[LCD] Scanning for 16x2 I2C LCD...");
    uint8_t addr = 0;

    // Probe primary address (0x27)
    Wire.beginTransmission(I2C_ADDR_LCD_PRIMARY);
    if (Wire.endTransmission() == 0) {
      addr = I2C_ADDR_LCD_PRIMARY;
    } else {
      // Probe fallback address (0x3F)
      Wire.beginTransmission(I2C_ADDR_LCD_FALLBACK);
      if (Wire.endTransmission() == 0) {
        addr = I2C_ADDR_LCD_FALLBACK;
      }
    }

    if (addr != 0) {
      _lcd = new LiquidCrystal_I2C(addr, 16, 2);
      _lcd->init();
      _lcd->backlight();
      _lcd->clear();
      _available = true;
      Serial.printf("[LCD] 16x2 LCD initialized at address 0x%02X.\n", addr);
      showSplash();
      return true;
    }

    Serial.println("[LCD] Notice: No I2C LCD detected. Display output disabled.");
    _available = false;
    return false;
  }

  void showSplash() {
    if (!_available || !_lcd) return;
    _lcd->clear();
    _lcd->setCursor(0, 0);
    _lcd->print("Creative");
    _lcd->setCursor(6, 1);
    _lcd->print("Electronic");
  }

  void showConnecting(const String& ssid) {
    if (!_available || !_lcd) return;
    _lcd->clear();
    _lcd->setCursor(0, 0);
    _lcd->print("Connecting WiFi:");
    _lcd->setCursor(0, 1);
    String s = ssid.substring(0, 16);
    _lcd->print(s);
  }

  void showConnected(const String& ip) {
    if (!_available || !_lcd) return;
    _lcd->clear();
    _lcd->setCursor(0, 0);
    _lcd->print("WiFi Connected!");
    _lcd->setCursor(0, 1);
    _lcd->print(ip);
  }

  void showAPMode(const String& ip) {
    if (!_available || !_lcd) return;
    _lcd->clear();
    _lcd->setCursor(0, 0);
    _lcd->print("Router NOT FOUND");
    _lcd->setCursor(0, 1);
    _lcd->print("AP: " + ip);
  }

  void updateTelemetry(const String& ip, uint8_t activeRelays, float temp, float hum, float pres) {
    if (!_available || !_lcd) return;

    unsigned long now = millis();
    if (now - _lastCycleTime >= LCD_CYCLE_INTERVAL_MS) {
      _lastCycleTime = now;
      _currentScreen = (_currentScreen + 1) % 2;

      _lcd->clear();
      if (_currentScreen == 0) {
        // Screen 0: Network & Relay Summary
        _lcd->setCursor(0, 0);
        _lcd->print("IP:" + ip);
        _lcd->setCursor(0, 1);
        _lcd->printf("Relays: %d/8 ON", activeRelays);
      } else {
        // Screen 1: Environmental Sensor Data
        _lcd->setCursor(0, 0);
        _lcd->printf("T:%.1fC H:%.0f%%", temp, hum);
        _lcd->setCursor(0, 1);
        _lcd->printf("P:%.0fhPa", pres);
      }
    }
  }

  bool isAvailable() const { return _available; }

private:
  LiquidCrystal_I2C* _lcd;
  bool _available;
  unsigned long _lastCycleTime;
  uint8_t _currentScreen;
};

extern DisplayManager displayMgr;
