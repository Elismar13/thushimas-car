#include "SonarScan.h"

SonarScan::SonarScan(uint8_t headPin, uint8_t trig1Pin, uint8_t echo1Pin,
                     uint8_t trig2Pin, uint8_t echo2Pin)
    : head_(headPin),
      us1_(trig1Pin, echo1Pin),
      us2_(trig2Pin, echo2Pin),
      stepDegrees_(15),
      settleMs_(300),
      scanning_(false),
      forward_(true),
      currentAngle_(0),
      stage_(Stage::IDLE),
      nextEventMs_(0) {}

void SonarScan::begin() {
    head_.begin();
    us1_.begin();
    us2_.begin();
}

void SonarScan::start() {
    if (scanning_) {
        return;
    }
    scanning_ = true;
    forward_ = true;
    builder_.reset();
    moveTo(0);
    stage_ = Stage::SETTLE;
    nextEventMs_ = millis() + settleMs_;
}

void SonarScan::stop() {
    scanning_ = false;
    stage_ = Stage::IDLE;
}

bool SonarScan::scanning() const {
    return scanning_;
}

void SonarScan::setStepDegrees(int stepDegrees) {
    stepDegrees_ = constrain(stepDegrees, 1, kMaxAngle - kMinAngle);
}

void SonarScan::setSettleMs(uint32_t settleMs) {
    settleMs_ = settleMs;
}

void SonarScan::moveTo(int angleDegrees) {
    currentAngle_ = constrain(angleDegrees, kMinAngle, kMaxAngle);
    head_.setAngle(currentAngle_);
}

bool SonarScan::tick(uint32_t nowMs, ScanData& out) {
    if (!scanning_ || stage_ == Stage::IDLE) {
        return false;
    }
    if (nowMs < nextEventMs_) {
        return false;
    }

    if (stage_ == Stage::SETTLE) {
        // Head has settled: fire both sensors sequentially (never simultaneously).
        builder_.add(currentAngle_, 1, us1_.measureMm());
        builder_.add(currentAngle_, 2, us2_.measureMm());

        int target = forward_ ? kMaxAngle : kMinAngle;
        if (currentAngle_ == target) {
            // Half-sweep complete: this is a full 360 degree scan. Reverse
            // direction; the turnaround point belongs to both half-sweeps.
            forward_ = !forward_;
            stage_ = Stage::SETTLE;
            nextEventMs_ = nowMs + settleMs_;
            out = builder_.finish(nowMs);
            return true;
        }

        int next = forward_ ? currentAngle_ + stepDegrees_ : currentAngle_ - stepDegrees_;
        moveTo(next);
        stage_ = Stage::SETTLE;
        nextEventMs_ = nowMs + settleMs_;
        return false;
    }

    return false;
}
