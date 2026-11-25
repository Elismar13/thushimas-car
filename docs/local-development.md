# Local Development

## Prerequisites

* PlatformIO (the repository expects `~/.platformio/penv/bin/pio`)
* an ESP32 DevKit V1 for firmware work
* a phone or `curl` for the HTTP interface

There is no MQTT broker, Python environment or PC-side application on this
branch.

---

## Configure the firmware

`include/config.h` is gitignored. Create it from the committed template:

```bash
cp include/config.example.h include/config.h
```

Set at least:

* `WIFI_SSID`
* `WIFI_PASSWORD`
* `CAR_HOSTNAME`
* `CONTROL_TOKEN` — a long random shared secret

Every HTTP request must send that token in the `X-Control-Token` header. If
`CONTROL_TOKEN` is empty, the endpoint rejects every request.

---

## Build and test

```bash
~/.platformio/penv/bin/pio run
~/.platformio/penv/bin/pio test -e native --without-uploading
```

Run one native suite while working on it:

```bash
~/.platformio/penv/bin/pio test -e native --without-uploading \
  --filter test_car_control
```

The firmware build and the complete host suite are both required before a
change is complete.

---

## Flash and monitor

```bash
~/.platformio/penv/bin/pio run -t upload
~/.platformio/penv/bin/pio monitor
```

The serial monitor runs at 115200 baud. The current firmware joins the
configured Wi-Fi network and starts the HTTP server. The access-point fallback
and wireless update wiring are later tickets.

---

## Exercise the HTTP interface

Set the token in the shell without committing it:

```bash
export CONTROL_TOKEN='the-value-from-include/config.h'
```

Read the state:

```bash
curl -H "X-Control-Token: $CONTROL_TOKEN" http://CAR_IP/api/state
```

Post a control intent:

```bash
curl -X POST \
  -H "X-Control-Token: $CONTROL_TOKEN" \
  -H 'Content-Type: application/json' \
  -d '{"t":40,"s":0,"arm":true,"latch":false,"seq":1}' \
  http://CAR_IP/api/control
```

The sequence counter must increase on every accepted request. A stale or
replayed counter returns HTTP 409.

The current firmware does not yet have the motor adapter or the phone page,
so an accepted intent changes the reported state but does not move the car.
Issue #7 adds both.

---

## Safety during development

* Keep the wheels off the ground.
* The obstacle interlock and physical E-stop are not implemented yet.
* Never bypass the E-stop path.
* Do not commit `include/config.h`, credentials or the shared token.
