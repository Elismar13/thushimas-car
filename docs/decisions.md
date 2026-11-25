# Decisions & Open Questions

This file records committed decisions and open (TBD) items. Agents should build for
`DECIDED` items and flag or ask about `TBD` items rather than guessing.

---

## Branch Overrides — `feat/phone-controlled-car`

This branch pivots the project from an autonomous-navigation research platform to a
**phone-controlled car with an obstacle interlock**. The `main` line keeps the original
decisions unchanged; on this branch the rows below are replaced. Build for the
replacement, not the original.

| Overridden | Replacement | What changes |
| ---------- | ----------- | ------------ |
| #5 Servos (2× SG90) | #45 | One steering servo only — the sensor-head servo is removed |
| #6 Ultrasonic (2× on a rotating head) | #40 | One fixed frontal sensor; no rotating head |
| #21 Scan geometry | #40 | 360° coverage is gone; only forward coverage remains |
| #25, #26, #27, #28 Scan payload, angles, trigger, motion | — | The `car/scan` topic and the wiper sweep are removed |
| #12 Messaging (MQTT, broker on PC) | #38 | The ESP32 serves HTTP directly; the phone is the only client |
| #23 MQTT client (`PubSubClient`) | #38 | Replaced by an HTTP server; `ArduinoJson` is kept |
| #13 Movement commands | #39 | Control intent is a TTL'd setpoint, not a fire-and-forget command |
| #14 PC stack (paho-mqtt, PyQtGraph) | #43 | The PC tools are deleted; the browser is the UI |
| #16 Repo structure (single evolving firmware) | #43 | Selective rewrite: research modules deleted, new ones added |
| #29 PC GUI | #44 | The desktop viewer is removed; a phone web app replaces it |
| #30, #31, #35 On-car display + `TextDisplay` seam | #44 | The OLED and its display abstraction are removed |
| #3, #4, #7, #8 Motor driver, rear drive, battery, regulation | #41 | **Carried forward unchanged** — L298N and 2× 18650 stay |
| #9, #10 E-stop (software + physical) | — | **Carried forward unchanged** — see § Safety Invariants |
| #11 Wi-Fi (router first, AP fallback) | #38 | **Carried forward unchanged** — the phone reaches the car over it |
| #19, #20, #22 Driver libraries, wrapper policy, ultrasonic driver | #43 | **Carried forward unchanged** — `NewPing` and `ESP32Servo` stay |
| #24 Wi-Fi config (`include/config.h`) | #38 | **Carried forward unchanged** |
| #32, #33 Steering range and `STEER` mapping | — | **Carried forward unchanged** |
| #34 OTA updates | — | **Carried forward unchanged** — wireless flash is now the primary path |

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
| 30 | On-car display (initial) | SUPERSEDED by #35 — was 16×2 HD44780 + PCF8574 I2C (SDA=21, SCL=22), status-only, auto-rotating pages | Phase 5 |
| 31 | Display abstraction | Render via generic `TextDisplay` interface; `StatusDisplay` is display-agnostic so a future OLED/20×4 is a drop-in driver | Phase 5 |
| 32 | Steering servo range | Pin 16; physical linkage limits servo travel to `[40°, 140°]` (90° = wheels straight) | Phase 1 |
| 33 | STEER mapping | `STEER {angle}` maps 1:1 to servo degrees relative to straight (0→90°, +50→140°, −50→40°); wheel `δ_max` (TBD #2) will be re-mapped later | Phase 1/2 |
| 34 | OTA updates | Wireless flash via ArduinoOTA protocol (UDP 3232), driven by PlatformIO `espota` (`pio run -t upload --upload-port <ip>`); coexists with MQTT (TCP 1883); first flash still via USB | Phase 2+ |
| 35 | On-car display (revised) | 128×64 monochrome SSD1306 OLED over I2C (SDA=21, SCL=22, @ 0x3C), status-only, auto-rotating pages; replaces the 16×2 HD44780 (#30). 21×8 character grid via the `TextDisplay` seam (#31) | Phase 5 |
| 36 | Branch strategy | Separate branch `feat/phone-controlled-car` off `main`; `main` is left intact as the autonomous-navigation research line; the two lines are not kept in sync | Process, docs |
| 37 | Product scope (this branch) | A phone-controlled car with an obstacle **interlock**; the autonomy roadmap (mapping → localization → exploration → path planning → navigation) is archived on this branch, not carried forward | Branch roadmap |
| 38 | Phone link | The ESP32 joins the home Wi-Fi (STA) with AP fallback (#11) and serves a web app over HTTP; the phone is the only client; no broker, no PC | Transport, protocol, tooling |
| 39 | Control protocol | The phone posts a **control intent** (throttle + steering angle) with a TTL; the car holds it only while the intent is fresh, then stops. A heartbeat rides the same channel | Protocol, safety |
| 40 | Obstacle avoidance | **Interlock only**: throttle is refused while the frontal distance is below the stop threshold. The car never steers itself to avoid an obstacle | Behaviour, safety |
| 41 | Power hardware | SUPERSEDED by #50 and #56 — the pack became 1S2P with a step-up converter, not a 7.4 V series rail. The battery-monitoring half survives as #51 | Hardware, wiring |
| 42 | Charge management | The charger is hardware-only in v1; the car has no charge-state input and is **not** designed to be operated while charging | BOM, safety |
| 43 | Codebase strategy | Selective rewrite: keep the PlatformIO skeleton and the `lib/` wrapper conventions; keep `ServoControl` and `Ultrasonic`; delete the research modules and the PC tools; add `MotorDriver`, `CarControl`, `HttpApi`, `BatteryMonitor` | Repo layout, modules |
| 44 | Display surface | The OLED and the `TextDisplay`/`StatusDisplay`/`Ssd1306Display` modules are removed, along with the desktop viewer; the phone browser is the only display | Hardware, modules, tooling |
| 45 | Sensor head | The sensor-head servo and the second HC-SR04 are removed; one SG90 steers the front wheels and one HC-SR04 faces forward. The rotating 360° head is gone | Hardware, wiring, pin map |
| 46 | Reverse | Reverse is permitted without restriction or a speed cap. The car carries **no obstacle detection rearward** — the phone UI must state this plainly, and the car is blind behind at all times | Safety, behaviour, UI |
| 47 | Phone control surface | A single **self-centering virtual joystick** (x = steering angle, y = requested throttle), plus a separate soft STOP button and a latched-stop button that requires a confirm to clear. The page holds a screen wake lock so the browser cannot background mid-drive | UI |
| 48 | Arming | The car boots **disarmed**. The phone arms it by **press-and-hold**; arming expires with the car link, so a dropped phone cannot leave the car armed | Safety, UX |
| 49 | Stop model | Three levels: **soft STOP** (requested throttle → 0, the car stays armed), **latched stop** (motor off, cleared from the phone with a confirm), **physical E-stop** (motor off, cleared only by physically releasing the button — the phone cannot clear it) | Safety |
| 50 | Battery pack | The pack is **two 18650 cells in parallel** (1S2P: 3.7 V nominal, 4.2 V max), not the series pair in #7. A **DC/DC step-up converter** supplies the motor and logic rails. Parallel cells must be matched and charged with a parallel-capable charger | Hardware, power, charging |
| 51 | Battery monitoring | The ADC divider measures the **pack voltage**, not the boosted rail. Warn at **≤ 3.5 V**, cut throttle at **≤ 3.1 V**; the percentage comes from a piecewise Li-ion discharge table, not a linear map. The cut re-enables once the pack recovers above the warn threshold | Power, safety |
| 52 | Sensor fault policy | A missing ultrasonic echo is treated as **clear space**. There is **no sensor health check**, so a disconnected sensor is indistinguishable from an open road | Safety |
| 53 | Testing | The native unity env is kept and the safety logic is built **test-first**: `CarControl` (interlock, link freshness, arming, latched stop, battery cut), `BatteryMonitor` (voltage → percentage) and the control-intent parser are covered by native unit tests. `test_display` is deleted; `test_sonar` is replaced by interlock tests | Quality, repo layout |
| 54 | ADR | The pivot is recorded as `docs/adr/0001-pivot-to-phone-controlled-car.md`. Every other decision stays in this table | Process |
| 55 | Rear motor | The rear motor is a 5 V, ~500 mA DVD-drive **DC motor with two conductors**, driven by a **single H-bridge with only `IN1`/`IN2`** — no enable pin. The driver truth table is: forward `IN1` = 1/PWM, `IN2` = 0; reverse `IN1` = 0, `IN2` = 1/PWM; standby (coast) `0,0`; brake `1,1`. Resolves Open #26. Supersedes #3 | Firmware, hardware |
| 56 | Power rail | A **single 5 V rail** from the 1S2P step-up converter feeds the ESP32 (VIN), the servos, the HC-SR04 and the motor driver's supply. No separate motor rail, no second converter. Resolves Open #19 | Hardware, power |
| 57 | Standby sleep | When no control intent has arrived for ~3 s the ESP32 drops to **Wi-Fi modem sleep** (~20–30 mA instead of ~80 mA associated-and-idle). Not deep sleep — deep sleep drops the association and the phone could no longer reach the car. The ramp delay on wake is absorbed by the 3 s idle window | Power, firmware |
| 58 | Transport and rate | **HTTP**, control intent posted at **20 Hz**, car-side throttle slew limiting, and the transport hidden behind a small interface so a UDP phone app can replace it later without touching `CarControl`. Car state rides back in the control response, so there is no separate telemetry channel. Resolves Open #17 | Protocol, firmware |
| 59 | Motor driver | The driver is a **TC1508A** (ChipSourceTek, SOP-16) — a dual-channel integrated-MOSFET full bridge. Channel A is used (INA/INB → OUTA/OUTB); channel C is left free. Confirmed truth table: `L,L` = standby (outputs Hi-Z), `H,L` = forward, `L,H` = backward, `H,H` = brake. **Single supply** `VDD` 2.4–7.2 V — there is no separate logic rail. `R_ON` 1.0 Ω typ / 1.6 Ω max per MOSFET, so the two-FET path drops ~1.0 V at 0.5 A and the motor sees ~4.0 V of the 5 V rail. 1.8 A continuous / 2.5 A peak per channel; standby current ≤ 10 µA. Resolves Open #27 | Hardware, wiring |
| 60 | Motor idle | The motor is **de-energised when the car is stopped** — no standing current, no heat. On this driver, stop = standby `IN1=0, IN2=0`, which coasts the motor (outputs Hi-Z). Free-spinning rear axle, self-locking steering servo, so there is nothing to hold. Resolves Open #22 (first half) | Firmware, power |
| 61 | Power switch | A physical **maintained power switch** in the pack → converter path, with a main fuse ahead of it. Cutting it kills the car completely — motor, ESP32, radio and OTA — and is recovered only by flipping it back | Hardware, safety |
| 62 | Branch roadmap | The branch runs five phases — Bench → Safety core → Control surface → On-car → Polish — defined in `docs/roadmap.md` | Planning |
| 63 | Control API | **One** `POST /api/control` endpoint carries the intent (`t`, `s`, `arm`, `latch`, `seq`) and returns the full state snapshot. `GET /` serves the app; `GET /api/state` covers page load and reconnect. A **shared token** in a request header authenticates every request | Protocol, security |
| 64 | E-stop and power switch | **Both are kept.** The power switch kills the car completely; the GPIO-4 E-stop button cuts the motor while leaving the link alive, so it can be released and re-armed | Safety, hardware |
| 65 | Throttle mechanism | The throttle is realised as **PWM duty on the active input** — `IN1` for forward, `IN2` for reverse — with the other input held static. Duty maps as `minDuty + (255 − minDuty) × |throttle| / 100`, so the motor breaks away from rest. PWM frequency depends on Open #27 | Firmware, wiring |
| 66 | Drivetrain calibration | The gear ratio, wheel size and top speed are **measured empirically later**, not derived now. The throttle→duty map and the minimum starting duty are calibrated on the real car | Firmware, calibration |
| 67 | Direction-change interlock | The driver is never switched from one direction to the other directly. Sequence: ramp the duty to 0 → drive standby `0,0` → hold standby ~50 ms to let the back-EMF and current decay → then set the new direction and ramp up. A direct flip reverses a spinning motor and dumps its back-EMF through the bridge, which browns out the boost converter and resets the ESP32 | Firmware, safety, power |
| 68 | Stop states | **Coast** (`0,0`) for an ordinary stop — the car rolls freely. **Brake** (`1,1`) for the latched stop and the physical E-stop, where the car should stop hard rather than roll. Leaving brake requires passing through standby first, per #67 | Firmware, behaviour |
| 69 | PWM frequency | **16 kHz**, 8-bit. The TC1508A is a MOSFET bridge, so high-frequency chopping is efficient and inaudible — unlike the L298N, where ~2 kHz was the ceiling. The motor's ~10 ms electrical time constant is far longer than the 62 µs PWM period, so throttle maps near-linearly to speed | Firmware |
| 70 | Driver thermal limit | The TC1508A's maximum dissipation is **1.73 W** (θJA 75 °C/W). At the motor's 500 mA the bridge drops ~0.5 W, which is fine — but a **stalled** motor can pull ~1.5 A and dissipate ~4.5 W, well over the limit. The chip has no current limiting, so the obstacle interlock is the only protection against thermal damage | Hardware, safety |

---

## Open (TBD)

| # | Item | Notes | Needed by |
| - | ---- | ----- | --------- |
| 1 | Chassis | Car-style chassis with steerable front wheels; source/model unknown | Phase 0/1 |
| 2 | Wheelbase L and δ_max | Not needed on this branch (no odometry, no path planning); only the physical servo travel limits matter (#32) | — |
| 3 | Motor specs | RESOLVED by Decided #55 — 5 V DVD-drive stepper, ~500 mA per phase | — |
| 4 | Battery capacity | mAh of each cell; the pack is 1S2P, so pack capacity ≈ 2× cell mAh at 3.7 V | v1 |
| 5 | Sensor-head layout | SUPERSEDED by Decided #21 (two sensors facing opposite on one head) | — |
| 6 | Scan geometry | SUPERSEDED by Decided #21 (opposed sensors, 180° head sweep = 360°) | — |
| 7 | Motor encoder | Not needed on this branch (no odometry, no path planning) | — |
| 8 | Localization method | Not needed on this branch (no odometry, no path planning) | — |
| 9 | Obstacle threshold | RENAMED on this branch to #13 (stop threshold) — the E-stop part is withdrawn, the interlock part is kept | Phase 3 |
| 10 | Simulation | README mentions `simulator/`; scope undecided | Any |
| 11 | License | README says TBD | Any |
| 12 | Reverse policy | RESOLVED by Decided #46 | — |
| 13 | Stop threshold value | The distance (mm) ahead of the car at which the interlock engages | v1 |
| 14 | Battery thresholds | RESOLVED by Decided #51 | — |
| 15 | Arming model | RESOLVED by Decided #48 | — |
| 16 | Phone control surface | RESOLVED by Decided #47 | — |
| 17 | Telemetry transport | RESOLVED by Decided #58 | — |
| 18 | Pivot ADR | RESOLVED by Decided #54 | — |
| 19 | Step-up converter output | RESOLVED by Decided #56 | — |
| 20 | Throttle slew rate | The maximum rate at which the actual throttle ramps toward the requested throttle (realised as a step-rate ramp) | v1 |
| 21 | Motor driver for the stepper | RESOLVED by Decided #59 | — |
| 22 | Stepper drive semantics | RESOLVED by Decided #60 (idle coils); the sequencer's timing model is settled in the roadmap | — |
| 23 | Power switch vs. E-stop button | RESOLVED by Decided #61 and #64 | — |
| 24 | Gear ratio / wheel size | RESOLVED by Decided #66 — measured empirically in Phase 4 | — |
| 26 | Rear motor type | RESOLVED by Decided #55 | — |
| 27 | H-bridge module silicon | RESOLVED by Decided #59 — TC1508A | — |
| 28 | H-bridge logic supply | RESOLVED by Decided #59 — single `VDD` supply, no separate logic rail; `VIH` = 2.0 V min, so the ESP32's 3.3 V GPIOs drive `INA`/`INB` directly | — |

---

## Change Policy

* Moving an item from **Open** to **Decided** requires recording the decision here (and updating the affected docs).
* Breaking protocol changes bump `protocolVersion` (see `docs/communication.md`).
* Add a new row rather than silently rewriting history; if a decision is reversed, mark it `SUPERSEDED` with a pointer.
