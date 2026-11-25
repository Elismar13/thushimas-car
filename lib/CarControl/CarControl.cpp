#include "CarControl.h"

CarState CarControl::update(const ControlIntent& intent, const CarInputs& inputs,
                             uint32_t nowMs) {
    uint32_t elapsedMs = 0;
    if (hasUpdated_) {
        elapsedMs = nowMs - lastUpdateMs_;
    }
    lastUpdateMs_ = nowMs;
    hasUpdated_ = true;

    if (intent.arm == ArmIntent::Arm) {
        state_.armed = true;
    }

    if (state_.armed) {
        state_.requestedThrottle = intent.requestedThrottle;
        state_.motor.steeringAngle = intent.steeringAngle;
        state_.motor.state = intent.requestedThrottle == 0 ? MotorState::Coast
                                                           : MotorState::Drive;
    } else {
        state_.requestedThrottle = 0;
        state_.motor.steeringAngle = 0;
        state_.motor.state = MotorState::Coast;
    }

    const int step = availableThrottleStep(elapsedMs);
    int actual = state_.motor.actualThrottle;
    const int requested = state_.requestedThrottle;

    if (actual > 0 && requested < 0) {
        actual = actual > step ? actual - step : 0;
    } else if (actual < 0 && requested > 0) {
        actual = actual < -step ? actual + step : 0;
    } else {
        const int difference = requested - actual;
        if (difference > step) {
            actual += step;
        } else if (difference < -step) {
            actual -= step;
        } else {
            actual = requested;
        }
    }
    state_.motor.actualThrottle = actual;

    state_.obstacleDistanceMm = inputs.obstacleDistanceMm;
    state_.batteryState = inputs.batteryState;
    return state_;
}

int CarControl::availableThrottleStep(uint32_t elapsedMs) {
    // One throttle point represents three milliseconds of ramp time. Carrying
    // the fractional remainder keeps the average rate independent of cadence.
    const int64_t scaledElapsed =
        static_cast<int64_t>(elapsedMs) * kThrottleRange + rampRemainder_;
    int64_t step = (scaledElapsed + kThrottleRampPeriodMs / 2) /
                   kThrottleRampPeriodMs;

    if (step > kMaxThrottleStep) {
        // A delayed update moves at most one normal 50 ms slice; it never
        // catches up in one jump.
        rampRemainder_ = 0;
        return kMaxThrottleStep;
    }

    rampRemainder_ = static_cast<int32_t>(
        scaledElapsed - step * kThrottleRampPeriodMs);
    return static_cast<int>(step);
}

const CarState& CarControl::state() const {
    return state_;
}
