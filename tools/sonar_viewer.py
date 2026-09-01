#!/usr/bin/env python3
"""Real-time sonar scan viewer (test harness).

Subscribes to car/scan and renders each scan as a polar (radar) plot and a
Cartesian point plot. This is a throwaway test frontend: the durable artifact is
the scan protocol itself, which is designed to be reused by the future web or
mobile UI.
"""

import argparse
import json
import math
import sys

import numpy as np
import paho.mqtt.client as mqtt
import pyqtgraph as pg

try:  # paho-mqtt >= 2.0
    from paho.mqtt.client import CallbackAPIVersion

    _API_VERSION = CallbackAPIVersion.VERSION2
except ImportError:  # paho-mqtt < 2.0
    _API_VERSION = None


class ScanViewer:
    def __init__(self, max_distance_mm=4000):
        self.app = pg.mkQApp()
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
        else:
            self.polar_scatter.setData([], [])
            self.cart_scatter.setData([], [])

        self.win.setWindowTitle(
            "Sonar scan - %d points, proto %s, t=%d"
            % (len(points), scan.get("protocolVersion", "?"), scan.get("timestamp", 0)))

    def start(self, broker, port, topic):
        self.topic = topic
        self.client = mqtt.Client(callback_api_version=_API_VERSION)
        self.client.on_connect = self._on_connect
        self.client.on_message = self._on_message
        self.client.connect(broker, port)
        self.client.loop_start()
        print("listening on %s" % topic)
        sys.exit(self.app.exec())

    def _on_connect(self, client, userdata, flags, reason_code, properties=None):
        if reason_code == 0:
            client.subscribe(self.topic)
        else:
            print("MQTT connection failed: rc=%s" % reason_code)

    def _on_message(self, client, userdata, msg):
        self.on_scan(msg.payload.decode("utf-8"))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--broker", default="localhost", help="MQTT broker host")
    parser.add_argument("--port", type=int, default=1883, help="MQTT broker port")
    parser.add_argument("--topic", default="car/scan", help="MQTT topic to subscribe to")
    parser.add_argument("--max-distance", type=int, default=4000, help="plot range in mm")
    args = parser.parse_args()

    ScanViewer(max_distance_mm=args.max_distance).start(args.broker, args.port, args.topic)


if __name__ == "__main__":
    main()
