#include "Ultrasonic.h"

Ultrasonic::Ultrasonic(uint8_t trigPin, uint8_t echoPin)
    : trigPin_(trigPin), echoPin_(echoPin), sonar_(trigPin, echoPin, kMaxDistanceCm) {}

void Ultrasonic::begin() {
    pinMode(trigPin_, OUTPUT);
    pinMode(echoPin_, INPUT);
    digitalWrite(trigPin_, LOW);
}

uint32_t Ultrasonic::measureCm() {
    return sonar_.ping_cm();
}

uint32_t Ultrasonic::measureMm() {
    return static_cast<uint32_t>(sonar_.ping_cm()) * 10;
}
