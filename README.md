<!-- Gradient Divider -->
<img src="https://user-images.githubusercontent.com/73097560/115834477-dbab4500-a447-11eb-908a-139a6edaec5c.gif">

<!-- Dynamic Header Banner -->
<img width="100%" height=350 alt="Repo Banner Legacy Controller" src="https://capsule-render.vercel.app/api?type=waving&color=0:0F172A,100:1E293B&height=300&section=header&text=⚡%20Legacy%20Controller&fontSize=42&fontColor=FFFFFF&fontAlign=50&fontAlignY=38&desc=Self-Contained%20Smart%20Home%20IoT%20Gateway%20%20Controller&descSize=16&descColor=FFFFFF&descAlign=50&descAlignY=60" />

<p align="center">
  <b>Legacy Controller – ESP32 Smart Home IoT Controller & Local Gateway (v2.0)</b><br>
  <small><i>- An asynchronous, zero-cloud embedded home automation gateway featuring 8-channel relay switching, live BME280 telemetry via SSE, interactive multi-mode clock suite, LittleFS web interface, and I2C LCD status cycler -</i></small>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Status-Archived%20%2F%20Legacy-lightgrey.svg" alt="Status: Archived">
  <a href="https://platformio.org/"><img src="https://img.shields.io/badge/PlatformIO-Framework%20Arduino-orange.svg" alt="PlatformIO"></a>
  <a href="https://www.espressif.com/"><img src="https://img.shields.io/badge/Hardware-ESP32%20NodeMCU-red.svg" alt="ESP32"></a>
  <a href="https://github.com/espressif/arduino-esp32"><img src="https://img.shields.io/badge/Filesystem-LittleFS-blue.svg" alt="Filesystem"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="License: MIT"></a>
</p>

---

## 📌 Project Status: Archived & Academic Origins

> [!NOTE]
> **Archived Repository & Harvard CS50x Final Project**  
> Originally engineered and submitted as the **Harvard CS50x Final Project** by **Nariman Ziaie**.  
> 
> **Archive Notice:** This repository is preserved in a read-only archived state for historical, educational, and reference purposes. Modern ongoing developments have transitioned to the **Dimmo** and **Ambinet** platforms detailed below.

### 📖 The Origin Story

> *"The spark for this project began back when I was a high school student. In those days, there were no AI coding assistants to generate code or debug errors—learning embedded programming meant diving headfirst into raw technical datasheets, community forum threads, compiler errors, and late nights surrounded by breadboards, jumper wires, and microcontrollers.*  
> 
> *I completely immersed myself in the IoT domain, experimenting relentlessly with hardware peripherals, asynchronous web engines, I2C telemetry, and local network protocols. Later on, when completing **Harvard's CS50x**, I decided to fuse these isolated hardware experiments and individual prototypes into a single, unified, production-grade platform: an autonomous, zero-cloud smart home controller and local web gateway.*  
> 
> *This repository stands as a preserved milestone of that journey—a testament to curiosity, self-taught embedded engineering, and hands-on hardware exploration."*  
> — **Nariman Ziaie**

This repository contains the complete firmware, web interface assets, and partition configuration for **Legacy Controller** — a self-contained, production-grade **Smart Home IoT Controller and Local Gateway** built for the **ESP32** microcontroller using PlatformIO and the Arduino framework.

---

## 💡 Executive Summary & Core Capabilities

Built on a non-blocking asynchronous core, this firmware hosts a rich, responsive, dark-themed dashboard served directly from on-chip **LittleFS** flash storage. It provides robust hardware abstraction and instant client feedback with zero cloud dependencies:

