# Developer Guide — Data Flows

The current branch has one phone-facing flow: an HTTP control intent enters
the protocol module, the safety core decides what the car may do, and the
state snapshot returns over the same request.

---

## 1. Boot

```mermaid
sequenceDiagram
    participant M as src/main.cpp
    participant W as Wi-Fi
    participant H as HttpApi
    participant P as ControlProtocol
    participant C as CarControl

    M->>C: construct (boot state: disarmed)
    M->>W: WiFi.begin(WIFI_SSID, WIFI_PASSWORD)
    M->>H: begin()
    H->>H: register POST /api/control and GET /api/state
    H->>H: server.begin()
    loop every loop()
        M->>W: reconnect when disconnected
        M->>H: loop()
        H->>H: handleClient()
    end
```

The boot invariant is that the safety core is disarmed and its motor output
is coast. No request is needed to reach a stopped state, and no request can
make the car move before the motor adapter in issue #7 exists.

---

## 2. Accepted control intent

```mermaid
sequenceDiagram
    participant P as Phone
    participant H as HttpApi
    participant E as ControlProtocol
    participant C as CarControl

    P->>H: POST /api/control + X-Control-Token + JSON intent
    H->>E: handleControl(token, payload, context)
    E->>E: authenticate token
    E->>E: parse and validate fields
    E->>E: reject stale/replayed sequence
    E->>C: update(intent, inputs, nowMs)
    C->>C: apply rules and ramp actual throttle
    C-->>E: motor output and state
    E-->>H: accepted response + state snapshot
    H-->>P: 200 application/json
```

The HTTP adapter only moves bytes. Authentication, validation, sequence
handling and safety rules all live behind `ControlEndpoint`.

---

## 3. Rejected control intent

```mermaid
sequenceDiagram
    participant P as Phone
    participant H as HttpApi
    participant E as ControlProtocol
    participant C as CarControl

    P->>H: POST /api/control
    H->>E: handleControl(...)
    alt missing or wrong token
        E-->>H: unauthorized
        H-->>P: 401
    else malformed, missing, wrong type or out of range
        E-->>H: invalid_request
        H-->>P: 400
    else stale or replayed sequence
        E-->>H: stale_sequence
        H-->>P: 409
    end
    Note over E,C: CarControl is not called
```

A rejected request changes neither the car's state nor its last accepted
sequence counter.

---

## 4. Safety-core update

```mermaid
flowchart TD
    A[Control intent + inputs + current time] --> B{Disarmed?}
    B -- yes --> C[Force requested throttle to zero]
    C --> D[Return steering to straight]
    D --> E[Coast]
    B -- no --> F[Apply requested throttle and steering]
    F --> G[Ramp actual throttle toward requested]
    G --> H{Requested throttle zero?}
    H -- yes --> I[Coast]
    H -- no --> J[Drive]
```

This is the scope implemented by issue #6: matrix rows S1 and S2 plus the
throttle ramp. Later safety tickets add their rows inside the same module.

---

## 5. State request

```mermaid
sequenceDiagram
    participant P as Phone
    participant H as HttpApi
    participant E as ControlProtocol

    P->>H: GET /api/state + X-Control-Token
    H->>E: state(token, context)
    E-->>H: current state snapshot
    H-->>P: 200 application/json
```

The state request never calls the safety core and never changes the car.
