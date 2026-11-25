# Development Roadmap — Phone-Controlled Car

The plan of record for `feat/phone-controlled-car`. Five phases, in dependency
order. Each phase states its objective, the tickets it contains, and its
**definition of done**.

* The safety rules live in [`docs/safety-matrix.md`](safety-matrix.md). Every
  matrix row becomes at least one host test before the rule it describes is
  coded.
* The abandoned `main`-line roadmap is archived at
  [`docs/archive/main-line-roadmap.md`](archive/main-line-roadmap.md).
* The exact order of work is tracked by the native issue dependencies in
  GitHub; this roadmap groups them into phases.

**Rules of the road**

* Do not begin a phase whose predecessor's definition of done is not met.
* Do not code a safety rule before its matrix row has a test.
* The safety core is not coded until the roadmap and the safety matrix are
  agreed (issue #4).
* **The car is not driven on the floor until the interlock (#11) and the
  physical E-stop (#13) are in place.** Until then, every driving test happens
  with the wheels off the ground. The obstacle interlock is the only protection
  the motor driver has against a stalled motor (DECIDED #70), and the E-stop is
  never bypassed in any phase (AGENTS.md §7).

---

## Phase 1 — Bench

**Objective:** power, wiring and the hardware measured and confirmed, with the
motor turning and the wheels off the ground.

**Tickets:** #2 (remove the research modules), #3 (the hardware documentation),
#5 (the bench bring-up — the human walks the checklist and closes it).

**Definition of done**

* The bill of materials, the pin map, the power architecture and the wiring
  diagram are written down and match the assembled car (#3).
* The pack, the step-up converter's 5 V rail, the motor driver, the steering
  servo and the frontal ultrasonic are each confirmed against the checklist.
* The motor turns forward and backward at a low duty, with the wheels off the
  ground.
* The measured values are recorded: the 5 V rail under load, the pack's range,
  the **minimum starting duty**, the **duty at full throttle**, and the
  servo's actual travel limits.

**Status:** done — #3 and #5 are closed.

---

## Phase 2 — Safety core

**Objective:** the matrix written down, then the safety core implemented and
green on the host. No hardware in the loop.

**Tickets:** #4 (the plan of record), #6 (the control endpoint and the safety
core's skeleton).

**Definition of done**

* The roadmap and the safety matrix are **agreed** by the maintainer.
* The safety core takes a control intent, the sensor readings and the current
  time, and produces the motor outputs and the car's state. It performs no I/O
  and knows nothing about HTTP, PWM or pins.
* The phone-facing protocol sits behind an interface, so the transport can be
  replaced without touching a safety rule.
* Every implemented matrix row has a host test, including the transitions
  between rows.
* The requested-to-actual throttle ramp is tested in both directions and
  across zero.
* The native test suite is green.

**Status:** complete — #4 was agreed by the maintainer; #6 implements the
transport-neutral control endpoint, the safety-core seam, matrix rows S1–S2,
and the requested-to-actual throttle ramp with host tests.

---

## Phase 3 — Control surface

**Objective:** the motor driver, the HTTP API and the phone's page, so the car can
be driven from a phone.

**Tickets:** #7 (the motor driver and the phone page), #8 (the joystick),
#9 (arming), #10 (the car link's lifetime), #12 (the three stops), #14 (reverse
and the direction-change interlock).

**Definition of done**

* The motor driver reaches forward, reverse, coast and brake, and the requested
  throttle maps to a duty that starts at the measured minimum starting duty.
* The phone page is one self-contained file served by the car. It posts the
  control intent, renders the state snapshot, and holds a screen wake lock
  while driving.
* The joystick sets the requested throttle and the steering angle, and returns
  to zero and straight when released.
* The page arms the car with a press-and-hold, shows whether it is armed, and
  arming expires when the car link goes stale.
* A stale car link coasts the car to a stop, and the page says so.
* The soft stop, the latched stop and the brake behave as the matrix says, and
  clearing the latched stop needs an explicit confirmation.
* Reverse works, and the driver never changes direction directly.
* The car drives from a phone with the wheels off the ground.

---

## Phase 4 — On-car

**Objective:** the real interlock, the E-stop, the battery cut, standby sleep
and over-the-air flashing, validated against the matrix on the actual car.

**Tickets:** #11 (the interlock), #13 (the physical E-stop), #15 (the battery
state and the critical cut), #16 (standby sleep), #17 (over-the-air flashing).

**Definition of done**

* The frontal sensor is polled continuously, the interlock refuses the
  requested throttle within the stop threshold, and the steering angle is still
  applied while it does.
* The physical E-stop brakes the motor immediately, and only physically
  releasing the button releases it.
* The pack voltage is measured through the divider and reported; the car warns
  when the pack is low, cuts the throttle when it is critical, and the cut
  clears itself when the pack recovers.
* The parked car drops to Wi-Fi modem sleep, keeps its association, and serves
  the first request after the idle interval.
* Wireless flashing works with the HTTP server running, and the car does not
  move while it updates.
* Every matrix row has been walked on the real car.
* **Only now does the car drive on the floor** — the interlock and the
  physical E-stop are in place, so the safety paths the earlier phases left
  out are closed before the car is driven in a room.

---

## Phase 5 — Polish

**Objective:** the documentation, the bill of materials and the wiring diagram
describe the car as it actually is.

**Tickets:** #18 (the architecture, the developer reference and the README).

**Definition of done**

* The README, the architecture document, the developer reference, the
  communication document and the local-development document describe the
  branch's actual code, not the `main` line's.
* Every document that still names a removed module or the old protocol has been
  updated.
* The bill of materials and the wiring diagram match the assembled car, and the
  measured values from the bench are recorded.
* The decisions log's branch override table matches what the code does.
