#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "ControlProtocol.h"
#include "HttpApi.h"

namespace {
constexpr uint32_t kWifiRetryIntervalMs = 2000;
uint32_t lastWifiRetryMs = 0;
}  // namespace

ControlProtocol control(CONTROL_TOKEN);
HttpApi http(control);

void setup() {
    Serial.begin(115200);
    Serial.println("[boot] phone-controlled car firmware");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.printf("[net] connecting to Wi-Fi \"%s\"...\n", WIFI_SSID);

    http.begin();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        const uint32_t now = millis();
        if (now - lastWifiRetryMs >= kWifiRetryIntervalMs) {
            lastWifiRetryMs = now;
            WiFi.reconnect();
        }
    }

    http.loop();
}
