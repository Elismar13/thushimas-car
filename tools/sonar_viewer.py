#!/usr/bin/env python3
"""Real-time sonar scan viewer (test harness).

Subscribes to car/scan and renders each scan as a polar (radar) plot and a
Cartesian point plot. This is a throwaway test frontend: the durable artifact is
the scan protocol itself, which is designed to be reused by the future web or
mobile UI.

The car stays stopped until commanded, so this viewer can also drive it:

  S  -> publish START_SCAN to car/cmd
  X  -> publish STOP_SCAN
  P  -> publish PING
  A  -> steer left (decrease STEER angle by --steer-step)
  D  -> steer right (increase STEER angle by --steer-step)
  --auto-start sends START_SCAN as soon as it connects

Progress is printed to the console so the script is never silently idle:
  * MQTT connect result
  * every command sent
  * a one-line summary for every scan rendered
"""

import argparse
import json
import math
import sys

import numpy as np
import paho.mqtt.client as mqtt
import pyqtgraph as pg
from pyqtgraph.Qt import QtCore, QtGui

try:  # paho-mqtt >= 2.0
    from paho.mqtt.client import CallbackAPIVersion

    _API_VERSION = CallbackAPIVersion.VERSION2
except ImportError:  # paho-mqtt < 2.0
    _API_VERSION = None


class _ScanBridge(QtCore.QObject):
    # Delivers scan payloads to the Qt main thread, so the MQTT network thread
    # never touches widgets directly.
    scan_received = QtCore.Signal(str)