* **8-Channel Appliance Control:** Active-LOW relay driver supporting independent toggling and master all-on/all-off controls.
* **Non-Volatile State Restoration:** Relay latching states and network credentials persist in NVS flash via `Preferences.h`, resuming exact states across power outages.
* **Live Environmental Telemetry:** Continuous BME280 monitoring (temperature, relative humidity, atmospheric pressure, and approximate altitude) pushed via **Server-Sent Events (SSE)**.
* **Interactive Multi-Mode Clock:** Client-side synchronized clock supporting four interchangeable viewports (3D Fliqlo mechanical Flip Clock, Analog sweep clock, CS50x Neon Glow, and Minimal Card).
* **WiFi-Manager Captive Portal:** Dynamic DNS-intercepting SoftAP (`Legacy Controller`) for instant Wi-Fi credential onboarding and static IP configuration.
* **I2C Status LCD Display:** Cyclic hardware reporting on a standard 16x2 I2C LCD, displaying connection status, assigned IP, and environmental readings.
* **100% Offline & Private:** Zero external telemetry, tracking, or cloud brokers required — operates seamlessly on local LANs or air-gapped environments.

---

## 📂 Repository & Firmware Structure

```text
legacy-controller/
├── platformio.ini              # PlatformIO build configuration & board profiles
├── partitions.csv              # Custom 4MB partition table (2MB App, 2MB LittleFS)
├── README.md                   # System documentation & developer runbook
├── LICENSE                     # MIT Open-Source License
├── .gitignore                  # Git repository exclusion rules
├── include/
│   ├── config.h                # Central firmware settings, defaults & timers
│   ├── hardware_pins.h         # GPIO assignments and I2C address definitions
│   ├── preferences_manager.h   # NVS persistent configuration & relay state storage
│   ├── sensor_manager.h        # BME280 environmental telemetry driver
│   ├── display_manager.h       # 16x2 I2C LCD status & telemetry cycler
│   └── web_server_engine.h     # HTTP routes, SSE broadcaster & captive portal
├── src/
│   └── main.cpp                # Firmware lifecycle, watchdog & event loop
└── data/                       # LittleFS on-chip web filesystem root
    ├── index.html              # Main landing portal & system overview
    ├── dashboard.html          # 8-channel relay control dashboard
    ├── weather.html            # Live weather telemetry cards
    ├── clock.html              # Multi-mode clock (Fliqlo Flip, Analog, CS50x Glow, Minimal)
    ├── wifimanager.html        # Wi-Fi credentials & network setup form
    ├── logout.html             # Administrative session logout page
    └── assets/                 # Modular web assets
        ├── css/                # Stylesheets (W3.css, font-awesome, raleway)
        ├── fonts/              # Vector web fonts (WOFF, WOFF2)
        └── images/             # Optimized WebP imagery (background, icon)
```

---

## 🔌 Hardware Specifications & Pin Mapping

> [!IMPORTANT]
> The default GPIO mappings are configured for a standard **NodeMCU-32S** / **ESP32-WROOM-32** board wired to an 8-channel relay module and I2C peripherals. All board-specific GPIO mappings are centralized in [`include/hardware_pins.h`](include/hardware_pins.h).

### Pinout Configuration Table

| Peripheral Signal | Direction | ESP32 GPIO | Logic Level | Circuit Purpose |
| :--- | :--- | :--- | :--- | :--- |
| **Relay Channel 1** | Output | `GPIO 14` | Active-LOW (`0` = ON) | Channel 1 Relay Trigger (Appliance 1) |
| **Relay Channel 2** | Output | `GPIO 18` | Active-LOW (`0` = ON) | Channel 2 Relay Trigger (Appliance 2) |
| **Relay Channel 3** | Output | `GPIO 19` | Active-LOW (`0` = ON) | Channel 3 Relay Trigger (Appliance 3) |
| **Relay Channel 4** | Output | `GPIO 23` | Active-LOW (`0` = ON) | Channel 4 Relay Trigger (Appliance 4) |
| **Relay Channel 5** | Output | `GPIO 26` | Active-LOW (`0` = ON) | Channel 5 Relay Trigger (Appliance 5) |
| **Relay Channel 6** | Output | `GPIO 27` | Active-LOW (`0` = ON) | Channel 6 Relay Trigger (Appliance 6) |
| **Relay Channel 7** | Output | `GPIO 32` | Active-LOW (`0` = ON) | Channel 7 Relay Trigger (Appliance 7) |
| **Relay Channel 8** | Output | `GPIO 33` | Active-LOW (`0` = ON) | Channel 8 Relay Trigger (Appliance 8) |
| **Status LED** | Output | `GPIO 2` | Active-HIGH (`1` = ON) | Wi-Fi Connection & Activity Indicator |
| **I2C SDA** | Bidirectional | `GPIO 21` | 3.3V Logic | Shared I2C Serial Data (BME280 & LCD) |
| **I2C SCL** | Output | `GPIO 22` | 3.3V Logic | Shared I2C Serial Clock (BME280 & LCD) |

