#include <Arduino.h>

#include "ServoControl.h"
#include "Ultrasonic.h"

static constexpr uint8_t kSensorHeadServoPin = 17;
static constexpr uint8_t kTrigPin = 25;
static constexpr uint8_t kEchoPin = 26;
static constexpr uint32_t kStepDelayMs = 250;

ServoControl headServo(kSensorHeadServoPin);
Ultrasonic ultrasonic(kTrigPin, kEchoPin);

void setup() {
    Serial.begin(115200);
    headServo.begin();
    ultrasonic.begin();
    Serial.println("servo + ultrasonic interface ready");
}

void loop() {
    for (int angle = 0; angle <= 180; angle += 15) {
        headServo.setAngle(angle);
        delay(kStepDelayMs);
        Serial.printf("angle=%d distance=%u mm\n", angle, ultrasonic.measureMm());
    }
    for (int angle = 180; angle >= 0; angle -= 15) {
        headServo.setAngle(angle);
        delay(kStepDelayMs);
        Serial.printf("angle=%d distance=%u mm\n", angle, ultrasonic.measureMm());
    }
}
