#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>

class ServoControl {
public:
    static constexpr int kMinAngle = 0;
    static constexpr int kMaxAngle = 180;

    ServoControl(uint8_t pin);

    void begin();
    void setAngle(int angleDegrees);
    int angle() const;

private:
    uint8_t pin_;
    Servo servo_;
    int angle_ = 90;
};