### I2C Bus Peripheral Addresses

| Device | Primary I2C Address | Alternate I2C Address | Notes |
| :--- | :--- | :--- | :--- |
| **BME280 Sensor** | `0x76` | `0x77` | Automatic fallback detection during setup |
| **16x2 LCD Display** | `0x27` | `0x3F` | PCF8574 / PCF8574A I2C backpack support |

---

## 💾 Flash Partitioning & Storage Layout

The firmware utilizes an optimized partition scheme validated for standard **4MB SPI Flash**:

```csv
# Name,   Type, SubType, Offset,   Size,     Flags
nvs,      data, nvs,     0x9000,   0x5000,
otadata,  data, ota,     0xe000,   0x2000,
app0,     app,  ota_0,   0x10000,  0x1e0000,
spiffs,   data, spiffs,  0x1f0000, 0x1f0000,
```

* **`app0` (1.9 MB):** Compiled Arduino application firmware.
* **`spiffs / littlefs` (1.9 MB):** LittleFS static web filesystem partition. The bundled assets (`data/`) occupy only **~656 KB**, reserving over **1.2 MB** of free storage headroom.
* **`nvs` (20 KB):** Non-volatile preferences partition holding Wi-Fi credentials, custom IP settings, and saved relay channel states.

---

## 🖥️ Web Dashboard & User Interface Suite

### Available Routes & Endpoints

| Route | Source File | Authentication | Description |
| :--- | :--- | :--- | :--- |
| **`/`** | `index.html` / `wifimanager.html` | Public | Main landing portal (STA mode) or Wi-Fi Setup (AP mode) |
| **`/dashboard`** | `dashboard.html` | **Basic Auth** (`admin` / `admin`) | 8-Channel interactive dashboard with animated toggle switches |
| **`/weather`** | `weather.html` | Public | Real-time environmental gauges (Temp, Humidity, Pressure, Altitude) |
| **`/clock`** | `clock.html` | Public | Multi-mode clock suite (Fliqlo 3D Flip, Analog Dial, CS50x Glow, Minimal Card) |
| **`/wifimanager`** | `wifimanager.html` | Public | Captive portal network & static IP configuration form |
| **`/logout`** | — | Public | Invalidation endpoint returning HTTP 401 Unauthorized |
| **`/logged-out`**| `logout.html` | Public | Confirmation screen with 5-second automatic redirection |

---

## 📡 REST API & Real-Time Event Stream Reference

### 1. Relay Control REST Endpoint
Toggles one or all channels via an HTTP GET query:
```http
GET /update?relay=<ID>&state=<STATE> HTTP/1.1
Host: <ESP32-IP>
```
* **Parameters:**
  * `relay`: Channel index (`1` through `8`) or `"all"`
  * `state`: Target binary state (`1` for ON, `0` for OFF)
* **Response:** `HTTP 200 OK` (Body: `"OK"`)

