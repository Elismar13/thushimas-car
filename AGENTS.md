# AGENTS.md

Guidance for AI agents (and humans) working on this repository. Read this file first;
it is a contract for how work should be done here.

---

## 1. What this project is

An experimental **autonomous car** built incrementally. It is **not** a differential-drive rover. The platform is a car:

* One rear DC motor = throttle (forward/reverse) via an L298N.
* One front servo steers both front wheels (bicycle/Ackermann model).
* One second servo rotates a head carrying the two HC-SR04 sensors, mounted facing opposite
  ways so a 180° head sweep covers 360°.
* ESP32 DevKit V1 (WROOM-32), PlatformIO + Arduino framework.
* PC talks to the car over Wi-Fi + MQTT (JSON), broker runs on the PC.

There is **no turn-in-place**. Movement is expressed as throttle (`-100..+100`) and a
steering angle (degrees). Kinematics: `R = L / tan(δ)`.

---

## 2. Read these before doing anything

| File | What it gives you |
| ---- | ----------------- |
| `docs/architecture.md` | System layers, robot state, kinematic model, data flow |
| `docs/hardware.md` | BOM, proposed GPIO pin map, power architecture |
| `docs/communication.md` | MQTT topics, JSON command/telemetry schemas, safety behavior |
| `docs/roadmap.md` | Phase definitions and Definitions of Done (DoD) |
| `docs/decisions.md` | `DECIDED` choices vs open `TBD` items + change policy |
| `docs/developer/index.md` | Developer reference: modules, data flows, state machines, integrations, build & test |

**Golden rule:** treat `docs/decisions.md` as the source of truth for what is decided.
When a doc conflicts with code, the doc should be updated too — never silently diverge.

---

## 3. Handling DECIDED vs TBD

* Build for `DECIDED` items without re-litigating them.
* For `TBD` items: **ask the user** — do not guess. Guessing a chassis, wheelbase `L`,
  `δ_max`, motor specs, or sensor-head layout will propagate wrong assumptions into
  kinematics, odometry, and path planning.
* If you discover a new open question, add it to `docs/decisions.md` (Open table).
* If you resolve a TBD, move it to Decided and update every affected doc.

---

## 4. Repository layout

```text
src/       # firmware source (single evolving PlatformIO project)
lib/       # firmware libraries
include/   # firmware public headers (incl. gitignored config.h)
test/      # unit tests
tools/     # PC-side tooling (MQTT viewer, mock publisher)
docs/      # project documentation (see §2)
platformio.ini
```

Phases are **not** separate folders. Each phase milestone is a git tag
(e.g. `phase-1-throttle-steering`); phase definitions live in `docs/roadmap.md`.

---

## 5. Build & test commands

PlatformIO is available at `~/.platformio/penv/bin/pio` (not on PATH by default):

```bash
~/.platformio/penv/bin/pio run                 # compile firmware (esp32dev)
~/.platformio/penv/bin/pio run -t upload       # compile + flash via USB
~/.platformio/penv/bin/pio monitor             # serial monitor (115200)
~/.platformio/penv/bin/pio test -e native --without-uploading  # host unit tests
```

* Always run `pio run` after changing firmware code — the task is not done until it compiles.
* `pio test` currently errors because `test/` is empty; add real tests before running it.
* `platformio.ini` must remain parseable — the file is strict INI. A broken comment/section
  header will fail every build (this happened once already).

---

## 6. Code conventions

### Firmware (C++, Arduino for ESP32)

* Target the environment `[env:esp32dev]` in `platformio.ini`.
* Prefer mature external libraries for complex peripherals; simple peripherals may use
  thin hand-rolled drivers. Keep stable units (mm, degrees) in `lib/` wrappers — the rest
  of the code never includes the library headers directly. External deps:
  `teckel12/NewPing` (HC-SR04) and `madhephaestus/ESP32Servo` (servos).
* Add new libraries via `lib_deps` in `platformio.ini`; never vendor library code into `lib/`.
* Respect the pin map in `docs/hardware.md` (§3). Never assign a pin already in use.
* HC-SR04 ECHO is 5 V; level-shift (resistor divider) — do not feed 5 V into a GPIO.
* Servos: 50 Hz PWM (~1–2 ms pulse). L298N: set IN1/IN2 before enabling ENA.
* Fire the two ultrasonic sensors **sequentially**, never simultaneously.
* Put shared/unit-testable logic in `lib/`; keep `src/main.cpp` as wiring/glue.
* No trailing secrets: Wi-Fi credentials and MQTT broker config belong in a config header
  that is gitignored or clearly marked, never hard-coded into committed code.

### PC side (Python)

* Phase 5+ tooling: `paho-mqtt`, `numpy`, `matplotlib`/`PyQtGraph`.
* Keep protocol payloads exactly matching `docs/communication.md` schemas.

### General

* No speculative features — implement only what the current phase's DoD requires.
* Keep units consistent: distances in **millimeters**, angles in **degrees**.
* Update docs in the same change as the code that affects them.

---

## 7. Safety invariants (never break these)

1. Safety must not depend on the PC. The ESP32 stops itself on:
   * command timeout (~500 ms),
   * physical E-stop button (active-low, INPUT_PULLUP),
   * latched `ESTOP` command until `RESET_ESTOP`,
   * obstacle below the threshold.
2. Default on boot: **stopped**. Never start moving on power-up.
3. Wi-Fi/MQTT reconnect while stopped; the car never drives without a live link.
4. Never remove or bypass the E-stop path, software or physical, in any phase.

---

## 8. Working within the roadmap

Follow the philosophy from `README.md`: **Build → Test → Document → Stabilize → Expand.**

* Do not begin a phase whose previous phase's DoD is not met.
* Do not jump ahead to later-phase functionality (no path planning in Phase 1).
* When starting a phase, re-read its Objective and DoD in `docs/roadmap.md`; implement
  exactly the deliverables listed.

---

## 9. Commits

* Follow the repo's existing commit style (short imperative subject, e.g.
  `init: init platform io project`).
* Never commit secrets, `.pio/`, or IDE-generated files (`.gitignore` covers these).
* Only commit when the user asks.

---

## 10. Branch: `feat/phone-controlled-car`

This branch pivots the project to a **phone-controlled car with an obstacle
interlock**. `main` remains the autonomous-navigation research line and is left
intact.

* `docs/decisions.md` opens with a **Branch Overrides** table. It lists which
  `main`-line decisions this branch replaces and which carry forward unchanged.
  **Build for the replacements.** Sections 4, 6 and 7 of this file still describe the
  `main`-line architecture (MQTT, the L298N, two ultrasonic sensors) — treat the
  Branch Overrides table as authoritative wherever the two disagree, until the doc
  surgery lands.
* Read `CONTEXT.md` (the glossary) before writing code, tests, issues or docs. Use
  its vocabulary; do not drift to the synonyms it lists under `_Avoid_`.
* Read `docs/adr/0001-pivot-to-phone-controlled-car.md` for the rationale.

---

## 11. Agent skills

### Issue tracker

Issues and specs are tracked as GitHub issues on `Elismar13/thushimas-car`. See
`docs/agents/issue-tracker.md`.

### Triage labels

The five canonical triage roles, default strings. See
`docs/agents/triage-labels.md`.

### Domain docs

Single-context: root `CONTEXT.md` plus `docs/adr/`. See `docs/agents/domain.md`.
