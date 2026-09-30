# Autonomous Car

An experimental autonomous mobile robot designed to progressively evolve from a remotely controlled car into an autonomous indoor navigation platform.

The platform is a **car**: a single rear motor provides throttle (forward/reverse) and a front servo steers the front wheels (bicycle/Ackermann model). It is not a differential-drive rover — there is no turn-in-place.

The project focuses on robotics fundamentals such as:

* Embedded systems
* Throttle & steering control
* Sensor acquisition
* Obstacle avoidance
* Distance measurement
* Environment mapping
* Localization
* Sensor fusion
* Path planning
* Autonomous navigation

The project is intentionally divided into small, independently achievable milestones.

---

## Vision

Build a small indoor autonomous car capable of:

1. Moving autonomously (throttle + steering).
2. Detecting obstacles.
3. Scanning its surroundings (rotating sonar head).
4. Building a 2D representation of the environment.
5. Estimating its own position.
6. Exploring an unknown environment.
7. Receiving a target coordinate from a user.
8. Planning a collision-free path.
9. Navigating autonomously to the selected destination.

The final system should allow a user to visualize the car and its environment and select a destination on the generated map.

---

## Project Philosophy

This project is intentionally incremental.

The final goal is complex, but each intermediate milestone must produce a working system.

The project should follow this principle:

> Build → Test → Document → Stabilize → Expand.

A new phase should only begin when the current phase has a clearly defined Definition of Done.

Avoid implementing future functionality prematurely.

---

## High-Level Architecture

```text
                         USER
                          │
                          ▼
                  ┌───────────────┐
                  │ Visualization │
                  │   & Control   │
                  └───────┬───────┘
                          │
                     Wi-Fi / MQTT
                          │
                          ▼
                  ┌───────────────┐
                  │     Robot     │
                  │               │
                  │    ESP32      │
                  └───────┬───────┘
                          │
          ┌───────────────┼────────────────┐
          │               │                │
          ▼               ▼                ▼
      Actuators         Sensors        Communication
          │               │
          ▼               ▼
   Rear motor       2× Ultrasonic /
   2× servos        E-stop / encoder
```

---

## Roadmap

| Phase | Objective                  | Status |
| ----- | -------------------------- | ------ |
| 0     | Project foundation         | ⬜      |
| 1     | Throttle & steering control | ⬜      |
| 2     | Remote control             | ⬜      |
| 3     | Obstacle avoidance         | ⬜      |
| 4     | Rotating sonar             | 🟡      |
| 5     | Sensor visualization       | 🟡      |
| 6     | Environment mapping        | ⬜      |
| 7     | Localization               | ⬜      |
| 8     | Autonomous exploration     | ⬜      |
| 9     | Path planning              | ⬜      |
| 10    | Autonomous navigation      | ⬜      |

---

## Current Phase

**Phase 4/5 — Rotating sonar data transfer + visualization (in progress)**

The current objective is the sonar data path end-to-end, focusing on the
communication layer:

* ESP32 produces structured 360° scans (wiper sweep of the dual opposed
  HC-SR04s) and publishes them to `car/scan` over MQTT.
* `START_SCAN` / `STOP_SCAN` commands drive continuous scanning; the car is
  stopped and silent at boot.
* PC tooling in `tools/` (`sonar_viewer.py`, `mock_scan_publisher.py`) validates
  the protocol. The scan schema in `docs/communication.md` is the durable
  interface; the desktop viewer is only a test harness for the future
  web/mobile UI.
* An on-car 128×64 I2C OLED (SDA=21/SCL=22) shows PC-independent status: Wi-Fi/MQTT
  link, scan count, nearest obstacle. Rendered through a display-agnostic
  interface so the panel can be swapped without touching the render logic.

Wi-Fi credentials and broker address go in `include/config.h` (gitignored —
copy from `include/config.example.h`).

---

## Documentation

* [`docs/architecture.md`](docs/architecture.md) — system architecture, robot state, kinematic model, data flow
* [`docs/hardware.md`](docs/hardware.md) — bill of materials, pin map, power architecture
* [`docs/communication.md`](docs/communication.md) — MQTT/JSON protocol, topics, safety behavior
* [`docs/local-development.md`](docs/local-development.md) — local setup: MQTT broker, PC tools, firmware config, end-to-end checks
* [`docs/roadmap.md`](docs/roadmap.md) — phase definitions and Definitions of Done
* [`docs/decisions.md`](docs/decisions.md) — decided choices vs open (TBD) items
* [`docs/developer/`](docs/developer/index.md) — developer reference: modules, data flows, state machines, integrations, build & test

---

## Future Capabilities

The final car may support:

* Manual driving
* Autonomous obstacle avoidance
* 360° environmental scanning (2 opposed 180° heads)
* 2D occupancy mapping
* Car localization
* Autonomous exploration
* Coordinate-based navigation
* Path planning
* Real-time map visualization
* Telemetry
* Remote commands

---

## Non-Goals

The project does not initially aim to implement:

* Outdoor GPS navigation
* High-speed autonomous driving
* Computer vision-based object recognition
* Autonomous manipulation
* Industrial-grade navigation accuracy
* Safety-critical autonomous operation

These may be considered future experiments but are outside the initial scope.

---

## Repository Structure

A single evolving PlatformIO firmware:

```text
src/      # firmware source
lib/      # libraries
include/  # firmware public headers (incl. gitignored config.h)
tools/    # PC-side tooling (MQTT viewer, mock publisher)
test/     # unit tests
docs/     # project documentation
```

Each phase milestone is tracked with a git tag (e.g. `phase-1-motor-control`) rather than
a separate project folder. Phase definitions and Definitions of Done live in `docs/roadmap.md`.

---

## License

TBD.

---

## Project Status

Early development.

Hardware and software architecture are expected to evolve throughout the project.
