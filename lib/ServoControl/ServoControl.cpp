#include "ServoControl.h"

ServoControl::ServoControl(uint8_t pin) : pin_(pin) {}

void ServoControl::begin() {
    servo_.attach(pin_);
    setAngle(90);
}

void ServoControl::setAngle(int angleDegrees) {
    angle_ = constrain(angleDegrees, kMinAngle, kMaxAngle);
    servo_.write(angle_);
}

int ServoControl::angle() const {
    return angle_;
}
