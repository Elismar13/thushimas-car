#pragma once

#include <Arduino.h>
#include <NewPing.h>

class Ultrasonic {
public:
    static constexpr uint16_t kMaxDistanceCm = 400;

    Ultrasonic(uint8_t trigPin, uint8_t echoPin);

    void begin();
    uint32_t measureCm();
    uint32_t measureMm();

private:
    uint8_t trigPin_;
    uint8_t echoPin_;
    NewPing sonar_;
};
