#pragma once

#include <stdint.h>

enum class MotorState : uint8_t {
    Coast,
    Drive,
    Brake,
};

enum class BatteryState : uint8_t {
    Ok,
    Low,
    Critical,
};

enum class ArmIntent : uint8_t {
    None,
    Arm,
};

enum class LatchIntent : uint8_t {
    None,
    Set,
    Clear,
};

struct ControlIntent {
    int requestedThrottle = 0;
    int steeringAngle = 0;
    ArmIntent arm = ArmIntent::None;
    LatchIntent latch = LatchIntent::None;
    uint64_t sequence = 0;
};

struct CarInputs {
    uint16_t obstacleDistanceMm = 0;
    bool estopPressed = false;
    bool carLinkFresh = false;
    BatteryState batteryState = BatteryState::Ok;
};

struct MotorOutput {
    MotorState state = MotorState::Coast;
    int actualThrottle = 0;
    int steeringAngle = 0;
};

struct CarState {
    bool armed = false;
    bool latched = false;
    const char* latchReason = "";
    bool estop = false;
    bool interlock = false;
    uint16_t obstacleDistanceMm = 0;
    BatteryState batteryState = BatteryState::Ok;
    int requestedThrottle = 0;
    MotorOutput motor;
};
