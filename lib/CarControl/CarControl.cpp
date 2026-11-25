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

    const int step = maxThrottleStep(elapsedMs);
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

int CarControl::maxThrottleStep(uint32_t elapsedMs) {
    if (elapsedMs >= 300) {
        return 100;
    }
    return static_cast<int>((100UL * elapsedMs + 299UL) / 300UL);
}

const CarState& CarControl::state() const {
    return state_;
}