### 2. System Status API
Returns complete diagnostic state in JSON format:
```http
GET /api/status HTTP/1.1
Host: <ESP32-IP>
```
* **Response Body (`application/json`):**
```json
{
  "firmware": "Legacy Controller",
  "version": "2.1.0",
  "uptime_sec": 142,
  "rssi": -58,
  "relays": [
    { "id": 1, "gpio": 14, "state": 1 },
    { "id": 2, "gpio": 18, "state": 0 },
    { "id": 3, "gpio": 19, "state": 0 },
    { "id": 4, "gpio": 23, "state": 0 },
    { "id": 5, "gpio": 26, "state": 0 },
    { "id": 6, "gpio": 27, "state": 0 },
    { "id": 7, "gpio": 32, "state": 0 },
    { "id": 8, "gpio": 33, "state": 0 }
  ],
  "sensor": {
    "available": true,
    "temperature": 24.6,
    "humidity": 48.2,
    "pressure": 1012.8,
    "altitude": 124.5
  }
}
```

### 3. Server-Sent Events (SSE) Stream
Continuous unidirectional event stream for instant UI synchronization without polling:
```http
GET /events HTTP/1.1
Host: <ESP32-IP>
Accept: text/event-stream
```
* **Dispatched Events:**
  * `temperature`: Floating point temperature string (e.g. `"24.6"`)
  * `humidity`: Floating point relative humidity percentage (e.g. `"48.2"`)
  * `pressure`: Floating point barometric pressure in hPa (e.g. `"1012.8"`)
  * `altitude`: Floating point altitude estimation in meters (e.g. `"124.5"`)
  * `relay`: JSON string `{ "id": 1, "state": 1 }` dispatched whenever any channel state changes

---

## 🚀 Getting Started & Build Runbook

