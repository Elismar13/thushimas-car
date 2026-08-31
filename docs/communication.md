# Communication Protocol

## 1. Purpose

The communication layer connects the robot controller to an external computer.

The protocol supports:

* Commands
* Telemetry
* Sensor measurements
* Robot state
* Configuration

---

## 2. Transport

**DECIDED:** Wi-Fi + MQTT, JSON payloads.

```text
ESP32 ←──── Wi-Fi / MQTT ────→ PC (broker)
```

Two modes, **router mode first**:

* Primary: ESP32 joins an existing Wi-Fi network; the PC runs the MQTT broker (e.g. Mosquitto).
* Fallback: ESP32 starts its own access point so the PC can connect directly (diagnostics / no-router).

The MQTT broker runs on the PC. Default QoS 0; QoS 1 may be used for safety commands.

### Proposed topic structure

```text
car/cmd          # PC → ESP32: commands (QoS 1)
car/telemetry    # ESP32 → PC: periodic state (QoS 0)
car/scan         # ESP32 → PC: scan datasets (Phase 4+)
car/status       # ESP32 → PC: connection / health / errors
```

---

## 3. Command Model

Commands are JSON messages published to `car/cmd`.

Throttle is a signed percentage (`-100` = full reverse, `0` = stop, `+100` = full forward).
Steering is a signed angle in degrees, clamped to the platform's `±δ_max`.

```json
{
  "type": "command",
  "command": "THROTTLE",
  "throttle": 40,
  "timestamp": 1234
}
```

```json
{
  "type": "command",
  "command": "STEER",
  "angle": -15,
  "timestamp": 1235
}
```

Possible commands:

```text
THROTTLE    { throttle }            # -100 .. +100, 0 = stop
STEER       { angle }               # degrees, clamped to ±δ_max
STOP        { }                     # throttle = 0
ESTOP       { }                     # latched emergency stop; cleared by RESET_ESTOP
RESET_ESTOP { }                     # re-arms the car after an E-stop
SET_SPEED_LIMIT { throttleLimit }   # cap on throttle magnitude
START_SCAN  { }
STOP_SCAN   { }
PING        { }                     # liveness check
```

> The differential-drive vocabulary (MOVE_FORWARD / TURN_LEFT / TURN_RIGHT) is **not used**.
> Movement is expressed as throttle + steering angle.

---

## 4. Telemetry

Telemetry is published periodically to `car/telemetry`.

```json
{
  "type": "telemetry",
  "timestamp": 1234,
  "mode": "MANUAL",
  "throttle": 40,
  "steeringAngle": -15,
  "speed": 0.42,
  "battery": 7.2,
  "estop": false,
  "position": {
    "x": 0,
    "y": 0,
    "theta": 0
  }
}
```

Fields will be added as the project evolves. `position` is only meaningful once localization exists (Phase 7).

---

## 5. Sensor Measurement

A distance measurement:

```json
{
  "type": "distance",
  "sensor": "ultrasonic_1",
  "angle": 90,
  "distance": 123,
  "timestamp": 1234
}
```

Units:

```text
distance   → millimeters
angles     → degrees
```

`sensor` values: `ultrasonic_1`, `ultrasonic_2` (mounted on the rotating head).

---

## 6. Reliability

Safety must not depend on the PC.

* **Connection timeout:** if no command is received within a timeout window (e.g. 500 ms), the ESP32 stops the car (throttle = 0).
* **Physical E-stop:** a wired push button immediately stops the motor, independent of software/network.
* **Software E-stop:** an `ESTOP` command latches the stop until `RESET_ESTOP`.
* **Reconnect:** the ESP32 should reconnect to Wi-Fi/MQTT automatically; while disconnected it remains stopped.

```text
PC connection lost
        ↓
ESP32 detects timeout
        ↓
STOP (throttle = 0)
```

---

## 7. Versioning

The protocol should include a version once it becomes stable.

```json
{
  "protocolVersion": "1.0"
}
```

Breaking protocol changes should result in a new version.
