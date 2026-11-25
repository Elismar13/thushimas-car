#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    Serial.println("[boot] phone-controlled car firmware");
}

void loop() {
    // The control surface and hardware adapters are added by later tickets.
}
