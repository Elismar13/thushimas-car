# Communication Protocol

## 1. Purpose

The phone and the car communicate over one small HTTP interface. The phone
posts a **control intent**; the car answers with a **state snapshot**. The
protocol is deliberately transport-neutral: HTTP is the current adapter, but
the safety rules do not depend on it.

Units are fixed:

* requested and actual throttle: `-100` … `+100`
* steering angle: degrees relative to **straight**, `-50` … `+50`
* distance: millimetres

The full safety behaviour is specified in
[`docs/safety-matrix.md`](safety-matrix.md).

---

## 2. Transport and authentication

The ESP32 serves HTTP on port 80. The current firmware joins the configured
Wi-Fi network in station mode; access-point fallback and the phone page are
later control-surface work.

Every request carries the shared token in the `X-Control-Token` header:

```http
X-Control-Token: <shared secret>
```

The token lives in the gitignored `include/config.h` as `CONTROL_TOKEN`.
`include/config.example.h` is the committed template. An empty configured
token is not a valid token: the endpoint rejects every request.

---

## 3. Endpoints

| Method | Path | Purpose |
| ------ | ---- | ------- |
| `POST` | `/api/control` | Accept a control intent and return the resulting state snapshot. |
| `GET`  | `/api/state`  | Return the current state snapshot without changing the car. |

The phone page at `GET /` is added by issue #7. It will use the same token and
the same control endpoint.

### HTTP status codes

| Status | Meaning |
| ------ | ------- |
| `200` | The request was accepted (or the state snapshot was returned). |
| `400` | The body is malformed, a required field is missing, a field has the wrong type, or a value is out of range. |
| `401` | The shared token is missing or wrong. |
| `409` | The sequence counter is stale or replayed. |

A rejected request never changes the car's state.

---

## 4. Control intent

`POST /api/control` takes JSON. All five fields are required.

```json
{
  "t": 40,
  "s": -15,
  "arm": true,
  "latch": false,
  "seq": 123
}
```

| Field | Type | Range | Meaning |
| ----- | ---- | ----- | ------- |
| `t` | integer | `-100` … `+100` | Requested throttle. |
| `s` | integer | `-50` … `+50` | Steering angle relative to straight. |
| `arm` | boolean | — | `true` requests arming. `false` does not disarm the car. |
| `latch` | boolean | — | Latched-stop intent. Its stopping behaviour is implemented by issue #12. |
| `seq` | unsigned integer | — | Strictly increasing request counter. |

### Sequence handling

The car remembers the last accepted sequence counter. An intent is dropped
when `seq` is less than or equal to that counter. This rejects both stale
requests and replays. A rejected request does not advance the counter.

The phone should persist or obtain a counter greater than the last accepted
one when it reconnects. The state endpoint can be used to recover the car's
current state before posting again.

---

## 5. State snapshot

A successful control response and the state endpoint return the same schema:

```json
{
  "protocolVersion": "1.0",
  "armed": true,
  "latched": false,
  "latchReason": "",
  "estop": false,
  "interlock": false,
  "distanceMm": 0,
  "batteryState": "ok",
  "batteryMv": 0,
  "batteryPct": 0,
  "actualThrottle": 0,
  "requestedThrottle": 40,
  "steering": -15,
  "rssi": -55,
  "ip": "192.168.1.20",
  "uptimeMs": 1234
}
```

| Field | Meaning |
| ----- | ------- |
| `armed` | Whether the car accepts motion. |
| `latched` | Whether a latched stop is engaged. |
| `latchReason` | Why the latch is engaged, or an empty string. |
| `estop` | Whether the physical E-stop is pressed. |
| `interlock` | Whether the obstacle interlock is refusing throttle. |
| `distanceMm` | Distance to the nearest obstacle; `0` means no echo. |
| `batteryState` | `ok`, `low` or `critical`. |
| `batteryMv` / `batteryPct` | Measured pack state; populated by issue #15. |
| `actualThrottle` | Throttle the motor is running at now. |
| `requestedThrottle` | Throttle the phone asked for. |
| `steering` | Steering angle relative to straight. |
| `rssi` / `ip` / `uptimeMs` | Car-link and identity information. |

The fields for the latched stop, E-stop, interlock, battery and stale-link rules
are present from the first protocol version so the page can use one stable
schema. Their behaviour is implemented by the tickets listed in
[`docs/roadmap.md`](roadmap.md); until then they report their inactive state.

### Error body

Errors use a small JSON object:

```json
{"error":"unauthorized"}
```

Possible values are `unauthorized`, `invalid_request` and `stale_sequence`.

---

## 6. Current safety-core behaviour

The safety core (`lib/CarControl`) owns the rules and performs no I/O. Issue #6
implements matrix rows S1 and S2 and the throttle ramp:

* The car boots **disarmed**, stopped and coasting.
* A disarmed car forces requested throttle to zero and returns steering to
  straight.
* An arm request arms the car. While armed, the requested throttle applies and
  the steering angle is applied.
* The actual throttle ramps toward the requested throttle at a bounded rate:
  100% in 300 ms. The ramp works in both directions and reaches exactly zero
  before changing direction.

The stale-link stop, latched stop, physical E-stop, obstacle interlock and
battery cut are deliberately separate tickets. The car must not be driven on
the floor until the interlock (#11) and physical E-stop (#13) are in place.

---

## 7. Versioning

Breaking protocol changes bump `protocolVersion`. The current version is
`1.0`.
