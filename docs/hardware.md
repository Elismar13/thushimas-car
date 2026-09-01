# Hardware

## 1. Motion Model

The platform is a **car**, not a differential-drive rover:

* A single rear DC motor drives both rear wheels through a differential axle (throttle).
* A front servo steers both front wheels together (bicycle/Ackermann steering).
* There is **no turn-in-place**. Every turn has a turning radius.

The kinematic model is documented in `docs/architecture.md` (§ Kinematic Model).

---

## 2. Current Hardware

| Component                  | Spec                                        | Purpose                  | Status  |
| -------------------------- | ------------------------------------------- | ------------------------ | ------- |
| ESP32 DevKit V1 (WROOM-32) | Dual-core, Wi-Fi + BLE                      | Main controller          | DECIDED |
| DC geared motor (rear)     | Single, drives differential axle            | Throttle (fwd/rev)       | DECIDED |
| Motor driver               | L298N (one channel used)                    | Motor control            | DECIDED |
| Steering servo             | SG90 micro servo (front)                    | Cornering                | DECIDED |
| Sensor-head servo          | SG90 micro servo                            | Rotates the sonar head   | DECIDED |
| Ultrasonic sensors         | 2× HC-SR04 on one head, facing opposite     | Distance measurement     | DECIDED |
| Chassis                    | Car-style, steerable front wheels           | Mechanical platform      | TBD     |
| Wheels                     | 2 driven rear, 2 steered front              | Movement                 | TBD     |
| Battery                    | 2× 18650 (7.4 V nominal)                    | Power                    | DECIDED |
| Voltage regulation         | Motor rail + logic rail (separate 5 V buck) | Power                    | DECIDED |
| Emergency stop             | Physical push button (wired to ESP32 GPIO)  | Safety                   | DECIDED |
| Motor encoder              | On the rear drive motor                     | Odometry (Phase 7)       | TBD     |

**Status legend:** `DECIDED` = committed choice, agents should build for it. `TBD` = open decision, verify before relying on it.

---

## 3. Proposed Pin Map

Proposed allocation for the ESP32 DevKit V1 (WROOM-32). Adjust during wiring if conflicts arise.

| Function             | GPIO  | Notes                                                        |
| -------------------- | ----- | ------------------------------------------------------------ |
| L298N ENA (PWM)      | 13    | PWM throttle. Do not enable ENA before IN1/IN2 are set.       |
| L298N IN1            | 14    | Direction A                                                   |
| L298N IN2            | 27    | Direction B                                                   |
| Steering servo       | 16    | PWM 50 Hz, ~1–2 ms pulse                                      |
| Sensor-head servo    | 17    | PWM 50 Hz, rotates the dual-sensor head                       |
| Ultrasonic 1 TRIG    | 25    | HC-SR04                                                      |
| Ultrasonic 1 ECHO    | 26    | **5 V → 3.3 V level shifting required** (resistor divider)   |
| Ultrasonic 2 TRIG    | 32    | HC-SR04                                                      |
| Ultrasonic 2 ECHO    | 33    | **5 V → 3.3 V level shifting required** (resistor divider)   |
| E-stop button        | 4     | INPUT_PULLUP, active low, wired to GND                        |
| Encoder A (future)   | 34    | Input-only, interrupt-capable, for Phase 7                    |
| Encoder B (future)   | 35    | Input-only, interrupt-capable, for Phase 7                    |
| I2C SDA (future)     | 21    | Reserved for IMU or similar                                   |
| I2C SCL (future)     | 22    | Reserved for IMU or similar                                   |
| UART0                | 1/3   | Built-in USB serial for debug                                 |

Constraints respected:

* GPIO 6–11 are flash pins — never use.
* GPIO 0, 2, 5, 12, 15 are strapping pins — avoided for outputs.
* GPIO 34–39 are input-only — used only for inputs (encoder).
* HC-SR04 ECHO outputs 5 V; ESP32 pins are not 5 V tolerant — add a divider.

---

## 4. Power Architecture

Two independent rails:

```text
2× 18650 (7.4 V nominal)
        │
        ├──────────→ L298N motor supply (VS) ──────→ rear motor
        │
        └──────────→ 5 V buck regulator ────┬──→ ESP32 VIN
                                            ├──→ steering + head servos (SG90)
                                            └──→ HC-SR04 VCC
```

* Motor rail: 7.4 V directly to the L298N. Do not draw logic power from it.
* Logic rail: separate 5 V buck for ESP32 (via VIN), the two SG90 servos, and both HC-SR04s.
* The L298N has an onboard 5 V regulator; it is available but **should not be the primary logic supply** to avoid brownouts when the motor draws current.
* Add a bulk capacitor (e.g. 470–1000 µF) across the motor rail near the L298N.
* SG90 servos must share a rail with enough current for two servos under load.

---

## 5. Potential Future Hardware

Additional hardware may be introduced as required. Candidates include:

* Rear motor encoder (the preferred candidate for Phase 7 odometry — status TBD)
* IMU
* Battery voltage sensor
* Better time-of-flight distance sensors
* LiDAR
* Optical flow sensor
* Camera
* UWB positioning

Hardware should only be added when a project phase demonstrates a requirement for it.

---

## 6. Sensor Strategy

### Ultrasonic

Primary purpose:

* Obstacle detection
* Directional distance measurement
* Environmental scanning

Limitations:

* Measurement noise
* Limited angular resolution
* Wide beam
* Surface-dependent reflections
* Sensor interference (both sensors on the same head)

The two HC-SR04s are mounted on one rotating head, facing **opposite directions**
(180° apart). One servo sweeps the head 180°; each sensor covers its own 180° arc,
giving combined 360° coverage (see `docs/decisions.md`, Decided #21).

---

## 7. Rotating Sensor Head

One servo rotates a head carrying two ultrasonic sensors facing opposite ways.

```text
      [US1]   (front-facing)
        │
    [HEAD / SERVO]  sweeps 0–180° (GPIO17)
        │
    ┌──────────────┐
    │     CAR      │
    └──────────────┘
        │
      [US2]   (rear-facing, opposite to US1)
```

* US1 and US2 face 180° apart, so as the head sweeps 0–180° the pair covers 360°.
* Both sensors fire sequentially (never simultaneously) to avoid crosstalk.

---

## 8. Sensor Synchronization

Multiple ultrasonic sensors must not transmit simultaneously.

Measurements should be scheduled sequentially to reduce interference.

```text
Sensor 1
   ↓
Measurement
   ↓
Wait
   ↓
Sensor 2
   ↓
Measurement
```

---

## 9. Localization Hardware

No final localization hardware decision has been made.

The leading candidate is a wheel encoder on the rear drive motor combined with the
measured steering angle (bicycle-model odometry). Status: **TBD**.

### Wheel encoder

Advantages:

* Simple
* Cheap
* Directly related to movement

Disadvantages:

* Wheel slip
* Accumulated error

### IMU

Advantages:

* Useful orientation information
* Inexpensive

Disadvantages:

* Drift
* Poor standalone position estimation
