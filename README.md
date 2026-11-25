# Autonomous Car

> **Branch status — `feat/phone-controlled-car`.** This README still describes
> the `main`-line research platform. The branch pivots to a phone-controlled car
> with an obstacle interlock: its plan of record is
> [`docs/roadmap.md`](docs/roadmap.md) and
> [`docs/safety-matrix.md`](docs/safety-matrix.md), and the full README rewrite
> is tracked in issue #18.

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
Phone browser
     │ Wi-Fi / HTTP
     ▼
HTTP adapter ──► phone-facing protocol ──► safety core ──► hardware adapters
                                               │
                                               └─ rules, state and throttle ramp
```

The safety core is pure logic: it performs no I/O and knows nothing about
HTTP, PWM or pins. The phone-facing protocol sits behind an interface, so a
future transport can replace HTTP without changing a safety rule.

---

## Roadmap — the `main` line

This branch's plan of record is [`docs/roadmap.md`](docs/roadmap.md); the
`main`-line phases are archived at
[`docs/archive/main-line-roadmap.md`](docs/archive/main-line-roadmap.md).

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

## Current Implementation

The branch is in **Phase 2 — Safety core**:

* the ESP32 serves `POST /api/control` and `GET /api/state` over HTTP;
* every request is authenticated with a shared token and a strictly increasing
  sequence counter;
* the transport-neutral protocol sits behind an interface;
* the pure safety core implements the disarmed and normal-driving rows and the
  requested-to-actual throttle ramp;
* the native host suites cover the safety core and protocol without hardware.

The motor adapter and phone page arrive in Phase 3. The obstacle interlock and
physical E-stop arrive in Phase 4; until both exist, driving tests happen with
the wheels off the ground.

Wi-Fi credentials and the shared control token go in `include/config.h`
(gitignored — copy from `include/config.example.h`).

---

## Documentation

* [`docs/architecture.md`](docs/architecture.md) — system architecture, robot state, kinematic model, data flow
* [`docs/hardware.md`](docs/hardware.md) — bill of materials, pin map, power architecture
* [`docs/communication.md`](docs/communication.md) — HTTP control protocol, authentication and state snapshot
* [`docs/local-development.md`](docs/local-development.md) — local setup: firmware config, host tests and HTTP checks
* [`docs/roadmap.md`](docs/roadmap.md) — the branch's phases and their Definitions of Done
* [`docs/safety-matrix.md`](docs/safety-matrix.md) — the safety rules, one row per situation
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
lib/      # firmware modules
include/  # firmware public headers (incl. gitignored config.h)
test/     # host unit tests
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
