#pragma once

#include <Arduino.h>

//=============================================================================
// Firmware Metadata & Branding
//=============================================================================
#define FW_NAME             "Legacy Controller"
#define FW_VERSION          "2.1.0"
#define FW_AUTHOR           "Nariman Ziaie"
#define FW_COPYRIGHT        "© 2019 - 2026 Nariman Ziaie"
#define GITHUB_REPO_URL     "https://github.com/nariman-z/legacy-controller"

//=============================================================================
// Hardware Configuration
//=============================================================================
#define NUM_RELAYS          8
#define RELAY_NO            true      // Normally Open: LOW = Energized (Active LOW relay board)
#define RESTORE_RELAY_STATE true      // Restore relay state from NVS on boot

// Environmental Sensor Calibration
#define SEALEVELPRESSURE_HPA (1013.25f)

//=============================================================================
// Network & AP Defaults
//=============================================================================
#define DEFAULT_AP_SSID     "Legacy Controller"
#define DEFAULT_AP_PASS     "esp32controller"   // TODO: CHANGE THIS
#define DEFAULT_AP_IP       IPAddress(192, 168, 4, 1)
#define DEFAULT_AP_GW       IPAddress(192, 168, 4, 1)
#define DEFAULT_AP_SUBNET   IPAddress(255, 255, 255, 0)

#define WIFI_CONNECT_TIMEOUT_MS 15000  // 15 seconds before falling back to AP mode
#define SENSOR_READ_INTERVAL_MS 4000   // Read BME280 & push SSE every 4s
#define LCD_CYCLE_INTERVAL_MS   3500   // Rotate LCD screen information every 3.5s

//=============================================================================
// Web Server & Security
//=============================================================================
#define HTTP_PORT           80
#define HTTP_AUTH_USER      "admin"
#define HTTP_AUTH_PASS      "admin"
#define HTTP_AUTH_REALM     "IoT Admin"

//=============================================================================
// Preferences (NVS) Namespace
//=============================================================================
#define NVS_NAMESPACE       "iot_config"
