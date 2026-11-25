# AGENTS.md

Guidance for AI agents (and humans) working on this repository. Read this file first;
it is a contract for how work should be done here.

---

## 1. What this project is

On this branch, an experimental **phone-controlled car** with an obstacle
interlock. It is **not** a differential-drive rover. The platform is a car:

* One rear DC motor provides throttle (forward/reverse) through a TC1508A bridge.
* One front servo steers both front wheels (bicycle/Ackermann model).
* One fixed frontal HC-SR04 refuses throttle when an obstacle is near.
* ESP32 DevKit V1 (WROOM-32), PlatformIO + Arduino framework.
* A phone browser talks to the car over Wi-Fi + HTTP; there is no PC or broker.

There is **no turn-in-place**. Movement is expressed as requested throttle
(`-100..+100`) and a steering angle (degrees). Kinematics: `R = L / tan(δ)`.
The `main` branch retains the abandoned autonomous-navigation research line.

---

## 2. Read these before doing anything

| File | What it gives you |
| ---- | ----------------- |
| `docs/architecture.md` | System layers, robot state, kinematic model, data flow |
| `docs/hardware.md` | BOM, the branch's GPIO pin map (§3), power architecture, wiring |
| `docs/communication.md` | HTTP control endpoint, token authentication, JSON schemas, safety behavior |
| `docs/roadmap.md` | Phase definitions and Definitions of Done (DoD) |
| `docs/safety-matrix.md` | The safety rules as a matrix — one row per situation, one test per row |
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
docs/      # project documentation (see §2)
platformio.ini
```

Phases are **not** separate folders. Each phase milestone is a git tag
(e.g. `phase-1-bench`); phase definitions live in `docs/roadmap.md`.

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
* Run the full native suite before completion; the safety core and protocol have host tests.
* `platformio.ini` must remain parseable — the file is strict INI. A broken comment/section
  header will fail every build (this happened once already).

---

## 6. Code conventions

### Firmware (C++, Arduino for ESP32)

* Target the environment `[env:esp32dev]` in `platformio.ini`.
* Prefer mature external libraries for complex peripherals; simple peripherals may use
  thin hand-rolled drivers. Keep stable units (mm, degrees) in `lib/` wrappers — the rest
  of the code never includes the library headers directly. External deps:
  `teckel12/NewPing` (HC-SR04), `madhephaestus/ESP32Servo` (servos), and
  `bblanchon/ArduinoJson` (confined to `lib/ControlProtocol`).
* Add new libraries via `lib_deps` in `platformio.ini`; never vendor library code into `lib/`.
* Respect the pin map in `docs/hardware.md` (§3). Never assign a pin already in use.
* HC-SR04 ECHO is 5 V; level-shift (resistor divider) — do not feed 5 V into a GPIO.
* Servos: 50 Hz PWM (~1–2 ms pulse). The TC1508A has no enable pin; use its
  `INA`/`INB` truth table and pass through standby before changing direction.
* The branch has one fixed frontal ultrasonic sensor; it is not fired
  simultaneously with another sensor.
* Put shared/unit-testable logic in `lib/`; keep `src/main.cpp` as wiring/glue.
* No trailing secrets: Wi-Fi credentials and `CONTROL_TOKEN` belong in a config
  header that is gitignored or clearly marked, never hard-coded into committed code.

### Phone-facing protocol

* Keep request and response payloads exactly matching `docs/communication.md`.
* Transport adapters implement `ControlEndpoint`; safety rules stay in
  `lib/CarControl` and do not depend on HTTP.

### General

* No speculative features — implement only what the current phase's DoD requires.
* Keep units consistent: distances in **millimeters**, angles in **degrees**.
* Update docs in the same change as the code that affects them.

---

## 7. Safety invariants (never break these)

1. Safety must not depend on the phone. The ESP32 stops itself on:
   * a stale car link,
   * the physical E-stop button (active-low, `INPUT_PULLUP`),
   * a latched stop until it is explicitly cleared,
   * an obstacle within the stop threshold.
2. Default on boot: **stopped and disarmed**. Never start moving on power-up.
3. Wi-Fi reconnects while stopped; the car never drives without a live car
   link.
4. The actual throttle always ramps at a bounded rate and never jumps.
5. Never remove or bypass the E-stop path, software or physical, in any phase.

---

## 8. Working within the roadmap

Follow the philosophy from `README.md`: **Build → Test → Document → Stabilize → Expand.**

* Do not begin a phase whose previous phase's DoD is not met.
* Do not jump ahead to later-phase functionality. The roadmap's phases are a
  dependency order, not a wish list.
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
  **Build for the replacements.** The branch uses HTTP, a TC1508A and one
  frontal ultrasonic sensor; the current implementation and its seams are in
  `docs/architecture.md` and `docs/developer/`.
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
