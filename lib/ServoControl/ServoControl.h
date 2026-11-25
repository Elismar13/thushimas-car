#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>

// Thin SG90 wrapper. Each servo can be configured with its own angular travel
// (hard limits); setAngle() clamps to it. The phone-controlled car uses one
// steering servo with the tighter [40, 140] travel imposed by its physical
// linkage. Motor/servo units are degrees.
class ServoControl {
public:
    static constexpr int kDefaultMinAngle = 0;
    static constexpr int kDefaultMaxAngle = 180;

    ServoControl(uint8_t pin, int minAngle = kDefaultMinAngle,
                 int maxAngle = kDefaultMaxAngle);

    void begin();
    void setAngle(int angleDegrees);
    int angle() const;
    int minAngle() const;
    int maxAngle() const;

private:
    uint8_t pin_;
    int minAngle_;
    int maxAngle_;
    Servo servo_;
    int angle_ = 90;
};