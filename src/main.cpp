///////////////////////////////////////////////////////////////////////////////////////
//                                                                                   //
//                             ESP32 IOT HOME AUTOMATION                             //
//                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////
//                              Ambinet Legacy Controller                             //
//                  © 2019 - 2026 Nariman Ziaie, All rights reserved                  //
///////////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <LittleFS.h>

#include "config.h"
#include "hardware_pins.h"
#include "preferences_manager.h"
#include "sensor_manager.h"
#include "display_manager.h"
#include "web_server_engine.h"

//=============================================================================
// Global Subsystem Instances
//=============================================================================
PreferencesManager prefsMgr;
SensorManager      sensorMgr;
DisplayManager     displayMgr;
WebServerEngine    webServer;

// Runtime state
static bool isAPMode = false;
static unsigned long lastSensorBroadcast = 0;
static unsigned long lastWiFiCheck = 0;

//=============================================================================
// ASCII Art Banner
//=============================================================================
void printBanner() {
  Serial.println("\n");
  Serial.println("=======================================================================");
  Serial.println("     _   _            _                                ______          ");
  Serial.println("    | \\ | |          (_)                              |___  /         ");
  Serial.println("    |  \\| | __ _ _ __ _ _ __ ___   __ _ _ __             / /          ");
  Serial.println("    | . ` |/ _` | '__| | '_ ` _ \\ / _` | '_ \\           / /          ");
  Serial.println("    | |\\  | (_| | |  | | | | | | | (_| | | | | ______  / /__          ");
  Serial.println("    |_| \\_|\\__,_|_|  |_|_| |_| |_|\\__,_|_| |_||______|/_____|       ");
  Serial.println("                                                                       ");
  Serial.println("                ESP32 IoT Home Automation & Gateway                    ");
  Serial.printf ("             Version %s | %s\n", FW_VERSION, FW_COPYRIGHT);
  Serial.println("=======================================================================\n");
}

//=============================================================================
// Hardware Relays Initialization
//=============================================================================
void initRelays() {
  Serial.println("[Relay] Initializing 8-Channel Relay Controller...");
  for (uint8_t i = 0; i < NUM_RELAYS; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    
    // Restore persistent state if configured
    bool state = false;
    if (RESTORE_RELAY_STATE) {
      state = prefsMgr.getRelayState(i);
    }
    
    WebServerEngine::setRelayHardware(i, state);
    Serial.printf("[Relay] CH%d (GPIO %2d) -> %s\n", i + 1, RELAY_PINS[i], state ? "ON" : "OFF");
  }
}

//=============================================================================
// Mount Filesystem (LittleFS)
//=============================================================================
void initFilesystem() {
  Serial.println("[Storage] Mounting LittleFS...");
  if (!LittleFS.begin(true)) {
    Serial.println("[Storage] Error: LittleFS mount failed even after formatting!");
  } else {
    size_t total = LittleFS.totalBytes();
    size_t used = LittleFS.usedBytes();
    Serial.printf("[Storage] LittleFS mounted successfully: %u KB used / %u KB total (%u KB free)\n",
                  (unsigned int)(used / 1024), 
                  (unsigned int)(total / 1024), 
                  (unsigned int)((total - used) / 1024));

    // Verify key files and sizes
    const char* testFiles[] = {"/index.html", "/wifimanager.html", "/dashboard.html", "/weather.html", "/clock.html"};
    for (const char* f : testFiles) {
      if (LittleFS.exists(f)) {
        File fl = LittleFS.open(f, "r");
        Serial.printf("[Storage] File %-18s: OK (%u bytes)\n", f, fl ? (unsigned int)fl.size() : 0);
        if (fl) fl.close();
      } else {
        Serial.printf("[Storage] File %-18s: MISSING!\n", f);
      }
    }
  }
}

