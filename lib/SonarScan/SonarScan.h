#pragma once

#include <Arduino.h>

#include "ScanBuilder.h"
#include "ServoControl.h"
#include "Ultrasonic.h"

// Drives the rotating dual-sensor head through a continuous wiper sweep
// (0 -> 180 -> 0 -> ...). Each half-sweep is a complete 360 degree scan because
// the two HC-SR04s face opposite directions. Measurement is cooperative: call
// tick() frequently from the main loop and the scan progresses one step at a
// time, leaving the caller free to service MQTT in between.
class SonarScan {
public:
    static constexpr int kMinAngle = 0;
    static constexpr int kMaxAngle = 180;

    SonarScan(uint8_t headPin, uint8_t trig1Pin, uint8_t echo1Pin,
              uint8_t trig2Pin, uint8_t echo2Pin);

    void begin();

    // Start/stop continuous scanning. Stopped at boot; never starts by itself.
    void start();
    void stop();
    bool scanning() const;

    void setStepDegrees(int stepDegrees);   // default 15
    void setSettleMs(uint32_t settleMs);    // default 300

    // Advance the scan by one step. Returns true once per completed half-sweep,
    // filling `out` with the finished 360 degree dataset.
    bool tick(uint32_t nowMs, ScanData& out);

private:
    void moveTo(int angleDegrees);

    ServoControl head_;
    Ultrasonic us1_;
    Ultrasonic us2_;
    ScanBuilder builder_;

    int stepDegrees_;
    uint32_t settleMs_;
    bool scanning_;
    bool forward_;
    int currentAngle_;
    enum class Stage : uint8_t { IDLE, SETTLE, MEASURE };
    Stage stage_;
    uint32_t nextEventMs_;
};
