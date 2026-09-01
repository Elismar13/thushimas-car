#pragma once

#include <stdint.h>
#include <vector>

struct ScanPoint {
    int bearing;          // degrees, 0 = forward, CCW positive, in [0, 360)
    uint16_t distanceMm;  // millimeters; 0 = no echo / out of range
    uint8_t sensor;       // 1 = ultrasonic_1, 2 = ultrasonic_2
};

struct ScanData {
    uint32_t timestampMs;
    std::vector<ScanPoint> points;
};

// Absolute bearing for a sensor given the head angle (degrees).
// Sensor 1 faces the sweep direction (bearing == head angle).
// Sensor 2 faces opposite (bearing == head angle + 180).
// Result is normalized to [0, 360).
int bearingFor(int headAngleDegrees, uint8_t sensorIndex);

// Sort a scan's points by bearing (ascending, [0, 360)).
void sortByBearing(ScanData& scan);
