#include "ServoControl.h"

ServoControl::ServoControl(uint8_t pin, int minAngle, int maxAngle)
    : pin_(pin),
      minAngle_(constrain(minAngle, 0, 180)),
      maxAngle_(constrain(maxAngle, 0, 180)) {}

void ServoControl::begin() {
    servo_.attach(pin_);
    setAngle(90);
}

void ServoControl::setAngle(int angleDegrees) {
    angle_ = constrain(angleDegrees, minAngle_, maxAngle_);
    servo_.write(angle_);
}

int ServoControl::angle() const {
    return angle_;
}

int ServoControl::minAngle() const {
    return minAngle_;
}

int ServoControl::maxAngle() const {
    return maxAngle_;
}
