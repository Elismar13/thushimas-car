#pragma once

#include <stdint.h>

#include "CarTypes.h"

// The safety core's public interface. It owns the car's allowed behaviour and
// performs no I/O: callers provide a control intent, sensor readings and time.
class CarControl {
public:
    CarState update(const ControlIntent& intent, const CarInputs& inputs, uint32_t nowMs);

    const CarState& state() const;

private:
    static constexpr uint32_t kThrottleRampPeriodMs = 300;
    static constexpr uint32_t kThrottleRange = 100;
    static constexpr int kMaxThrottleStep = 17;

    int availableThrottleStep(uint32_t elapsedMs);

    CarState state_;
    uint32_t lastUpdateMs_ = 0;
    int32_t rampRemainder_ = 0;
    bool hasUpdated_ = false;
};
