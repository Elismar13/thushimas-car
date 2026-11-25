# Developer Guide — Build & Test

PlatformIO is installed at `~/.platformio/penv/bin/pio`; it is not on `PATH`
by default.

---

## Commands

| Task | Command |
| ---- | ------- |
| Compile firmware | `~/.platformio/penv/bin/pio run` |
| Run all host tests | `~/.platformio/penv/bin/pio test -e native --without-uploading` |
| Run one host suite | `~/.platformio/penv/bin/pio test -e native --without-uploading --filter test_car_control` |
| Flash over USB | `~/.platformio/penv/bin/pio run -t upload` |
| Open the serial monitor | `~/.platformio/penv/bin/pio monitor` (115200 baud) |

The firmware build and the full native suite are both required before a change
is complete.

Wireless flashing is not wired into the current firmware. The retained
`OtaService` module is verified and reconnected by issue #17.

---

## Environments

### `esp32dev`

* ESP32 DevKit V1 (`esp32dev`)
* Arduino framework
* libraries: ArduinoJson, NewPing, ESP32Servo
* framework libraries: Wi-Fi, WebServer, ArduinoOTA

### `native`

* host compiler
* Unity test framework
* `build_src_filter = -<*>` so `src/main.cpp` is excluded
* ArduinoJson is available for the protocol module
* hardware modules are ignored: `ServoControl`, `Ultrasonic`, `OtaService`,
  `HttpApi`

The native environment must contain only hardware-free logic. A library that
includes Arduino or ESP32 APIs belongs in `lib_ignore`.

---

## Host test suites

| Suite | Cases | Covers |
| ----- | ----- | ------ |
| `test_car_control` | 7 | boot/disarmed behaviour, arming, matrix rows S1–S2, throttle ramp up/down/across zero |
| `test_control_protocol` | 15 | valid, malformed, missing, wrong-type and out-of-range intents; token authentication; stale/replayed sequences; state snapshot |
| `test_config` | 4 | committed configuration template, including the control token and absence of broker settings |

Tests use Unity's `TEST_ASSERT_*` macros and a `main()` that calls
`RUN_TEST(...)`.

---

## Adding a testable module

1. Keep the logic hardware-free: no `Arduino.h`, Wi-Fi, WebServer, PWM or pin
   calls.
2. Put it in `lib/<Module>/`.
3. Add `test/test_<module>/test_<module>.cpp` with the same Unity shape.
4. Do not add a hardware-facing module to the native environment; add it to
   `lib_ignore` instead.
5. Run the single suite while developing, then the complete native suite.

Use a fake adapter at the tested module's public interface. Do not assert on
private helpers or adapter call order.

---

## Common failures

| Symptom | Cause / fix |
| ------- | ----------- |
| `pio test` reports no tests | Use `-e native --without-uploading` and ensure the suite is under `test/`. |
| Native build fails on Arduino types | The module is hardware-facing; move it behind an adapter and ignore it in the native environment. |
| Firmware does not build after `platformio.ini` changes | The file is strict INI; check section headers and indentation. |
| The HTTP request returns `401` | The `X-Control-Token` header is missing or differs from `CONTROL_TOKEN` in `include/config.h`. |
| The HTTP request returns `409` | The JSON `seq` is not greater than the last accepted sequence counter. |
