#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <LittleFS.h>
#include "config.h"

class PreferencesManager {
public:
  PreferencesManager() : _initialized(false) {}

  bool begin() {
    if (_prefs.begin(NVS_NAMESPACE, false)) {
      _initialized = true;
      Serial.println("[NVS] Preferences initialized successfully.");
      return true;
    }
    Serial.println("[NVS] Error: Failed to initialize Preferences!");
    return false;
  }

  // Check if legacy SPIFFS/LittleFS text files exist and migrate to NVS
  void migrateLegacyFiles() {
    if (!_initialized) return;

    if (getSSID().isEmpty() && LittleFS.exists("/ssid.txt")) {
      File f = LittleFS.open("/ssid.txt", "r");
      if (f) {
        String s = f.readStringUntil('\n');
        s.trim();
        if (s.length() > 0) {
          saveSSID(s);
          Serial.printf("[Migration] Migrated SSID '%s' from legacy file.\n", s.c_str());
        }
        f.close();
      }
    }

    if (getPassword().isEmpty() && LittleFS.exists("/pass.txt")) {
      File f = LittleFS.open("/pass.txt", "r");
      if (f) {
        String p = f.readStringUntil('\n');
        p.trim();
        savePassword(p);
        Serial.println("[Migration] Migrated password from legacy file.");
        f.close();
      }
    }

    if (getStaticIP().isEmpty() && LittleFS.exists("/ip.txt")) {
      File f = LittleFS.open("/ip.txt", "r");
      if (f) {
        String ip = f.readStringUntil('\n');
        ip.trim();
        saveStaticIP(ip);
        Serial.printf("[Migration] Migrated IP '%s' from legacy file.\n", ip.c_str());
        f.close();
      }
    }
  }

  // Wi-Fi Configuration
  String getSSID() {
    return _initialized ? _prefs.getString("ssid", "") : "";
  }

  void saveSSID(const String& ssid) {
    if (_initialized) _prefs.putString("ssid", ssid);
  }

  String getPassword() {
    return _initialized ? _prefs.getString("pass", "") : "";
  }

  void savePassword(const String& pass) {
    if (_initialized) _prefs.putString("pass", pass);
  }

  String getStaticIP() {
    return _initialized ? _prefs.getString("ip", "") : "";
  }

  void saveStaticIP(const String& ip) {
    if (_initialized) _prefs.putString("ip", ip);
  }

  void saveWiFiConfig(const String& ssid, const String& pass, const String& ip) {
    saveSSID(ssid);
    savePassword(pass);
    saveStaticIP(ip);
    Serial.println("[NVS] Wi-Fi settings saved to flash.");
  }

  // Relay State Persistence (8-bit mask)
  uint8_t getRelayMask() {
    return _initialized ? _prefs.getUChar("relay_mask", 0) : 0;
  }

  void saveRelayMask(uint8_t mask) {
    if (_initialized) _prefs.putUChar("relay_mask", mask);
  }

  bool getRelayState(uint8_t index) {
    if (index >= NUM_RELAYS) return false;
    uint8_t mask = getRelayMask();
    return (mask & (1 << index)) != 0;
  }

  void setRelayState(uint8_t index, bool state) {
    if (index >= NUM_RELAYS || !_initialized) return;
    uint8_t mask = getRelayMask();
    if (state) {
      mask |= (1 << index);
    } else {
      mask &= ~(1 << index);
    }
    saveRelayMask(mask);
  }

  void clearAll() {
    if (_initialized) {
      _prefs.clear();
      Serial.println("[NVS] Factory reset: Preferences partition cleared.");
    }
  }

private:
  Preferences _prefs;
  bool _initialized;
};

extern PreferencesManager prefsMgr;
