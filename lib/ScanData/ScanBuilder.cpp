#include "ScanBuilder.h"

#include <algorithm>

int bearingFor(int headAngleDegrees, uint8_t sensorIndex) {
    int bearing = headAngleDegrees;
    if (sensorIndex == 2) {
        bearing += 180;
    }
    bearing %= 360;
    if (bearing < 0) {
        bearing += 360;
    }
    return bearing;
}

void sortByBearing(ScanData& scan) {
    std::sort(scan.points.begin(), scan.points.end(),
              [](const ScanPoint& a, const ScanPoint& b) { return a.bearing < b.bearing; });
}

uint16_t nearestObstacle(const ScanData& scan, int* bearingOut) {
    uint16_t nearest = 0;
    int foundBearing = 0;
    for (const ScanPoint& point : scan.points) {
        if (point.distanceMm == 0) {
            continue;  // 0 = no echo / out of range
        }
        if (nearest == 0 || point.distanceMm < nearest) {
            nearest = point.distanceMm;
            foundBearing = point.bearing;
        }
    }
    if (bearingOut != nullptr) {
        *bearingOut = foundBearing;
    }
    return nearest;
}

void ScanBuilder::reset() {
    data_ = ScanData();
}

void ScanBuilder::add(int headAngleDegrees, uint8_t sensorIndex, uint16_t distanceMm) {
    ScanPoint point;
    point.bearing = bearingFor(headAngleDegrees, sensorIndex);
    point.distanceMm = distanceMm;
    point.sensor = sensorIndex;
    data_.points.push_back(point);
}

uint32_t ScanBuilder::pointCount() const {
    return static_cast<uint32_t>(data_.points.size());
}

ScanData ScanBuilder::finish(uint32_t timestampMs) {
    sortByBearing(data_);
    data_.timestampMs = timestampMs;
    ScanData finished = data_;
    data_ = ScanData();
    return finished;
}