//=============================================================================
// Network Initialization (STA with SoftAP Fallback)
//=============================================================================
void initNetwork() {
  String savedSSID = prefsMgr.getSSID();
  String savedPass = prefsMgr.getPassword();
  String savedIP   = prefsMgr.getStaticIP();

  if (savedSSID.length() > 0) {
    Serial.printf("[WiFi] Attempting connection to SSID: '%s'...\n", savedSSID.c_str());
    displayMgr.showConnecting(savedSSID);
    
    WiFi.mode(WIFI_STA);

    // Optional Static IP Configuration
    if (savedIP.length() > 0 && !savedIP.equalsIgnoreCase("dhcp")) {
      IPAddress staticIP;
      if (staticIP.fromString(savedIP)) {
        IPAddress gw;
        String gwStr = savedIP.substring(0, savedIP.lastIndexOf('.') + 1) + "1";
        gw.fromString(gwStr);
        IPAddress subnet(255, 255, 255, 0);
        WiFi.config(staticIP, gw, subnet);
        Serial.printf("[WiFi] Configured Static IP: %s\n", staticIP.toString().c_str());
      }
    }

    WiFi.begin(savedSSID.c_str(), savedPass.c_str());

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startAttempt < WIFI_CONNECT_TIMEOUT_MS)) {
      delay(250);
      digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED)); // Blink while connecting
      Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
      isAPMode = false;
      digitalWrite(PIN_STATUS_LED, HIGH); // Solid ON when connected
      String ip = WiFi.localIP().toString();
      Serial.printf("[WiFi] Connected! IP Address: %s (Signal RSSI: %d dBm)\n", ip.c_str(), WiFi.RSSI());
      displayMgr.showConnected(ip);
      delay(2000);
      return;
    } else {
      Serial.println("[WiFi] Connection timed out. Falling back to Access Point Mode.");
    }
  } else {
    Serial.println("[WiFi] No saved Wi-Fi credentials found. Launching initial Access Point Setup Mode.");
  }

  // Fallback: Start SoftAP & Captive Portal
  isAPMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(DEFAULT_AP_IP, DEFAULT_AP_GW, DEFAULT_AP_SUBNET);
  WiFi.softAP(DEFAULT_AP_SSID, DEFAULT_AP_PASS);

  String apIP = WiFi.softAPIP().toString();
  Serial.printf("[WiFi-AP] SoftAP Started: '%s' | Password: '%s'\n", DEFAULT_AP_SSID, DEFAULT_AP_PASS);
  Serial.printf("[WiFi-AP] Gateway IP: %s\n", apIP.c_str());
  displayMgr.showAPMode(apIP);
  delay(2500);
}

//=============================================================================
// Arduino Setup
//=============================================================================
void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);

  printBanner();

  // Initialize I2C Bus
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  // Initialize Peripherals & Storage
  displayMgr.begin();
  sensorMgr.begin();
  initFilesystem();
  prefsMgr.begin();
  prefsMgr.migrateLegacyFiles();
  initRelays();

  // Network & Web Server Startup
  initNetwork();
  webServer.begin(isAPMode);

  Serial.println("[System] Initialization complete. Entering operational event loop.\n");
}

//=============================================================================
// Arduino Main Loop
//=============================================================================
void loop() {
  // Process Captive Portal DNS queries in SoftAP mode
  if (isAPMode) {
    webServer.processDNS();
  }

  unsigned long now = millis();

  // Periodic Environmental Telemetry & SSE Broadcast
  if (now - lastSensorBroadcast >= SENSOR_READ_INTERVAL_MS) {
    lastSensorBroadcast = now;
    sensorMgr.update(true);

    float t = sensorMgr.getTemperature();
    float h = sensorMgr.getHumidity();
    float p = sensorMgr.getPressure();
    float a = sensorMgr.getAltitude();

    // Broadcast over Server-Sent Events to all open web clients
    webServer.broadcastTelemetry(t, h, p, a);

    // Calculate count of active energized relays
    uint8_t activeCount = 0;
    for (uint8_t i = 0; i < NUM_RELAYS; i++) {
      if (WebServerEngine::getRelayHardware(i)) activeCount++;
    }

    // Update 16x2 LCD display
    String currentIP = isAPMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
    displayMgr.updateTelemetry(currentIP, activeCount, t, h, p);
  }

  // Periodic Wi-Fi Watchdog (Auto-reconnect if dropped in STA mode)
  if (!isAPMode && (now - lastWiFiCheck >= 10000)) {
    lastWiFiCheck = now;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[Watchdog] WiFi connection dropped! Reconnecting...");
      digitalWrite(PIN_STATUS_LED, LOW);
      WiFi.reconnect();
    } else {
      digitalWrite(PIN_STATUS_LED, HIGH);
    }
  }

  delay(10);
}
