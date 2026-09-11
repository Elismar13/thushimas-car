#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>

// Thin SG90 wrapper. Each servo can be configured with its own angular travel
// (hard limits); setAngle() clamps to it. The sensor-head servo uses the
// default [0, 180]; the steering servo uses a tighter [40, 140] because of its
// physical steering linkage. Motor/servo units are degrees.
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