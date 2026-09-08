# Developer Guide — Data Flows

End-to-end flows through the system. All flows start from `loop()` in
`src/main.cpp`; MQTT endpoints are defined in `docs/communication.md`.

---

## 1. Boot and connect

```mermaid
sequenceDiagram
    participant P as src/main.cpp
    participant S as SonarScan
    participant M as MqttTransport
    participant W as WiFi (ESP32)
    participant B as Broker (PC)

    P->>S: setup(): begin() + setStepDegrees(15) + setSettleMs(300)
    P->>M: begin() (set SSID/pwd, broker, cmdTopic, callback)
    M->>W: WiFi.begin(ssid, password)  [STA mode]
    Note over M,W: loop() drives reconnects at 2 s intervals
    loop every loop()
        M->>W: WiFi.status()?
        W-->>M: WL_CONNECTED
        M->>B: mqtt.connect(clientId)
        M->>B: subscribe(car/cmd)
        M-->>P: connected() == true
    end
    Note over P,S: Car is STILL STOPPED: scanner not started, no scanning
```

Boot invariant: after setup, the car neither scans nor moves until a command
arrives on `car/cmd`.

---

## 2. START_SCAN command → servo sweep → scan publish

The two opposed HC-SR04s make every 0→180 half-sweep a complete 360° scan that
is published as one `car/scan` message.

```mermaid
sequenceDiagram
    participant V as sonar_viewer.py (or mosquitto_pub)
    participant B as Broker
    participant M as MqttTransport
    participant C as main.cpp handleCommand()
    participant S as SonarScan
    participant H as head servo + 2x HC-SR04
    participant P as main.cpp publishScan()

    V->>B: PUB car/cmd {"type":"command","command":"START_SCAN"} (QoS 1)
    B->>M: SUB car/cmd callback
    M->>C: handleCommand(topic, payload, len)
    C->>S: scanner.start()
    Note over S: begin wiper: moveTo(0), SETTLE
    C-->>P: publishStatus() -> car/status

    loop every settleMs (300) per angle
        S->>H: servo.setAngle(currentAngle)
        S->>H: us1.measureMm() then us2.measureMm() (sequential)
        alt reach end of half-sweep (0→180 or 180→0)
            S-->>C: tick() returns true, fill ScanData (26 pts)
            C->>P: publishScan(scan)
            P->>M: publishScan(json) -> car/scan (QoS 0)
            B->>V: deliver scan (viewer renders polar + cartesian)
        else
            S->>H: moveTo(next angle)
        end
    end
```

Timing note: step 15° × settle 300 ms → one half-sweep ≈ 3.6 s → a scan
roughly every 3.6 s.

---

## 3. STOP_SCAN

```mermaid
sequenceDiagram
    participant V as sonar_viewer.py
    participant B as Broker
    participant M as MqttTransport
    participant C as main.cpp handleCommand()
    participant S as SonarScan

    V->>B: PUB car/cmd {"command":"STOP_SCAN"} (QoS 1)
    B->>M: callback
    M->>C: handleCommand(...)
    C->>S: scanner.stop()
    Note over S: stage = IDLE; servo stays where it was
    C-->>B: publishStatus() -> car/status (scanning: false)
```

---

## 4. Link loss (safety path)

The car must never scan or publish without a live link.

```mermaid
sequenceDiagram
    participant M as MqttTransport
    participant W as WiFi
    participant L as main.cpp loop()
    participant S as SonarScan

    Note over M,W: MQTT or Wi-Fi drops
    M-->>L: connected() == false
    alt scanner was scanning
        L->>S: scanner.stop()  // remain idle while offline
    else
        Note over S: already idle, nothing to do
    end
    loop reconnect (every 2 s)
        M->>W: Wi-Fi reconnect
        M->>M: mqtt reconnect + subscribe(car/cmd)
    end
    Note over S: Re-armed only after a new START_SCAN command
```

---

## 5. Display path (every loop pass)

```mermaid
sequenceDiagram
    participant L as main.cpp loop()
    participant M as MqttTransport
    participant S as SonarScan
    participant D as StatusDisplay
    participant X as I2CLcdDisplay (LCD)

    L->>M: wifiConnected() / connected()
    L->>S: scanning()
    L->>L: scanCount, lastPointCount, lastNearest*
    Note over L: build StatusDisplay::State
    L->>D: display.tick(millis(), state)
    alt rotation due (every 3000 ms) or first run
        D->>X: printLine(0, line1) / printLine(1, line2)
    else
        Note over X: nothing written (page unchanged)
    end
```

Display is passive: it only reads state, never publishes or moves anything.

---

## 6. PC tool data roles

| Tool | Publishes | Subscribes | Drives the car? |
| ---- | --------- | ---------- | --------------- |
| `sonar_viewer.py` | `car/cmd` (S/X/P keys, `--auto-start`) | `car/scan` | Yes — commands only, car still enforces safety |
| `mock_scan_publisher.py` | `car/scan` (fake) | — | **No** — for GUI/schema tests only |

When testing with the real car, run the viewer (possibly `--auto-start`) and
stop the mock publisher so only real scans flow.