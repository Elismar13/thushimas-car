# Developer Guide — Integrations

All external touch points in one place: MQTT topics, GPIO pins, config header,
build environment, and PC tools. Where there are two sources of truth (e.g.
pins in code **and** in `docs/hardware.md`), keep them in sync.

---

## 1. MQTT

Broker on the PC (Mosquitto), Wi-Fi + MQTT. Schema in `docs/communication.md`.

| Topic | Direction | QoS | Producer | Consumer | Status |
| ----- | --------- | --- | -------- | -------- | ------ |
| `car/cmd` | PC → ESP32 | 1 | `sonar_viewer.py`, `mosquitto_pub` | `MqttTransport` callback → `handleCommand()` | **Implemented** (START_SCAN / STOP_SCAN / PING) |
| `car/scan` | ESP32 → PC | 0 | `main.cpp publishScan()` | `sonar_viewer.py`, `mock_scan_publisher.py` test subscriber | **Implemented** |
| `car/status` | ESP32 → PC | 0 | `main.cpp publishStatus()` (after commands) | — (no PC consumer yet) | Implemented, unused |
| `car/telemetry` | ESP32 → PC | 0 | — | — | Documented (`communication.md` §4), **not implemented** |

Command set in firmware today: `START_SCAN`, `STOP_SCAN`, `STEER {angle}`,
`PING`. `THROTTLE`, `STOP`, `ESTOP`, `RESET_ESTOP`, `SET_SPEED_LIMIT` are in
the protocol but dispatch is Phase 2.

**Constants:** broker host/port/clientId → `include/config.h`; topics →
`src/main.cpp` (`kCmdTopic`, `kScanTopic`, `kStatusTopic`).

**Publish size limit:** PubSubClient is compiled with
`MQTT_MAX_PACKET_SIZE=2048` (a full scan is ~1.7 kB; the default 256 B silently
drops publishes — this was a real bug, see `git log a916730`).

---

## 2. GPIO map (firmware ↔ hardware doc)

| Function | GPIO | Code constant | Accuracy |
| -------- | ---- | ------------- | -------- |
| Sensor-head servo | 17 | `kHeadServoPin` | matches `hardware.md` §3 |
| Steering servo | 16 | `kSteerServoPin` (range `[40, 140]`) | matches `hardware.md` §3 |
| Ultrasonic 1 TRIG | 25 | `kTrig1Pin` | matches |
| Ultrasonic 1 ECHO | 26 | `kEcho1Pin` | matches (5 V → level-shift!) |
| Ultrasonic 2 TRIG | 32 | `kTrig2Pin` | matches |
| Ultrasonic 2 ECHO | 33 | `kEcho2Pin` | matches (5 V → level-shift!) |
| I2C SDA | 21 | `Ssd1306Display` uses `Wire` | matches (`hardware.md` §4) |
| I2C SCL | 22 | `Ssd1306Display` uses `Wire` | matches |
| E-stop button | 4 | — (Phase 1) | matches |
| L298N ENA/IN1/IN2 | 13/14/27 | — (Phase 1) | matches |

> Never assign a pin already in use; consult `docs/hardware.md` §3 constraints
> (flash pins 6–11, strapping pins, input-only 34–39).

---

## 3. Config header (`include/config.h`)

Gitignored; template `include/config.example.h`. Read at compile time by
`MqttTransport` construction in `src/main.cpp`.

| Macro | Meaning |
| ----- | ------- |
| `WIFI_SSID` / `WIFI_PASSWORD` | network credentials |
| `MQTT_BROKER_HOST` | PC LAN IP (e.g. `10.220.0.18`) |
| `MQTT_BROKER_PORT` | default `1883` |
| `MQTT_CLIENT_ID` | broker-visible client id |

---

## 4. Build environment (`platformio.ini`)

| Section | Purpose |
| ------- | ------- |
| `[platformio] default_envs = esp32dev` | `pio run` builds firmware only |
| `[env:esp32dev]` | espressif32/esp32dev + arduino; `-DMQTT_MAX_PACKET_SIZE=2048` |
| `[env:native]` | host tests; `build_src_filter = -<*>` + `lib_ignore` |

`lib_deps` (external libraries — never vendored into `lib/`):

| Library | Used by | Purpose |
| ------- | ------- | ------- |
| `teckel12/NewPing` | `lib/Ultrasonic` | HC-SR04 ping |
| `madhephaestus/ESP32Servo` | `lib/ServoControl` | 50 Hz servo PWM |
| `knolleary/PubSubClient` | `lib/MqttTransport` | MQTT client |
| `bblanchon/ArduinoJson` | `src/main.cpp` | scan/status/command JSON |
| `adafruit/Adafruit SSD1306` + `adafruit/Adafruit GFX Library` | `lib/Ssd1306Display` | 128x64 I2C OLED |
| `ArduinoOTA` (framework lib) | `lib/OtaService` | wireless flash over UDP 3232 |

`lib_ignore` in the native env excludes hardware libs so tests link only pure
code: `ServoControl, Ultrasonic, MqttTransport, SonarScan, Ssd1306Display,
OtaService`.

**Ports:** MQTT (TCP 1883) and OTA (UDP 3232, ArduinoOTA) coexist on the same
Wi-Fi link without conflict.

---

## 5. PC tooling (`tools/`)

Dependencies (`requirements.txt`): `paho-mqtt`, `numpy`, `pyqtgraph`.

| Tool | Broker needed | Purpose | Talks to car |
| ---- | ------------- | ------- | ------------ |
| `sonar_viewer.py` | yes | view scans live; send start/stop/ping | yes (commands) |
| `mock_scan_publisher.py` | yes | fake scan source (no hardware) | **no** |

PyQt6 on Linux needs system package `libxcb-cursor0`; broker LAN setup needs a
Mosquitto `listener 1883 0.0.0.0` + `allow_anonymous true` on the PC — see
`docs/local-development.md`.

---

## 6. Cross-cutting matrix

| Concern | Pure (host-testable) | Hardware/ESP32 | PC-side |
| ------- | -------------------- | -------------- | ------- |
| Scan data model, nearest obstacle | `lib/ScanData` | — | — |
| Head sweep timing/measurement | — | `lib/SonarScan` (+`ServoControl`, `Ultrasonic`) | — |
| MQTT/Wi-Fi | — | `lib/MqttTransport` | broker + tools |
| Display rendering | `lib/StatusDisplay` (+`TextDisplay`) | `lib/Ssd1306Display` | — |
| JSON schema | (mirrored in docs) | `src/main.cpp` ArduinoJson | `tools/*.py` json |

**Rule of thumb:** keep units mm/degrees; keep `ScanData`-style logic pure so it
can be unit-tested on the host; keep `platformio.ini`, `docs/hardware.md`, and
`src/main.cpp` pin/flag constants in agreement.