class ScanViewer:
    def __init__(self, max_distance_mm=4000, cmd_topic="car/cmd", auto_start=False,
                 steer_step=10):
        self.app = pg.mkQApp()
        self.cmd_topic = cmd_topic
        self.auto_start = auto_start
        self.steer_step = steer_step
        self.steer_angle = 0
        self.win = pg.GraphicsLayoutWidget(show=True)
        self.win.setWindowTitle("Sonar scan - waiting for data...")

        self.polar = self.win.addPlot(row=0, col=0, title="Polar (mm)")
        self.polar.setAspectLocked(True)
        self.polar.hideAxis("left")
        self.polar.hideAxis("bottom")
        self._add_polar_grid(self.polar, max_distance_mm)

        self.cart = self.win.addPlot(row=1, col=0, title="Cartesian (mm, 0 deg = forward)")
        self.cart.setAspectLocked(True)
        self.cart.setLabel("bottom", "x", units="mm")
        self.cart.setLabel("left", "y", units="mm")

        self.polar_scatter = pg.ScatterPlotItem(
            size=8, pen=None, brush=pg.mkBrush(0, 150, 255))
        self.cart_scatter = pg.ScatterPlotItem(
            size=8, pen=None, brush=pg.mkBrush(0, 200, 0))
        self.polar.addItem(self.polar_scatter)
        self.cart.addItem(self.cart_scatter)

        self._bridge = _ScanBridge()
        self._bridge.scan_received.connect(self.on_scan)

        self._add_shortcuts()

    def _add_shortcuts(self):
        QtGui.QShortcut(QtGui.QKeySequence("s"), self.win,
                        activated=lambda: self.send_command("START_SCAN"))
        QtGui.QShortcut(QtGui.QKeySequence("x"), self.win,
                        activated=lambda: self.send_command("STOP_SCAN"))
        QtGui.QShortcut(QtGui.QKeySequence("p"), self.win,
                        activated=lambda: self.send_command("PING"))
        QtGui.QShortcut(QtGui.QKeySequence("a"), self.win,
                        activated=lambda: self.steer_by(-self.steer_step))
        QtGui.QShortcut(QtGui.QKeySequence("d"), self.win,
                        activated=lambda: self.steer_by(self.steer_step))

    def send_command(self, command, **fields):
        payload = {"type": "command", "command": command, **fields}
        self.client.publish(self.cmd_topic, json.dumps(payload), qos=1)
        print("[viewer] sent %s -> %s" % (command, self.cmd_topic))

    def steer_by(self, delta):
        # Clamp to the steering servo travel: [-(140-90), +(140-90)] = [-50, 50].
        self.steer_angle = max(-50, min(50, self.steer_angle + delta))
        self.send_command("STEER", angle=self.steer_angle)
        print("[viewer] steer angle %+dd" % self.steer_angle)

    def _add_polar_grid(self, plot, max_distance_mm):
        grid_pen = pg.mkPen((150, 150, 150, 80))
        for radius in range(500, max_distance_mm + 1, 500):
            theta = np.linspace(0, 2 * np.pi, 361)
            plot.plot(radius * np.cos(theta), radius * np.sin(theta), pen=grid_pen)
        for deg in range(0, 360, 45):
            rad = math.radians(deg)
            plot.plot([0, max_distance_mm * math.cos(rad)],
                      [0, max_distance_mm * math.sin(rad)], pen=grid_pen)

    def on_scan(self, payload):
        try:
            scan = json.loads(payload)
        except json.JSONDecodeError:
            print("[viewer] bad scan payload: %.120s..." % payload)
            return

        points = scan.get("points", [])
        valid = [p for p in points if p.get("distance", 0) > 0]
        if valid:
            angles = np.radians([p["bearing"] for p in valid])
            distances = np.array([p["distance"] for p in valid], dtype=float)
            x = distances * np.cos(angles)
            y = distances * np.sin(angles)
            self.polar_scatter.setData(x, y)
            self.cart_scatter.setData(x, y)
            self.polar.autoRange()
            self.cart.autoRange()
        else:
            self.polar_scatter.setData([], [])
            self.cart_scatter.setData([], [])

        self.win.setWindowTitle(
            "Sonar scan - %d points, proto %s, t=%d"
            % (len(points), scan.get("protocolVersion", "?"), scan.get("timestamp", 0)))
        print("[viewer] rendered scan: %d points, proto %s, t=%d"
              % (len(points), scan.get("protocolVersion", "?"), scan.get("timestamp", 0)))

    def start(self, broker, port, topic):
        self.topic = topic
        self.client = mqtt.Client(callback_api_version=_API_VERSION)
        self.client.on_connect = self._on_connect
        self.client.on_message = self._on_message
        self.client.connect(broker, port)
        self.client.loop_start()
        print("[viewer] listening on %s (broker %s:%d)" % (topic, broker, port))
        print("[viewer] keys: S=start scan, X=stop scan, P=ping, A/D=steer")
        sys.exit(self.app.exec())

    def _on_connect(self, client, userdata, flags, reason_code, properties=None):
        if reason_code == 0:
            client.subscribe(self.topic)
            print("[viewer] connected, subscribed to %s" % self.topic)
            if self.auto_start:
                self.send_command("START_SCAN")
        else:
            print("[viewer] MQTT connection failed: rc=%s" % reason_code)

    def _on_message(self, client, userdata, msg):
        # Runs on the paho network thread: forward to the Qt thread via signal.
        self._bridge.scan_received.emit(msg.payload.decode("utf-8"))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--broker", default="localhost", help="MQTT broker host")
    parser.add_argument("--port", type=int, default=1883, help="MQTT broker port")
    parser.add_argument("--topic", default="car/scan", help="MQTT topic to subscribe to")
    parser.add_argument("--cmd-topic", default="car/cmd", help="MQTT topic for commands")
    parser.add_argument("--auto-start", action="store_true",
                        help="send START_SCAN as soon as connected")
    parser.add_argument("--steer-step", type=int, default=10,
                        help="degrees per A/D steer press (clamped to +/-50)")
    parser.add_argument("--max-distance", type=int, default=4000, help="plot range in mm")
    args = parser.parse_args()

    ScanViewer(max_distance_mm=args.max_distance, cmd_topic=args.cmd_topic,
               auto_start=args.auto_start,
               steer_step=args.steer_step).start(args.broker, args.port, args.topic)


if __name__ == "__main__":
    main()
