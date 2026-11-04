#pragma once

#include <Arduino.h>
#include "config.h"

//=============================================================================
// GPIO Pin Mapping (NodeMCU-32S / ESP32 DevKit)
//=============================================================================

// 8-Channel Relay Module Outputs
// Default development mapping matching typical 8-channel relay board routing:
// Relay 1 -> GPIO 14
// Relay 2 -> GPIO 18
// Relay 3 -> GPIO 19
// Relay 4 -> GPIO 23
// Relay 5 -> GPIO 26
// Relay 6 -> GPIO 27
// Relay 7 -> GPIO 32
// Relay 8 -> GPIO 33
static const int RELAY_PINS[NUM_RELAYS] = { 14, 18, 19, 23, 26, 27, 32, 33 };

// Status & Activity LED
#define PIN_STATUS_LED      2   // Builtin LED on NodeMCU-32S / DevKit v1

// I2C Bus Mapping (ESP32 Standard)
#define PIN_I2C_SDA         21
#define PIN_I2C_SCL         22

// I2C Device Addresses (Primary with Auto-detection fallback)
#define I2C_ADDR_LCD_PRIMARY   0x27
#define I2C_ADDR_LCD_FALLBACK  0x3F

#define I2C_ADDR_BME_PRIMARY   0x76
#define I2C_ADDR_BME_FALLBACK  0x77
