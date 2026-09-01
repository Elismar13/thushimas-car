#include "Ultrasonic.h"

Ultrasonic::Ultrasonic(uint8_t trigPin, uint8_t echoPin)
    : trigPin_(trigPin), echoPin_(echoPin), sonar_(trigPin, echoPin, kMaxDistanceCm) {}

void Ultrasonic::begin() {
    pinMode(trigPin_, OUTPUT);
    pinMode(echoPin_, INPUT);
    digitalWrite(trigPin_, LOW);
}

uint32_t Ultrasonic::measureMm() {
    return sonar_.ping_median(5) * 10UL;
}
