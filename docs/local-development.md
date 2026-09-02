# Local Development & Testing

Setup guide for running the PC-side tools and talking to the car on your own
machine. This is the setup used for everyday local development/testing.

## 1. MQTT broker on the PC

The broker runs on the PC; the ESP32 and the PC tools connect to it over
Wi-Fi. Mosquitto is the reference broker (`docs/decisions.md`, Decided #12).

### 1.1 Install

```bash
sudo apt install mosquitto mosquitto-clients
```

`mosquitto-clients` provides `mosquitto_sub` / `mosquitto_pub` for quick tests.

### 1.2 Listen on the LAN

By default Mosquitto listens **only on loopback** (`127.0.0.1`), which the ESP32
cannot reach over Wi-Fi. A missing `listener` directive shows up on the car's
serial monitor as:

```
[net] MQTT connect failed (rc=-2), retrying
WiFiClient: connect(): socket error, errno: 104, "Connection reset by peer"
```

To serve the LAN, add a listener that binds all interfaces:

```bash
sudo tee /etc/mosquitto/conf.d/lan.conf <<'EOF'
listener 1883 0.0.0.0
allow_anonymous true
EOF
sudo systemctl restart mosquitto
```

> **Security note:** binding `0.0.0.0` exposes the broker to the whole network
> with no authentication (`allow_anonymous true`). Acceptable for a small indoor
> test environment; revisit before any other network use.

### 1.3 Verify

```bash
ss -tlnp | grep 1883
```

Expected output shows `0.0.0.0:1883` (not just `127.0.0.1:1883`).

Test from the PC against the machine's own LAN IP (from `ip -4 addr show`,
e.g. `10.220.0.18`):

```bash
mosquitto_sub -h 10.220.0.18 -t 'car/scan' -v        # terminal 1
mosquitto_pub -h 10.220.0.18 -t car/scan -m '{}'     # terminal 2 -> you see it arrive
```

## 2. Python tooling

The PC tools live in `tools/` (`sonar_viewer.py`, `mock_scan_publisher.py`).

```bash
cd tools
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

`tools/venv` is gitignored; recreate it on a fresh clone.

On Linux, PyQt6 (used by the viewer) also needs a system package that provides
the Qt `xcb` platform plugin support:

```bash
sudo apt install libxcb-cursor0
```

## 3. Firmware config

Wi-Fi and broker settings live in `include/config.h` (gitignored). Create it
from the template and fill in your values:

```bash
cp include/config.example.h include/config.h
```

Required fields: `WIFI_SSID`, `WIFI_PASSWORD`, `MQTT_BROKER_HOST` (the PC's LAN
IP, e.g. `10.220.0.18`), `MQTT_BROKER_PORT` (default `1883`).

## 4. End-to-end check

> The car stays **stopped at boot until commanded** (safety). Publishing or
> viewing scans never moves the car — you must send `START_SCAN` to `car/cmd`.

### 4.1 Without hardware (mock publisher)

Only exercises the PC → broker → viewer path. The mock publishes fake `car/scan`
messages and **does not** talk to the car.

```bash
# terminal 1: broker already running (section 1)
# terminal 2: fake scan source
cd tools && source venv/bin/activate
python mock_scan_publisher.py --broker 10.220.0.18

# terminal 3: viewer
cd tools && source venv/bin/activate
python sonar_viewer.py --broker 10.220.0.18
```

### 4.2 With the car

The viewer can command the car: press `S` (start scan), `X` (stop scan),
`P` (ping) in the viewer window, or pass `--auto-start` to send `START_SCAN` on
connect. Stop the mock publisher first so you see only real scans.

```bash
~/.platformio/penv/bin/pio run -t upload     # flash firmware
~/.platformio/penv/bin/pio monitor           # serial monitor, expect [net] logs
```

Start scanning — either press `S` in the viewer, or from the terminal:

```bash
mosquitto_pub -h 10.220.0.18 -t car/cmd -m '{"command":"START_SCAN"}'
```

The car publishes one complete 360 degree scan to `car/scan` per half-sweep;
`STOP_SCAN` halts it. Serial log confirms each publish:

```
[net] MQTT connected, subscribed to car/cmd
command received: START_SCAN
scan 1234 published (26 points)
```

If the servo does not sweep, the command never arrived — check the serial log
for `command received: START_SCAN` (it only prints if the ESP32 parsed it).

## 5. Troubleshooting

| Symptom | Cause / fix |
| ------- | ----------- |
| `[net] MQTT connect failed (rc=-2)` + `errno 104` | Broker not listening on the LAN IP. Check `ss -tlnp | grep 1883`; apply section 1.2. |
| No `[net]` logs at all after boot line | `include/config.h` not created or wrong credentials; the log `connecting to Wi-Fi "..."` shows the configured SSID. |
| `[net] Wi-Fi lost, reconnecting...` on a loop | Wrong SSID/password, or AP out of range. |
| Viewer shows nothing | Broker IP wrong for the client, or scans not started (`START_SCAN`). Use `mosquitto_sub -h <broker> -t 'car/scan' -v` to confirm messages flow. |
| Car connects but servo never sweeps | No `START_SCAN` reached the car (it is stopped until commanded). The viewer sends it via `S`/`--auto-start`; `car/scan` traffic never moves the car. Serial shows `command received: START_SCAN` only if the ESP32 parsed it. |
| `[net] scan publish FAILED` right before `scan ... published` | Scan JSON (~1.7 kB) exceeds PubSubClient's default 256-byte buffer. Firmware sets `MQTT_MAX_PACKET_SIZE=2048` in `platformio.ini`; reflash. |
| Viewer crashes with `qt.qpa.plugin ... xcb-cursor0 is needed` | Qt6 needs `libxcb-cursor0` on Linux: `sudo apt install libxcb-cursor0`. |
| `pio test` runs no tests | Use the native env: `~/.platformio/penv/bin/pio test -e native --without-uploading`. |
