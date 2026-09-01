#!/usr/bin/env python3
"""Publish fake car/scan messages so the viewer can be tested without hardware.

Emits the exact same JSON schema as the ESP32 firmware (see
docs/communication.md): a full 360 degree dataset from a simulated 0->180 head
sweep of two opposed HC-SR04s. The environment is a 3000 x 3000 mm room with the
car near the center.
"""

import argparse
import json
import math
import random
import time

import paho.mqtt.client as mqtt

ROOM_HALF = 1500.0  # mm from center to each wall
MAX_SENSOR_MM = 4000
DROPOUT_PROB = 0.05  # probability a reading is "no echo" (distance 0)


def bearing_for(head_angle, sensor_index):
    bearing = head_angle if sensor_index == 1 else head_angle + 180
    return bearing % 360


def distance_to_wall(bearing_deg):
    """Distance (mm) from the room center along a bearing to the nearest wall."""
    rad = math.radians(bearing_deg)
    cos = math.cos(rad)
    sin = math.sin(rad)
    candidates = []
    if abs(cos) > 1e-6:
        candidates.append(ROOM_HALF / abs(cos))
    if abs(sin) > 1e-6:
        candidates.append(ROOM_HALF / abs(sin))
    return min(candidates)


def build_scan(timestamp_ms):
    points = []
    for angle in range(0, 181, 15):
        for sensor_index in (1, 2):
            bearing = bearing_for(angle, sensor_index)
            if random.random() < DROPOUT_PROB:
                distance = 0
            else:
                distance = int(round(distance_to_wall(bearing) + random.uniform(-30, 30)))
            points.append({
                "bearing": bearing,
                "distance": min(distance, MAX_SENSOR_MM),
                "sensor": "ultrasonic_%d" % sensor_index,
            })
    points.sort(key=lambda p: p["bearing"])
    return {
        "type": "scan",
        "protocolVersion": "1.0",
        "sensor": "ultrasonic",
        "timestamp": timestamp_ms,
        "points": points,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--broker", default="localhost", help="MQTT broker host")
    parser.add_argument("--port", type=int, default=1883, help="MQTT broker port")
    parser.add_argument("--topic", default="car/scan", help="MQTT topic to publish on")
    parser.add_argument("--interval", type=float, default=4.0, help="seconds between scans")
    args = parser.parse_args()

    client = mqtt.Client()
    client.connect(args.broker, args.port)
    client.loop_start()
    print("publishing fake scans to %s:%d%s every %.1fs" %
          (args.broker, args.port, args.topic, args.interval))
    try:
        while True:
            payload = json.dumps(build_scan(int(time.time() * 1000)))
            client.publish(args.topic, payload)
            time.sleep(args.interval)
    except KeyboardInterrupt:
        pass
    finally:
        client.loop_stop()


if __name__ == "__main__":
    main()
