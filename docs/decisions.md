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
| 19 | Driver libraries | Prefer mature external libraries for hardware drivers: `teckel12/NewPing` (HC-SR04) and `madhephaestus/ESP32Servo` (servos), via `lib_deps` in `platformio.ini` | Phase 1+ |
| 20 | Wrapper policy | Thin in-house wrappers (`lib/ServoControl`, `lib/Ultrasonic`) keep a stable mm/degrees interface; external libraries do the real work | Everywhere |

---

## Open (TBD)

| # | Item | Notes | Needed by |
| - | ---- | ----- | --------- |
| 1 | Chassis | Car-style chassis with steerable front wheels; source/model unknown | Phase 0/1 |
| 2 | Wheelbase L and δ_max | Must be measured from the actual chassis; affects kinematics, odometry, path feasibility | Phase 1, 7, 9 |
| 3 | Motor specs | Voltage, RPM, stall current (affects gearing, current draw, PWM mapping) | Phase 1 |
| 4 | Battery capacity | mAh of the 18650 cells | Phase 1 |
| 5 | Sensor-head layout | Two HC-SR04s angled apart vs parallel on the rotating head | Phase 4 |
| 6 | Scan geometry | 180° sweep with both sensors sampling, vs split coverage | Phase 4 |
| 7 | Motor encoder | Rear drive motor with encoder, for Phase 7 odometry | Phase 7 |
| 8 | Localization method | Encoder+steering vs IMU vs fusion; experimental | Phase 7 |
| 9 | Obstacle threshold | Stop distance for avoidance/E-stop | Phase 3 |
| 10 | Wi-Fi credentials | SSID/password config strategy (config header, secrets) | Phase 2 |
| 11 | Simulation | README mentions `simulator/`; scope undecided | Any |
| 12 | License | README says TBD | Any |

---

## Change Policy

* Moving an item from **Open** to **Decided** requires recording the decision here (and updating the affected docs).
* Breaking protocol changes bump `protocolVersion` (see `docs/communication.md`).
* Add a new row rather than silently rewriting history; if a decision is reversed, mark it `SUPERSEDED` with a pointer.
