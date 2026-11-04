#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include "config.h"
#include "hardware_pins.h"
#include "preferences_manager.h"
#include "sensor_manager.h"

// Fallback captive portal Wi-Fi setup HTML in PROGMEM (used if LittleFS has not been flashed yet)
static const char FALLBACK_WIFI_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Legacy Controller - Wi-Fi Setup</title>
  <style>
    body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background: #0b0f19; color: #f1f5f9; display: flex; justify-content: center; align-items: center; min-height: 100vh; margin: 0; padding: 16px; box-sizing: border-box; }
    .card { background: rgba(30, 41, 59, 0.95); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 16px; padding: 32px; max-width: 420px; width: 100%; box-shadow: 0 20px 40px rgba(0,0,0,0.5); }
    h2 { margin-top: 0; color: #38bdf8; font-size: 24px; text-align: center; }
    p { color: #94a3b8; font-size: 14px; text-align: center; line-height: 1.5; }
    .badge { display: block; background: rgba(245, 158, 11, 0.15); border: 1px solid #f59e0b; color: #fbbf24; padding: 10px; border-radius: 8px; font-size: 12px; margin-bottom: 20px; text-align: center; }
    label { display: block; font-size: 13px; font-weight: 600; margin-bottom: 6px; color: #cbd5e1; }
    input[type=text], input[type=password] { width: 100%; padding: 12px; margin-bottom: 16px; background: rgba(15, 23, 42, 0.8); border: 1px solid rgba(255,255,255,0.15); border-radius: 8px; color: #fff; font-size: 14px; box-sizing: border-box; outline: none; }
    input[type=text]:focus, input[type=password]:focus { border-color: #38bdf8; }
    input[type=submit] { width: 100%; padding: 14px; background: #0284c7; color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; cursor: pointer; transition: background 0.2s; }
    input[type=submit]:hover { background: #0369a1; }
  </style>
</head>
<body>
  <div class="card">
    <h2>Legacy Controller</h2>
    <p>Configure Wi-Fi credentials to connect the controller to your local network.</p>
    <form action="/" method="POST">
      <label for="ssid">Network Name (SSID):</label>
      <input type="text" id="ssid" name="ssid" placeholder="Enter Wi-Fi SSID" required>
      <label for="pass">Password:</label>
      <input type="password" id="pass" name="pass" placeholder="Enter Wi-Fi Password">
      <label for="ip">Static IP (Optional):</label>
      <input type="text" id="ip" name="ip" placeholder="e.g. 192.168.1.200">
      <input type="submit" value="Connect & Reboot">
    </form>
  </div>
</body>
</html>
)rawliteral";

class WebServerEngine {
public:
  WebServerEngine() 
    : _server(HTTP_PORT), _events("/events"), _isAPMode(false), _dnsServer(nullptr) {}

  ~WebServerEngine() {
    if (_dnsServer) delete _dnsServer;
  }

  void begin(bool isAPMode) {
    _isAPMode = isAPMode;

    if (_isAPMode) {
      _dnsServer = new DNSServer();
      _dnsServer->start(53, "*", DEFAULT_AP_IP);
      Serial.println("[DNS] Captive Portal DNS server started on port 53.");
    }

    setupRoutes();
    _server.addHandler(&_events);
    _server.begin();
    Serial.println("[HTTP] Asynchronous Web Server initialized on port 80.");
  }

  void processDNS() {
    if (_isAPMode && _dnsServer) {
      _dnsServer->processNextRequest();
    }
  }

  void broadcastTelemetry(float temp, float hum, float pres, float alti) {
    _events.send(String(temp, 1).c_str(), "temperature", millis());
    _events.send(String(hum, 1).c_str(), "humidity", millis());
    _events.send(String(pres, 1).c_str(), "pressure", millis());
    _events.send(String(alti, 1).c_str(), "altitude", millis());
  }

  void broadcastRelayChange(uint8_t relayId, uint8_t state) {
    char payload[64];
    snprintf(payload, sizeof(payload), "{\"id\":%u,\"state\":%u}", relayId, state);
    _events.send(payload, "relay", millis());
  }

  static void setRelayHardware(uint8_t index, bool state) {
    if (index >= NUM_RELAYS) return;
    int pin = RELAY_PINS[index];
    if (RELAY_NO) {
      digitalWrite(pin, state ? LOW : HIGH); // Active LOW
    } else {
      digitalWrite(pin, state ? HIGH : LOW); // Active HIGH
    }
  }

  static bool getRelayHardware(uint8_t index) {
    if (index >= NUM_RELAYS) return false;
    int pin = RELAY_PINS[index];
    int val = digitalRead(pin);
    return RELAY_NO ? (val == LOW) : (val == HIGH);
  }

private:
  AsyncWebServer _server;
  AsyncEventSource _events;
  bool _isAPMode;
  DNSServer* _dnsServer;

  // Process placeholders in HTML files
  void setupRoutes() {
    // Index / Home Route
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
      Serial.printf("[HTTP] GET / from %s (AP: %d)\n", request->client()->remoteIP().toString().c_str(), _isAPMode);
      const char* fileToServe = _isAPMode ? "/wifimanager.html" : "/index.html";
      if (LittleFS.exists(fileToServe)) {
        File fl = LittleFS.open(fileToServe, "r");
        if (fl) {
          size_t sz = fl.size();
          fl.close();
          Serial.printf("[HTTP] 200 OK: Serving %s (%u bytes)\n", fileToServe, (unsigned int)sz);
          request->send(LittleFS, fileToServe, "text/html");
        } else {
          Serial.printf("[HTTP] LittleFS.open('%s') failed! Falling back to PROGMEM HTML.\n", fileToServe);
          request->send_P(200, "text/html", FALLBACK_WIFI_HTML);
        }
      } else {
        Serial.printf("[HTTP] File '%s' does not exist on LittleFS! Falling back to PROGMEM HTML.\n", fileToServe);
        request->send_P(200, "text/html", FALLBACK_WIFI_HTML);
      }
    });

    // Explicit Route for Index / Home
    _server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.printf("[HTTP] GET /index from %s\n", request->client()->remoteIP().toString().c_str());
      if (LittleFS.exists("/index.html")) {
        request->send(LittleFS, "/index.html", "text/html");
      } else {
        request->send(404, "text/plain", "index.html not found on LittleFS");
      }
    });
    _server.on("/home", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->redirect("/index");
    });

    // Relay / Dashboard Route (with HTTP Basic Auth)
    auto dashboardHandler = [](AsyncWebServerRequest *request) {
      Serial.printf("[HTTP] GET %s from %s\n", request->url().c_str(), request->client()->remoteIP().toString().c_str());
      if (!request->authenticate(HTTP_AUTH_USER, HTTP_AUTH_PASS)) {
        return request->requestAuthentication(HTTP_AUTH_REALM);
      }
      if (LittleFS.exists("/dashboard.html")) {
        request->send(LittleFS, "/dashboard.html", "text/html", false);
      } else {
        request->send(200, "text/plain", "Dashboard file not found. Upload LittleFS filesystem with 'pio run -t uploadfs'.");
      }
    };
    _server.on("/dashboard", HTTP_GET, dashboardHandler);

    // Weather Telemetry
    _server.on("/weather", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.printf("[HTTP] GET /weather from %s\n", request->client()->remoteIP().toString().c_str());
      if (LittleFS.exists("/weather.html")) {
        request->send(LittleFS, "/weather.html", "text/html", false);
      } else {
        request->send(200, "text/plain", "Weather file not found. Upload LittleFS filesystem with 'pio run -t uploadfs'.");
      }
    });

    // Digital Clock
    _server.on("/clock", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.printf("[HTTP] GET /clock from %s\n", request->client()->remoteIP().toString().c_str());
      if (LittleFS.exists("/clock.html")) {
        request->send(LittleFS, "/clock.html", "text/html", false);
      } else {
        request->send(200, "text/plain", "Clock file not found. Upload LittleFS filesystem with 'pio run -t uploadfs'.");
      }
    });

    // Wi-Fi Setup Form
    _server.on("/wifimanager", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.printf("[HTTP] GET /wifimanager from %s\n", request->client()->remoteIP().toString().c_str());
      if (LittleFS.exists("/wifimanager.html")) {
        request->send(LittleFS, "/wifimanager.html", "text/html", false);
      } else {
        request->send_P(200, "text/html", FALLBACK_WIFI_HTML);
      }
    });

    // Favicon icon
    _server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (LittleFS.exists("/assets/images/icon.webp")) {
        request->send(LittleFS, "/assets/images/icon.webp", "image/webp");
      } else {
        request->send(404);
      }
    });

    // Wi-Fi Configuration API
    _server.on("/api/wifi-config", HTTP_GET, [](AsyncWebServerRequest *request) {
      String json = "{\"ssid\":\"" + prefsMgr.getSSID() + "\",\"ip\":\"" + prefsMgr.getStaticIP() + "\"}";
      request->send(200, "application/json", json);
    });

    // Android/Windows/Apple captive portal probing endpoints
    _server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->redirect("http://" + DEFAULT_AP_IP.toString() + "/");
    });
    _server.on("/gen_204", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->redirect("http://" + DEFAULT_AP_IP.toString() + "/");
    });
    _server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->redirect("http://" + DEFAULT_AP_IP.toString() + "/");
    });
    _server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Microsoft NCSI");
    });
    _server.on("/canonical.html", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->redirect("http://" + DEFAULT_AP_IP.toString() + "/");
    });

    // Wi-Fi POST Submission
    _server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      String newSSID = "";
      String newPass = "";
      String newIP = "";

      if (request->hasParam("ssid", true)) {
        newSSID = request->getParam("ssid", true)->value();
        newSSID.trim();
      }
      if (request->hasParam("pass", true)) {
        newPass = request->getParam("pass", true)->value();
        newPass.trim();
      }
      if (request->hasParam("ip", true)) {
        newIP = request->getParam("ip", true)->value();
        newIP.trim();
      }

      if (newSSID.length() > 0) {
        prefsMgr.saveWiFiConfig(newSSID, newPass, newIP);
        String response = "Success! Credentials saved. ESP32 is rebooting to connect to '" + newSSID + "'...";
        request->send(200, "text/plain", response);
        Serial.printf("[HTTP] Saved new Wi-Fi credentials for SSID '%s'. Rebooting in 2s...\n", newSSID.c_str());
        
        delay(2000);
        ESP.restart();
      } else {
        request->send(400, "text/plain", "Error: SSID cannot be empty.");
      }
    });

    // Relay Toggle Endpoint: /update?relay=<1..8|all>&state=<0|1>
    _server.on("/update", HTTP_GET, [this](AsyncWebServerRequest *request) {
      if (request->hasParam("relay") && request->hasParam("state")) {
        String relayParam = request->getParam("relay")->value();
        int state = request->getParam("state")->value().toInt();
        bool active = (state == 1);

        if (relayParam.equalsIgnoreCase("all")) {
          for (uint8_t i = 0; i < NUM_RELAYS; i++) {
            setRelayHardware(i, active);
            prefsMgr.setRelayState(i, active);
            broadcastRelayChange(i + 1, active ? 1 : 0);
          }
          Serial.printf("[Relay] Master Control: All relays set to %s.\n", active ? "ON" : "OFF");
        } else {
          int relayId = relayParam.toInt();
          if (relayId >= 1 && relayId <= NUM_RELAYS) {
            uint8_t idx = relayId - 1;
            setRelayHardware(idx, active);
            prefsMgr.setRelayState(idx, active);
            broadcastRelayChange(relayId, active ? 1 : 0);
            Serial.printf("[Relay] Channel %d (GPIO %d) set to %s.\n", relayId, RELAY_PINS[idx], active ? "ON" : "OFF");
          }
        }
        request->send(200, "text/plain", "OK");
      } else {
        request->send(400, "text/plain", "Invalid Parameters. Expected relay & state.");
      }
    });

    // REST API Endpoint: /api/status (JSON)
    _server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
      String json = "{";
      json += "\"firmware\":\"" + String(FW_NAME) + "\",";
      json += "\"version\":\"" + String(FW_VERSION) + "\",";
      json += "\"uptime_sec\":" + String(millis() / 1000) + ",";
      json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
      json += "\"relays\":[";
      for (uint8_t i = 0; i < NUM_RELAYS; i++) {
        json += "{\"id\":" + String(i + 1) + ",\"gpio\":" + String(RELAY_PINS[i]) + ",\"state\":" + String(getRelayHardware(i) ? 1 : 0) + "}";
        if (i < NUM_RELAYS - 1) json += ",";
      }
      json += "],";
      json += "\"sensor\":{";
      json += "\"available\":" + String(sensorMgr.isAvailable() ? "true" : "false") + ",";
      json += "\"temperature\":" + String(sensorMgr.getTemperature(), 2) + ",";
      json += "\"humidity\":" + String(sensorMgr.getHumidity(), 2) + ",";
      json += "\"pressure\":" + String(sensorMgr.getPressure(), 2) + ",";
      json += "\"altitude\":" + String(sensorMgr.getAltitude(), 2);
      json += "}}";
      request->send(200, "application/json", json);
    });

    // Logout Endpoints
    _server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(401, "text/plain", "Logged out");
    });

    _server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(LittleFS, "/logout.html", "text/html", false);
    });

    // Static Asset Serving (LittleFS) with HTTP client-side caching
    _server.serveStatic("/assets", LittleFS, "/assets").setCacheControl("max-age=604800"); // 7 days cache for static assets
    _server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html").setCacheControl("max-age=3600");

    // 404 Handler / Captive Portal Fallback
    _server.onNotFound([this](AsyncWebServerRequest *request) {
      String host = request->host();
      Serial.printf("[HTTP] 404/NotFound: %s (Host: %s)\n", request->url().c_str(), host.c_str());
      if (_isAPMode) {
        // ONLY redirect to captive portal if query is directed to an external host (e.g. google.com, apple.com)
        if (!host.startsWith("192.168.4.1") && !host.startsWith(DEFAULT_AP_IP.toString())) {
          request->redirect("http://" + DEFAULT_AP_IP.toString() + "/");
          return;
        }
      }
      request->send(404, "text/plain", "404 Not Found");
    });
  }
};

extern WebServerEngine webServer;
