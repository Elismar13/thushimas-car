#include <Arduino.h>

#include "ServoControl.h"
#include "Ultrasonic.h"

static constexpr uint8_t kHeadServoPin = 17;

static constexpr uint8_t kTrig1Pin = 25;
static constexpr uint8_t kEcho1Pin = 26;
static constexpr uint8_t kTrig2Pin = 32;
static constexpr uint8_t kEcho2Pin = 33;
static constexpr int kStepDegrees = 15;

static constexpr uint32_t kStepDelayMs = 300;

ServoControl headServo(kHeadServoPin);
Ultrasonic ultrasonic1(kTrig1Pin, kEcho1Pin);
Ultrasonic ultrasonic2(kTrig2Pin, kEcho2Pin);

void setup() {
    Serial.begin(115200);
    headServo.begin();
    ultrasonic1.begin();
    ultrasonic2.begin();
    Serial.println("dual-sensor 360 scan interface ready");
}

void loop() {
    for (int angle = 0; angle <= 180; angle += kStepDegrees) {
        headServo.setAngle(angle);
        delay(kStepDelayMs);
        Serial.printf("angle=%d us1=%u us2=%u cm\n", angle, ultrasonic1.measureCm(),
                      ultrasonic2.measureCm());
    }
    for (int angle = 180; angle >= 0; angle -= kStepDegrees) {
        headServo.setAngle(angle);
        delay(kStepDelayMs);
        Serial.printf("angle=%d us1=%u us2=%u cm\n", angle, ultrasonic1.measureCm(),
                      ultrasonic2.measureCm());
    }
}
