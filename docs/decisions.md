# Decisions & Open Questions

This file records committed decisions and open (TBD) items. Agents should build for
`DECIDED` items and flag or ask about `TBD` items rather than guessing.

---

## Decided

| # | Decision | Value | Impact |
| - | -------- | ----- | ------ |
| 1 | Platform | Car: rear throttle motor + front steering servo (bicycle/Ackermann), not differential drive | Architecture, commands, kinematics, phases |
| 2 | Steering | One front servo steers both front wheels | Kinematic model (δ_max), hardware |
| 3 | Rear drive | Single DC motor through a differential axle | One L298N channel |
| 4 | Motor driver | L298N | Wiring, PWM control, power |
| 5 | Servos | 2× SG90 micro servo (steering + sensor head) | Power, PWM, torque limits |
| 6 | Ultrasonic | 2× HC-SR04 on a shared rotating head | Sequential firing, level shifting |
| 7 | Battery | 2× 18650 (7.4 V nominal) | Power rails |
| 8 | Regulation | Separate 5 V logic rail; motor rail independent | Brownout avoidance |
| 9 | E-stop | Software (timeout + ESTOP) **and** physical button | Safety, GPIO pin |
| 10 | Controller | ESP32 DevKit V1 (WROOM-32), PlatformIO/Arduino | Pin map, toolchain |
| 11 | Wi-Fi | Router mode first, ESP32 AP fallback | Transport |
| 12 | Messaging | MQTT, JSON payloads, broker on PC (Mosquitto) | Topics, schema |
| 13 | Movement commands | THROTTLE + STEER (no MOVE_FORWARD / TURN_LEFT / TURN_RIGHT) | Protocol |
| 14 | PC stack | Python (paho-mqtt, numpy, matplotlib/PyQtGraph) | Phase 5+ tooling |
| 15 | Environment | Small room (< 5 m) | Thresholds, map scale |
| 16 | Repo structure | Single evolving PlatformIO firmware; phases as git tags + docs | Layout |
| 17 | Reverse | Forward **and** reverse throttle | Phase 1, protocol |
| 18 | Pin map | Proposed GPIO allocation in `docs/hardware.md` | Wiring |
| 19 | Driver libraries | Mature external libraries via `lib_deps`: `teckel12/NewPing` (HC-SR04) and `madhephaestus/ESP32Servo` (servos) | Phase 1+ |
| 20 | Wrapper policy | Thin in-house wrappers (`lib/ServoControl`, `lib/Ultrasonic`) keep a stable mm/degrees interface; external libraries do the real work | Everywhere |
| 21 | Scan geometry | One head servo rotates a head carrying two HC-SR04s facing opposite (180° apart); 180° head sweep → 360° combined coverage | Phase 4 |
| 22 | Ultrasonic driver | SUPERSEDED by #19 (back to `teckel12/NewPing`); `lib/Ultrasonic` wraps it with correct µs→cm→mm conversion | Phase 1+ |
| 23 | MQTT client | `knolleary/PubSubClient` on the ESP32; JSON handled by `bblanchon/ArduinoJson` | Phase 4/5 |
| 24 | Wi-Fi config | Credentials + broker in `include/config.h` (gitignored); `include/config.example.h` committed as template | Phase 2+ |
| 25 | Scan payload | `car/scan` JSON: `{ type, protocolVersion, sensor, timestamp, points[ {bearing, distance, sensor} ] }` — schema in `docs/communication.md` §6 | Phase 5 |
| 26 | Angle convention | `bearing` in degrees, `0` = forward, CCW positive, `[0, 360)`; `distance` in mm; `0` = no echo | Phase 4/5 |
| 27 | Scan trigger | Command-driven + continuous: `START_SCAN`/`STOP_SCAN`; repeats periodically while active; stopped at boot | Phase 4/5 |
| 28 | Scan motion | Wiper sweep 0 → 180 → 0; each half-sweep published as one complete 360° scan | Phase 4 |
| 29 | PC GUI | Phase 5 test harness = PyQtGraph viewer in `tools/`; desktop is not the target platform (future: web/mobile); the protocol is the durable interface | Phase 5 |
| 30 | On-car display | 16×2 HD44780 + PCF8574 I2C (SDA=21, SCL=22), status-only, auto-rotating pages | Phase 5 |
| 31 | Display abstraction | Render via generic `TextDisplay` interface; `StatusDisplay` is display-agnostic so a future OLED/20×4 is a drop-in driver | Phase 5 |

---

## Open (TBD)

| # | Item | Notes | Needed by |
| - | ---- | ----- | --------- |
| 1 | Chassis | Car-style chassis with steerable front wheels; source/model unknown | Phase 0/1 |
| 2 | Wheelbase L and δ_max | Must be measured from the actual chassis; affects kinematics, odometry, path feasibility | Phase 1, 7, 9 |
| 3 | Motor specs | Voltage, RPM, stall current (affects gearing, current draw, PWM mapping) | Phase 1 |
| 4 | Battery capacity | mAh of the 18650 cells | Phase 1 |
| 5 | Sensor-head layout | SUPERSEDED by Decided #21 (two sensors facing opposite on one head) | — |
| 6 | Scan geometry | SUPERSEDED by Decided #21 (opposed sensors, 180° head sweep = 360°) | — |
| 7 | Motor encoder | Rear drive motor with encoder, for Phase 7 odometry | Phase 7 |
| 8 | Localization method | Encoder+steering vs IMU vs fusion; experimental | Phase 7 |
| 9 | Obstacle threshold | Stop distance for avoidance/E-stop | Phase 3 |
| 10 | Simulation | README mentions `simulator/`; scope undecided | Any |
| 11 | License | README says TBD | Any |

---

## Change Policy

* Moving an item from **Open** to **Decided** requires recording the decision here (and updating the affected docs).
* Breaking protocol changes bump `protocolVersion` (see `docs/communication.md`).
* Add a new row rather than silently rewriting history; if a decision is reversed, mark it `SUPERSEDED` with a pointer.
