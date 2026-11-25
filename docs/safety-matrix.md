# Safety Matrix

The specification of record for the safety core. Every row below is a rule, and
every rule becomes at least one host test **before** it is coded. Writing the
matrix first is what stops the safety core from being whatever the code
happened to do.

> **Agreement gate.** This document and the branch roadmap
> ([`docs/roadmap.md`](roadmap.md)) must be agreed by the maintainer before the
> safety core (#6) is written. Until then, the matrix is a proposal: the
> precedence order in §3 and the steering rule in §4 are recorded as proposals
> #78 and #79 in `docs/decisions.md`, not as decisions.

Units are fixed: distances in **millimetres**, angles in **degrees**, the
requested throttle from **-100** (full reverse) to **+100** (full forward).

---

## 1. The inputs

| Input              | Meaning                                                   | Values                                      |
| ------------------ | --------------------------------------------------------- | ------------------------------------------- |
| **Armed**          | Does the car accept motion intents?                       | yes / no                                    |
| **Latched stop**   | Is a latched stop engaged?                                 | yes (with a reason) / no                    |
| **E-stop**         | Is the physical E-stop button pressed?                     | pressed / released                          |
| **Car link**       | The live connection between the phone and the car, proven by a recent heartbeat (the heartbeat rides the control-intent channel). | fresh / stale |
| **Obstacle**       | The frontal distance against the stop threshold.           | clear / near                                |
| **Battery state**  | The pack's measured voltage.                               | ok / low / critical                         |

"–" in a cell means the condition is **not part of this row**. The row's
outcome holds when the listed conditions are active and the others are in their
neutral state. When a condition marked "–" is active at the same time, the
precedence in §3 decides, and the row that owns that condition describes the
car instead.

## 2. The outcomes

Every row states four things:

| Outcome                | Values                                                                   |
| ---------------------- | ------------------------------------------------------------------------ |
| **Motor**              | coast (standby) / drive / brake                                         |
| **Requested throttle** | applied / refused / forced to zero                                       |
| **Actual throttle**    | ramps toward the requested throttle at a bounded rate; never jumps       |
| **Steering angle**     | applied / returns to straight / not applied                              |

A row also states what the state snapshot reports: `armed`, `latched`,
`latchReason`, `estop`, `interlock`, the obstacle distance, the battery state,
the actual throttle and the steering angle.

## 3. Precedence

When more than one condition holds, the first rule that matches wins:

1. **E-stop pressed → brake.** Nothing else is consulted.
2. **Latched stop engaged → brake.** The latch is set by the phone or by a
   critical battery.
3. **Disarmed → coast.** No throttle, no steering.
4. **Car link stale → coast.** The last requested throttle is dropped, and the
   steering angle returns to straight.
5. **Obstacle near → the requested throttle is refused.** The motor coasts and
   the steering angle is still applied.
6. **Otherwise → the requested throttle applies**, slewed.

Rules 3 and 4 have the same motor outcome — coast, no throttle, no steering.
They differ only in what the snapshot reports: `armed: false` against
`linkStale: true`. A stale link expires the arming as well (T2), so the two
are usually true at once.

Two consequences worth stating outright:

* A **critical battery** enters through rule 2: it *sets* a latched stop whose
  reason is the battery, so the car brakes rather than coasts.
* A **low battery** only warns. It never changes the motor state.

The interlock (rule 5) only engages when the car would otherwise be driving.
A car that is already stopped for another reason is not "interlocked" — it is
stopped.

---

## 4. The matrix

| ID  | Armed | Latched stop | E-stop | Car link | Obstacle | Battery   | Motor | Requested throttle | Actual throttle  | Steering angle           | Reported in the snapshot                              | Ticket |
| --- | ----- | ------------ | ------ | -------- | -------- | --------- | ----- | ------------------ | ---------------- | ------------------------ | --------------------------------------------------- | ------ |
| S1  | no    | –            | –      | –        | –        | –         | coast | forced to zero     | ramps to zero    | not applied — returns to straight | `armed: false`                                  | #6, #9 |
| S2  | yes   | no           | no     | fresh    | clear    | ok / low  | drive | applied             | slewed toward the requested throttle | applied                 | `armed: true`                                    | #6, #8 |
| S3  | yes   | no           | no     | stale    | –        | –         | coast | forced to zero     | ramps to zero    | not applied — returns to straight | `linkStale: true`                               | #10 |
| S4  | yes   | **yes**      | no     | –        | –        | –         | brake | forced to zero     | ramps to zero    | not applied              | `latched: true`, `latchReason`                    | #12 |
| S5  | yes   | no           | **yes** | –      | –        | –         | brake | forced to zero     | ramps to zero    | not applied              | `estop: true`                                     | #13 |
| S6  | yes   | no           | no     | fresh    | **near** | ok        | coast | **refused**        | ramps to zero    | **still applied**       | `interlock: true`, `distanceMm`                   | #11 |
| S7  | yes   | **yes** — set by the firmware | no | fresh | clear | **critical** | brake | forced to zero | ramps to zero | not applied | `latched: true`, `latchReason: "battery"` | #15 |
| S8  | yes   | no           | no     | fresh    | clear    | ok        | coast | forced to zero (the phone's STOP) | ramps to zero | applied | `armed: true` | #12 |

Notes on the rows:

* **S2** is the only row where the motor is driven. The motor is in coast
  rather than drive when the requested throttle is zero — a stopped motor
  draws nothing. A **low battery** warns and changes nothing else, so S2 covers
  both `ok` and `low`.
* **S3** states the stale-link rule on its own: the throttle is zero and the
  motor coasts whether or not the car was armed. The same event also expires
  the arming (T2), so the car settles in S1 as well — the two rules are
  independent, and both are reported.
* **S6** is the only row where the steering angle is still applied while the
  throttle is refused. That is what makes it an interlock rather than a stop.
* **S5** is cleared only by physically releasing the E-stop button. The phone
  cannot clear it, and the link stays alive so the page can keep reporting the
  state.
* **S7** is a latched stop with the battery as its reason; the phone clears it
  the same way it clears any other latched stop, and the firmware also clears
  it by itself when the pack recovers above the low-battery threshold.
* **S8** is S2 with a zero requested throttle: the phone's STOP zeroes the
  throttle, the motor coasts, and the car stays armed.

### The three stops

| Stop                     | What it does                                                | How it clears                                |
| ------------------------ | ----------------------------------------------------------- | --------------------------------------------- |
| **Soft stop** (the phone's STOP) | The requested throttle goes to zero; the motor coasts; the car stays armed | The next control intent, or arming again |
| **Latched stop** (the phone, or a critical battery) | The motor brakes and stays braked                       | An explicit confirmation on the phone         |
| **Physical E-stop**      | The motor brakes immediately                                 | Physically releasing the button — never from the phone |

## 5. The transitions

The rules are not only the rows; they are also the moves between them.

| ID  | From       | Event                          | To         | Rule                                                                   | Ticket |
| --- | ---------- | ------------------------------ | ---------- | ---------------------------------------------------------------------- | ------ |
| T1  | disarmed   | The phone arms the car          | armed      | The phone arms the car with a press-and-hold. The arming lasts until the car link goes stale | #9 |
| T2  | armed      | The car link goes stale         | disarmed   | Arming expires with the car link, so a dropped phone cannot leave the car armed | #9, #10 |
| T3  | disarmed   | The car link becomes fresh      | disarmed   | A fresh link does not arm the car by itself; the phone has to arm it again (T1) | #9, #10 |
| T4  | armed      | The interlock engages           | S6         | The obstacle comes within the stop threshold; the closest of the last five measurements is used | #11 |
| T5  | S6         | The obstacle clears             | S2         | The distance goes back above the stop threshold                          | #11 |
| T6  | S2         | The phone sets the latched stop | S4         | The motor brakes and stays braked until explicitly cleared              | #12 |
| T7  | S4         | The phone clears the latched stop with an explicit confirmation | S2 | The latch is released only on confirmation                               | #12 |
| T8  | S4         | The phone clears the latch while the E-stop is pressed | S4 | Clearing is refused while the E-stop is pressed                          | #12, #13 |
| T9  | S2         | The E-stop is pressed           | S5         | The motor brakes immediately; the link stays alive                       | #13 |
| T10 | S5         | The E-stop is released          | disarmed   | Releasing the button does not re-arm the car; the phone must arm it again | #13 |
| T11 | S2         | The pack falls to critical      | S7         | The firmware sets a latched stop whose reason is the battery           | #15 |
| T12 | S7         | The pack recovers above the low-battery threshold | S2 | The firmware clears the battery latch by itself                         | #15 |
| T13 | S2         | The pack falls to low           | S2 (battery: low) | The car warns; it keeps driving                                  | #15 |
| T14 | S2         | The phone presses STOP          | S8         | The requested throttle goes to zero; the car stays armed               | #12 |

## 6. Statements that must not be broken

These are the properties the matrix exists to protect. Each one is a test.

1. **The interlock refuses throttle but leaves the steering angle applied.** The
   driver keeps the ability to steer around whatever is in the way; the car
   never chooses a different path on its own.
2. **A stale car link coasts rather than brakes.** Braking on a dropped link
   would be abrupt.
3. **The interlock is the only protection the motor driver has against a
   stalled motor.** The TC1508A has no current limiting, and a stalled motor
   dissipates more than twice its maximum (DECIDED #70). The interlock is what
   keeps the bridge from cooking.
4. **The car is stopped and disarmed on boot.** It never moves on its own.
5. **The phone cannot clear the physical E-stop.** Only releasing the button
   does.
6. **The latched stop needs an explicit confirmation** to clear.
7. **The actual throttle never jumps.** It ramps toward the requested throttle
   at a bounded rate, in both directions and across zero.
8. **Time is injected, never waited on.** Every timing rule is tested
   instantly and deterministically.

## 7. The test plan

Every row and every transition above becomes at least one host test, driven
through the safety core's public interface. A test asserts what the car *does*
— the motor state, the throttle it outputs, the flags it reports — never
internal variables or the order in which adapters are called.

| Matrix rows | Test suite                     | Ticket |
| ------------ | ------------------------------ | ------ |
| S1, S2 and the throttle ramp | `test/test_car_control/` | #6 |
| T1 (arming)                  | `test/test_car_control/`         | #9 |
| S3 and T2, T3                 | `test/test_car_control/`         | #10 |
| S4, S8 and T6, T7, T8, T14    | `test/test_car_control/`         | #12 |
| S5 and T9, T10                | `test/test_car_control/`         | #13 |
| S6 and T4, T5                  | `test/test_car_control/`         | #11 |
| S7 and T11, T12, T13           | `test/test_car_control/`, `test/test_battery_monitor/` | #15 |

The control-intent parser — valid intents, out-of-range values, malformed
payloads, stale and replayed sequence numbers — is covered separately in
#6. The battery monitor's voltage-to-percentage conversion is covered across
the whole discharge curve in #15.

## 8. Agreement

The maintainer agrees this matrix by confirming that:

* every row's outcome is the behaviour they expect;
* the precedence order in §3 is the order they expect;
* the statements in §6 hold.

Until they do, the safety core is not written. The matrix may then be amended
by adding a row, never by rewriting history — see the change policy in
`docs/decisions.md`.