### Prerequisites
* [Visual Studio Code](https://code.visualstudio.com/) with the [PlatformIO IDE Extension](https://platformio.org/platformio-ide) installed (or PlatformIO Core CLI).
* ESP32 Development Board (NodeMCU-32S / ESP32-WROOM-32) connected via USB.

### Step 1: Clone & Open the Workspace
```bash
git clone https://github.com/Nariman-Z/legacy-controller.git
cd legacy-controller
```

### Step 2: Build & Flash Application Firmware
Compile the C++ source code and flash the firmware binary to the ESP32:
```bash
pio run -t upload
```

### Step 3: Build & Flash LittleFS Web Filesystem
Flash the HTML, CSS, fonts, and images from the `data/` directory to the flash partition:
```bash
pio run -t uploadfs
```

> [!IMPORTANT]
> Both steps (`upload` and `uploadfs`) are required on first setup. If only the firmware is uploaded without flashing the LittleFS filesystem, the web server will return 404 because the web pages do not yet exist in SPI flash.

### Step 4: Serial Monitor Diagnostics
Launch the serial monitor at **115200 baud** to monitor boot sequences and connection status:
```bash
pio device monitor -b 115200
```

---

## 🌐 First Boot & Captive Portal Provisioning Workflow

```text
[Power On] ──> Check NVS for Wi-Fi Credentials
                    │
        ┌───────────┴───────────┐
        │ Credentials Found     │ No Credentials / Failed Connection
        ▼                       ▼
Connect to Home Router    Start SoftAP ("Legacy Controller")
        │                       │
        ▼                       ▼
Assigned IP (e.g.       AP IP: 192.168.4.1 (DNS Captive Portal)
192.168.1.100)                  │
        │                       ▼
        │                 Connect Phone/PC to SoftAP
        │                 Open Browser (auto-redirects to Setup)
        │                       │
        │                 Submit SSID, Password & Optional Static IP
        │                       │
        │                 Save to NVS & Reboot
        │                       │
        └───────────────────────┘
```

1. **First Boot**: When the ESP32 starts without saved credentials (or fails to connect after 15 seconds), it automatically launches a SoftAP configured via [`include/config.h`](include/config.h):
   * **SSID:** `Legacy Controller` (`DEFAULT_AP_SSID`)
   * **Password:** `esp32controller` (`DEFAULT_AP_PASS`)
   * **Default Gateway IP:** `192.168.4.1` (`DEFAULT_AP_IP`)
2. **Connect**: Connect your smartphone, tablet, or laptop to the `Legacy Controller` Wi-Fi network.
3. **Captive Portal**: The DNS server automatically intercepts browser requests and opens the setup wizard. Alternatively, browse to `http://192.168.4.1`.
4. **Configure**: Select or enter your local Wi-Fi SSID and password. Choose DHCP or enter custom static IP parameters.
5. **Reboot**: Click **Save & Connect**. The ESP32 commits your settings to NVS and restarts in station mode connected to your local network.

---

## 🛡️ Reliability, Safety & Fault Tolerance

* **Non-Blocking Bus Recovery:** The I2C scanner probes peripherals during boot. If the BME280 sensor or LCD display is missing or loose, the system skips it gracefully without freezing the event loop.
* **Hardware Watchdog Protection:** A dedicated watchdog timer monitors loop execution and reboots the ESP32 if an unexpected freeze or deadloop occurs.
* **Failsafe Appliance State:** Relay channels initialize to safe de-energized states before reading NVS, ensuring no spurious toggling occurs during power transitions.
* **Flash Wear Mitigation:** `Preferences.h` writes relay states using non-volatile key-value mapping with built-in wear-leveling to protect SPI flash longevity.

---

## 🔍 Diagnostics & Troubleshooting

| Symptom | Probable Cause | Corrective Action |
| :--- | :--- | :--- |
| **Browser returns "404 Not Found"** | LittleFS filesystem image not flashed | Run `pio run -t uploadfs` in PlatformIO to upload `data/` assets to flash. |
| **LCD displays blank / solid boxes** | Contrast potentiometer or incorrect I2C address | Adjust the blue contrast potentiometer on the backpack. Firmware scans `0x27` and `0x3F`. |
| **BME280 reads default values (`24.5 °C`)** | Sensor disconnected or reversed SDA/SCL lines | Ensure `SDA` is wired to `GPIO 21` and `SCL` to `GPIO 22`. Verify VCC is connected to `3.3V` (not 5V). |
| **ESP32 loops in SoftAP mode** | Invalid Wi-Fi password or weak 2.4 GHz signal | Verify credentials in the captive portal. ESP32 connects only to 2.4 GHz Wi-Fi networks (not 5 GHz). |

---

## 🏛️ Project Archive & Successor Ecosystems

> [!IMPORTANT]
> **This repository is officially archived.**  
> `legacy-controller` reflects the architectural baseline of an early Harvard CS50x final project. Active maintenance has concluded, and next-generation IoT innovations continue in the following modern platforms:

### 1. [Dimmo](https://github.com/Nariman-Z/Dimmo) (Public Open-Source)
* **Repository:** [github.com/Nariman-Z/Dimmo](https://github.com/Nariman-Z/Dimmo)
* **Architecture:** Precision-engineered AC power regulation, closed-loop TRIAC phase-angle control, zero-crossing detection circuit topologies, true RMS electrical telemetry, and hardware-level safety mechanisms.
* **Status:** Publicly available on GitHub.

### 2. Ambinet IoT Platform (Flagship Smart Home Ecosystem)
* **Official Website:** [**ambinet.pages.dev**](https://ambinet.pages.dev/)
* **Architecture:** Enterprise-grade IoT smart home ecosystem featuring hardware-anchored security (Protocomm Security 2 / SRP6a, NIST P-256 ECDSA), encrypted MQTTS (TLS 8883) communications, dynamic multi-profile device classification, and a cross-platform Flutter mobile suite.
* **Status:** Private repository — explore the live platform features, specs, and interfaces directly at [ambinet.pages.dev](https://ambinet.pages.dev/).

---

## 📄 License & Developer Signature

Licensed under the terms of the **MIT License**. See the full [`LICENSE`](LICENSE) file for details.

---

<div align="center">

### Made with ❤️ by [Nariman_Z](https://github.com/nariman-z)  
🌐 [Personal Website](https://nariman-z.pages.dev/)

</div>

<!-- Gradient Divider -->
<img src="https://user-images.githubusercontent.com/73097560/115834477-dbab4500-a447-11eb-908a-139a6edaec5c.gif">

_Last Update: November 2026_
