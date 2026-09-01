#pragma once

#include "ScanTypes.h"

// Accumulates scan points as a head sweep progresses and finishes a complete
// scan dataset. Pure logic: no hardware dependencies, host-testable.
class ScanBuilder {
public:
    void reset();

    // Record one reading for a sensor at a given head angle.
    void add(int headAngleDegrees, uint8_t sensorIndex, uint16_t distanceMm);

    uint32_t pointCount() const;

    // Sort the collected points by bearing, stamp the timestamp and return the
    // finished dataset. Clears the builder for the next sweep.
    ScanData finish(uint32_t timestampMs);

private:
    ScanData data_;
};
