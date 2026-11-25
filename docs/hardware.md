# Hardware

The hardware of the **phone-controlled car** on branch `feat/phone-controlled-car`.
This document is the authority for the bill of materials, the pin map, the power
architecture and the wiring. If it disagrees with the firmware, the firmware is
wrong.

Decisions referenced here (`DECIDED #n`) are recorded in `docs/decisions.md`.

---

## 1. Motion Model

The **car** has one rear motor and one front steering servo — it is not a
differential drive:

* A single rear DC motor drives the rear wheels through a differential axle
  (**throttle**: forward and reverse).
* One front servo steers both front wheels together (bicycle/Ackermann
  steering).
* There is **no turn-in-place**. Every turn has a turning radius.

The car is driven by hand from a phone. It has one frontal distance sensor, and
it refuses to drive into things — it never steers itself away from them.

---

## 2. The Car

| Component             | Spec                                            | Purpose                        | Status  |
| --------------------- | ----------------------------------------------- | ------------------------------ | ------- |
| Controller            | ESP32 DevKit V1 (WROOM-32)                      | Control and safety core        | DECIDED |
| Motor driver          | TC1508A (SOP-16), channel A used                | Throttle (forward/reverse)     | DECIDED |
| Rear motor            | 5 V DVD-drive DC motor, ~500 mA, 2 conductors   | Throttle                       | DECIDED |
| Steering servo        | 1× SG90 micro servo, travel `[40°, 140°]`       | Steering angle                 | DECIDED |
| Distance sensor       | 1× HC-SR04, frontal                             | Interlock distance              | DECIDED |
| Battery pack          | 2× 18650 in **parallel** (1S2P), 3.7 V nominal  | The only energy source         | DECIDED |
| Regulation            | One 5 V step-up converter, single rail          | Power for every load           | DECIDED |
| E-stop                | 1× normally-open push button                    | Physical stop                  | DECIDED |
| Power switch          | 1× maintained SPST toggle, ≥5 A DC              | Kills the car completely        | DECIDED |
| Main fuse             | 1× 5 A slow-blow, inline                        | Pack protection                | DECIDED |
| Chassis               | Car-style, steerable front wheels               | Mechanical base                 | TBD     |
| Cell capacity         | mAh of each 18650                               | Endurance                       | TBD     |

**Status legend:** `DECIDED` = committed choice, agents build for it. `TBD` = open
question, recorded in `docs/decisions.md` (Open table) and resolved on the bench.

### Removed on this branch

