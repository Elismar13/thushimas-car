# Developer Guide — Integrations

Every external touch point in the current branch.

---

## 1. HTTP control interface

| Method | Path | Direction | Purpose |
| ------ | ---- | --------- | ------- |
| `POST` | `/api/control` | Phone → ESP32 | Post a control intent; receive the state snapshot. |
| `GET` | `/api/state` | Phone → ESP32 | Read the current state snapshot. |

Both requests carry `X-Control-Token`. The request and response schemas are in
[`docs/communication.md`](../communication.md).

The HTTP server is implemented by `lib/HttpApi`; validation and safety are
implemented behind `ControlEndpoint` in `lib/ControlProtocol`.

The current firmware joins the configured Wi-Fi network in station mode. The
access-point fallback and the phone page are later control-surface work.

---

## 2. GPIO map

This table tracks the branch pin map in [`docs/hardware.md`](../hardware.md) §3.

| Function | GPIO | Current code | Status |
| -------- | ---- | ------------ | ------ |
| Motor driver `INA` | 32 | — | issue #7 |
| Motor driver `INB` | 33 | — | issue #7 |
| Steering servo | 16 | — | issue #7/#8 |
| Ultrasonic `TRIG` | 25 | — | issue #11 |
| Ultrasonic `ECHO` | 26 | — | issue #11; 5 V divider mandatory |
| E-stop button | 4 | — | issue #13; `INPUT_PULLUP`, active low |
| Battery divider tap | 35 | — | issue #15; ADC1_CH7 |

The sensor-head servo, second ultrasonic and OLED pins are free on this
branch.

> Never assign a pin already in use. GPIO 6–11 are flash pins; GPIO 0, 2, 5,
> 12 and 15 are strapping pins; GPIO 34–39 have no internal pull-up.

---

## 3. Config header

`include/config.h` is gitignored. `include/config.example.h` is the committed
template.

| Macro | Meaning |
| ----- | ------- |
| `WIFI_SSID` / `WIFI_PASSWORD` | station-mode network credentials |
| `CAR_HOSTNAME` | the car's network name |
| `CONTROL_TOKEN` | shared token for every HTTP request |

No MQTT broker setting belongs on this branch.

---

## 4. Build environments

| Section | Purpose |
| ------- | ------- |
| `[platformio] default_envs = esp32dev` | firmware is built by default |
| `[env:esp32dev]` | ESP32 DevKit + Arduino framework |
| `[env:native]` | host tests; no `src/main.cpp`; hardware libraries ignored |

External dependencies:

| Library | Used by | Purpose |
| ------- | ------- | ------- |
| `bblanchon/ArduinoJson` | `lib/ControlProtocol` | control-intent parsing and state serialization |
| `teckel12/NewPing` | `lib/Ultrasonic` | HC-SR04 reading |
| `madhephaestus/ESP32Servo` | `lib/ServoControl` | servo PWM |
| Arduino WebServer framework | `lib/HttpApi` | HTTP socket handling |
| `ArduinoOTA` framework | `lib/OtaService` | wireless update |

The native environment ignores `ServoControl`, `Ultrasonic`, `OtaService` and
`HttpApi`, because those modules touch ESP32 or Arduino APIs.