The rotating sensor head, the second HC-SR04 and the OLED are **gone**
(DECIDED #44, #45). Their pins are free — see §3.

---

## 3. Pin Map

Allocation for the ESP32 DevKit V1 (WROOM-32).

| Signal                          | GPIO | Direction | Notes                                                                            |
| ------------------------------- | ---- | --------- | -------------------------------------------------------------------------------- |
| Motor driver `INA` (forward)    | 32   | out       | 16 kHz LEDC PWM duty is the throttle (DECIDED #65, #69)                            |
| Motor driver `INB` (reverse)    | 33   | out       | PWM in reverse, held static in forward                                            |
| Steering servo                  | 16   | out       | 50 Hz PWM, ~1–2 ms pulse; travel `[40°, 140°]`, 90° = wheels straight             |
| Ultrasonic `TRIG`               | 25   | out       | 3.3 V logic; see the caveat in §7                                                  |
| Ultrasonic `ECHO`               | 26   | in        | **5 V → 3.3 V divider is mandatory** (10 kΩ / 15 kΩ, §7)                            |
| E-stop button                   | 4    | in        | `INPUT_PULLUP`, active low, button to GND (DECIDED #64, #76)                       |
| Battery divider tap             | 35   | in (ADC)  | **ADC1_CH7** — ADC1 is mandatory because ADC2 is unusable while Wi-Fi is on (DECIDED #72) |
| Debug UART                      | 1/3  | —         | Built-in USB serial                                                                |
| Free (input-only, ADC1_CH6)     | 34   | —         | Was the encoder input; no encoder on this branch                                    |
| Free                             | 17   | —         | Freed by removing the sensor-head servo                                            |
| Free                             | 21/22| —         | Freed by removing the OLED's I²C bus                                               |
| Free                             | 13/14/27 | —      | Freed by removing the L298N (ENA/IN1/IN2)                                          |
| Free                             | 36–39| —         | Input-only, ADC1 — available for future inputs                                     |

### Constraints

* **GPIO 6–11 are flash pins** — never used.
* **GPIO 0, 2, 5, 12, 15 are strapping pins** — not used for outputs. GPIO 4 is
  not a strapping pin, but the E-stop's internal pull-up holds the line high at
  boot, so the button must not be held down while powering up.
* **GPIO 13–15 are shared with the SDIO bus** on DevKit variants that carry
  PSRAM or SDIO — leave them unused on those boards.
* **GPIO 34–39 are input-only** and have **no internal pull-up or pull-down** —
  they are used only for inputs. The battery tap uses GPIO 35 (ADC1_CH7); an
  analog input needs no pull-up.
* **ADC2 is unusable while Wi-Fi is on**, so the battery divider must sit on
  ADC1 (GPIO 32–39).
* **No ESP32 pin is 5 V tolerant.** The HC-SR04's 5 V echo must be divided down
  before it reaches GPIO 26.
* **The motor driver has no enable pin.** `INA`/`INB` are the only controls, and
  the firmware must leave the bridge in standby (`0,0`) before changing direction
  (DECIDED #67).

---

## 4. Bill of Materials

Every part the car needs, with the reason it is there. Resistors are 1 % metal
film; capacitors are rated for 10 V or more.

| Qty | Part                                     | Spec / example                                          | Why it is there                                                       |
| --- | ---------------------------------------- | ------------------------------------------------------- | --------------------------------------------------------------------- |
| 1   | ESP32 DevKit V1 (WROOM-32)               | 30-pin, USB                                             | Controller: safety core, HTTP control API, OTA (DECIDED #10)           |
| 1   | Motor driver                             | **TC1508A**, SOP-16, dual full bridge                   | Motor throttle; channel A drives the motor, channel C is free (DECIDED #59) |
| 1   | Rear motor                               | 5 V DC gear motor, ~500 mA, 2 conductors                | Throttle (DECIDED #55)                                                 |
| 1   | Steering servo                           | SG90 micro servo                                        | Steering angle; travel `[40°, 140°]`, 90° = straight (DECIDED #32)    |
| 1   | Distance sensor                          | HC-SR04                                                 | Frontal distance for the interlock (DECIDED #40, #45)                 |
| 1   | Step-up converter                        | 5 V output, **≥2 A continuous** (3 A / 15 W recommended) | The single 5 V rail (DECIDED #56, #75)                                  |
| 2   | 18650 cells                              | Matched pair, same model and batch                      | The 1S2P pack (DECIDED #50); capacity TBD, record it on the bench      |
| 2   | Cell chargers                            | One charger per cell (or a dual-slot charger with per-cell channels) | Charge each cell separately; never charge a parallel pair from one channel (DECIDED #50) |
| 1   | Maintained power switch                  | SPST toggle, ≥5 A DC, panel or toggle                   | The physical power cut; kills motor, ESP32, radio and OTA (DECIDED #61, #74) |
| 1   | Fuse + inline holder                     | 5 A slow-blow (T5A)                                     | Protects the pack and the wiring from a short (DECIDED #74)           |
| 1   | E-stop button                            | Normally open, momentary push                           | Physical stop the phone cannot clear (DECIDED #64, #76)                |
| 1   | Electrolytic capacitor                   | 1000 µF / 16 V                                          | Bulk reservoir on the 5 V rail against motor current steps            |
| 1   | Ceramic capacitor                        | 10 µF / 16 V                                            | Local reservoir at the motor driver's supply                           |
| 3   | Ceramic capacitors                       | 100 nF / 50 V                                           | Decoupling: ESP32 supply, servo supply, battery-divider filter        |
| 1   | Resistor                                 | 47 kΩ                                                   | Battery divider, pack side (DECIDED #72, see §7)                       |
| 2   | Resistors                                | 10 kΩ                                                   | Battery divider, ground side; ultrasonic echo divider, sensor side    |
| 1   | Resistor                                 | 15 kΩ                                                   | Ultrasonic echo divider, ground side (DECIDED #73, see §7)             |
| —   | Wiring                                   | Hookup wire, heat-shrink, terminal blocks              | The car is assembled by hand; keep the motor and servo runs short     |

### Why the converter has to be this big

| Load                        | Typical  | Worst case |
| --------------------------- | -------- | ---------- |
| ESP32 DevKit (Wi-Fi TX)     | ~120 mA  | ~300 mA    |
| SG90 steering servo         | ~100 mA  | ~700 mA (stall) |
| HC-SR04                     | ~15 mA   | ~20 mA     |
| TC1508A bridge + motor      | ~10 mA   | ~1.5 A (stalled motor, DECIDED #70) |
| **Total on the 5 V rail**   | ~250 mA  | **~2.5 A** |

A 2 A converter covers normal driving; a stalled motor plus a stalled servo does
not fit in 2 A. **Use a 3 A (15 W) converter**, and set its output to 5.0 V with
a multimeter before anything is connected to it. The bulk capacitor in §5 buys
the time the converter needs to respond.

**The fuse and the switch are sized for the pack side, not the 5 V side.** The
worst case above is 2.5 A × 5 V ≈ 12.5 W out; at ~85 % converter efficiency and
a 3.7 V pack that is about **4 A drawn from the cells**. The fuse and the switch
are therefore rated at **5 A**. The cells themselves can deliver far more than
that — the fuse protects the wiring, not the pack.

---

## 5. Power Architecture

One pack, one converter, one rail.

```text
     ┌──────────────┐
     │ 18650 cell A │──┐
     └──────────────┘  │      PACK 1S2P — 3.7 V nominal, 4.2 V max
     ┌──────────────┐  ├───┬──────────────┬────────────────┐
     │ 18650 cell B │──┘   │              │                │
     └──────────────┘      │              │                │
                   + ──[ 5 A FUSE ]──[ MAIN SWITCH ]──┬───┘
                                                      │  5 V step-up IN+
                                                      │  and the battery tap (§7)
                                                      ▼
                             ┌────────────────────┐
                             │ 5 V STEP-UP        │
                             │ ≥2 A (3 A rec.)    │
                             └─────────┬──────────┘
                                       │ 5.0 V rail
        ┌──────────────────────────────┼──────────────┐
        │                              │              │
        ▼                              ▼              ▼   ▼
   ESP32 VIN (5 V)              SG90 servo       TC1508A   HC-SR04
                                (GPIO 16)        VDD +     VCC (5 V)
                                                motor

   BATTERY TAP — from the switched pack positive (see §6)
   ──────────────────────────────────────────────────────────
   pack (+) ──[47 kΩ]──┬──► GPIO 35 (ADC1_CH7)
                       ├──[100 nF]──► GND
                    [10 kΩ]
                       │
                      GND
```

* The **battery tap is on the switched pack positive**, ahead of the converter.
  The pack voltage is what the cell actually holds; the boosted rail sags under
  load and would read high. Taking the tap after the switch means the divider
  draws nothing at all while the car is switched off.
* There is **no separate motor rail**. The motor, the servos, the sensor and the
  controller all share the 5 V rail, and the bulk capacitor holds it up while
  the converter catches up.
* The 1000 µF electrolytic sits at the 5 V rail, close to the motor driver.
* The pack is **not** designed to be operated while charging (DECIDED #42).

---

## 6. Wiring

```text
  PACK (+) ──[5 A FUSE]──[MAIN SWITCH]──┬── 5 V step-up IN+
                                         └── battery tap (§5)

  5 V step-up OUT+ (5.0 V) ──┬───────── ESP32 VIN
                              ├───────── SG90 servo VCC (red)
                              ├───────── HC-SR04 VCC
                              ├───────── TC1508A VDD
                              └───────── SG90 servo GND / HC-SR04 GND / TC1508A GND
  5 V step-up OUT- (GND) ────┴───────── ESP32 GND

  STEERING SERVO (SG90)
  ─────────────────────────────────────
  GPIO 16 ──► signal (orange/yellow)
  5 V rail ─► VCC (red)
  GND ──────► GND (brown/black)

  MOTOR DRIVER (TC1508A, channel A)
  ─────────────────────────────────────
  GPIO 32 ──► INA            OUTA ──┬──► motor lead 1
  GPIO 33 ──► INB            OUTB ──┴──► motor lead 2
  5 V rail ──► VDD           GND ──────► GND
  (standby = INA 0 / INB 0, coast; brake = both 1)

  ULTRASONIC (HC-SR04)  ── frontal only
  ─────────────────────────────────────
  GPIO 25 ──► TRIG
  ECHO ──[10 kΩ]──┬──► GPIO 26
                  │
                [15 kΩ]
                  │
                 GND

  E-STOP
  ─────────────────────────────────────
  GPIO 4 ──┬─── internal pull-up (INPUT_PULLUP)
           │
        [ E-STOP BUTTON ]  (normally open)
           │
          GND              → reads LOW = pressed

  BATTERY TAP
  ─────────────────────────────────────
  pack (+) ──[47 kΩ]──┬──► GPIO 35 (ADC1_CH7)
                      ├──[100 nF]──► GND
                   [10 kΩ]
                      │
                     GND
```

The TC1508A's SOP-16 pin numbers are not reproduced here — check the datasheet
for the package pinout before soldering.

---

## 7. Signal Conditioning

This section is the authoritative description of both dividers — the values
here are the ones to wire against, and the tables in §4 and §6 refer back to
them.

### Ultrasonic echo: 5 V → 3.3 V

The HC-SR04 drives `ECHO` at 5 V. The ESP32 is not 5 V tolerant, so the echo is
divided:

| Resistor | Value | Connection                        |
| -------- | ----- | --------------------------------- |
| R1       | 10 kΩ | `ECHO` → GPIO 26                  |
| R2       | 15 kΩ | GPIO 26 → GND                     |

`V(GPIO 26) = 5 V × 15 / 25 = 3.0 V` at the worst case — a 0.3 V margin below
the 3.3 V rail. The divider's source impedance is 6 kΩ, which the HC-SR04 can
drive without trouble, and a resistive divider cannot stretch the pulse: the
shortest echo is 58 µs.

### Ultrasonic trigger: 3.3 V → the HC-SR04

A common HC-SR04 module specifies a trigger `VIH` of about `0.7 × Vcc` — 3.5 V
at a 5 V supply — so a 3.3 V trigger sits on the boundary. Most modules accept
it. If the sensor will not trigger reliably, fit a **74AHCT125** between GPIO 25
and the trigger: `VCC` to the 5 V rail, `GND` to ground, `/OE` tied low, the
input from GPIO 25 and the output to `TRIG`. An AHCT input threshold is ~2.0 V,
so 3.3 V logic drives it, and its output is a clean 5 V trigger. Do not put a
buffer on the echo path — the echo is divided down, not buffered up.

### Battery measurement: pack → ADC

| Resistor | Value | Connection                          |
| -------- | ----- | ----------------------------------- |
| R1       | 47 kΩ | pack (+) → GPIO 35                  |
| R2       | 10 kΩ | GPIO 35 → GND                       |
| C1       | 100 nF | GPIO 35 → GND (filter)              |

`V(GPIO 35) = V(pack) × 10 / 57`, so:

| Pack | ADC pin | Comment                          |
| ---- | ------- | -------------------------------- |
| 4.2 V | 0.737 V | Full charge                      |
| 3.7 V | 0.649 V | Nominal                          |
| 3.5 V | 0.614 V | **Low battery warning** (DECIDED #51) |
| 3.1 V | 0.544 V | **Critical battery** — throttle cut (DECIDED #51) |
| 2.5 V | 0.439 V | Pack floor                       |

The divider's source impedance is 47 kΩ ∥ 10 kΩ = 8.3 kΩ, low enough for the
ADC's sample-and-hold, and the 100 nF capacitor supplies its charge between reads
(RC ≈ 0.8 ms). The ESP32's 12-bit ADC resolves 0.8 mV per count, so the pack is
read to about 5 mV. The reading is **checked against a multimeter during the
bench bring-up** (issue #5): the firmware acts on the 3.5 V and 3.1 V
thresholds, so the divider's tolerance must not be allowed to move them.

---

## 8. Motor Driver

The **TC1508A** is a dual-channel integrated-MOSFET full bridge (DECIDED #59).
Channel A drives the motor; channel C is left free. The decision is recorded in
`docs/decisions.md` #59; the values below are the ones to wire and drive
against.

### Truth table

| `INA` | `INB` | Output | State                    |
| ----- | ----- | ------ | ------------------------ |
| LOW   | LOW   | Hi-Z   | **Standby / coast**      |
| HIGH  | LOW   | Driven | **Forward**              |
| LOW   | HIGH  | Driven | **Reverse**              |
| HIGH  | HIGH  | Driven | **Brake**                |

The throttle is realised as the **PWM duty on the active input**, with the other
input held static (DECIDED #65). PWM runs at **16 kHz**, 8-bit (DECIDED #69).

An ordinary stop puts the bridge in **standby**, so the motor coasts and draws
nothing (DECIDED #60). The **latched stop** and the **physical E-stop** brake
instead — both inputs high — so the car stops hard rather than rolling
(DECIDED #68). Leaving the brake state goes through standby first, per #67.

The firmware never flips direction directly: it ramps to zero, coasts, holds
standby for ~50 ms so the back-EMF decays, and only then sets the new direction
(DECIDED #67).

### Absolute limits

| Parameter                    | Value                    |
| ---------------------------- | ------------------------ |
| Supply `VDD`                | 2.4 V – 7.2 V            |
| Input high threshold `VIH`  | 2.0 V (3.3 V logic drives it directly) |
| `R_ON` per MOSFET            | 1.0 Ω typical, 1.6 Ω max |
| Continuous current per channel | 1.8 A                 |
| Peak current per channel    | 2.5 A                    |
| Maximum dissipation         | **1.73 W** (θJA 75 °C/W) |
| Standby current             | ≤ 10 µA                  |

**A stalled motor is the dangerous case.** At the motor's 500 mA the bridge drops
about 0.5 W, which is comfortable. A stalled motor pulls roughly 1.5 A and
dissipates about 4.5 W — more than twice the chip's limit. The chip has **no
current limiting**, so the obstacle interlock is the only protection against
thermal damage (DECIDED #70). Treat a stall as a fault, not a driving style.

---

## 9. Safety Hardware

### E-stop button

A normally-open push button between **GPIO 4** and GND, read with the internal
pull-up, so the pin reads **LOW while the button is pressed** (DECIDED #64,
#76). Pressing it **brakes** the motor immediately — both driver inputs high
(DECIDED #68). **The phone cannot clear it** — only physically releasing the
button does. The link stays alive, so the page keeps reporting the state.

### Power switch and fuse

The maintained toggle switch sits in the pack's positive lead, with the **5 A
slow-blow fuse** ahead of it (DECIDED #61, #74). Cutting the switch kills the car
completely: motor, controller, radio and OTA. It is the nuclear option, and it
is recovered only by flipping it back. The fuse protects the pack and the
wiring if a motor or servo wires short.

---

## 10. Charging

* The pack is **two cells in parallel**, so each cell is charged **on its own**,
  with one charger per cell. Never charge a parallel pair from a single
  channel (DECIDED #50).
* The cells must be **matched** — same model, same batch, same age.
* The car has **no charge-state input** and is **not designed to be driven while
  charging** (DECIDED #42). Cut the main switch before connecting a charger.

---

## 11. Bench Verification

These values are measured on the real car, not derived, and are recorded during
the bench bring-up (issue #5):

* The 5 V rail's actual voltage under load, and the converter's set point.
* The pack's voltage range and the capacity of the cells.
* The pack voltage the firmware reads, **checked against a multimeter**, and any
  correction the reading needs.
* The **minimum starting duty** that breaks the motor away from rest, and the
  **duty at full throttle** — both become configuration constants (DECIDED #66).
* The steering servo's actual travel limits, to confirm `[40°, 140°]`.
* The **idle current** of the parked car, for the standby-sleep work (issue #16).
