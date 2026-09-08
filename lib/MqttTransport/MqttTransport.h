#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include <functional>

// Thin MQTT transport: connects the ESP32 to the PC broker and exposes
// non-blocking reconnect + publish helpers. Safety invariant: this class never
// starts the car; the firmware stays stopped until commanded over MQTT.
class MqttTransport {
public:
    using CommandCallback =
        std::function<void(const char* topic, const uint8_t* payload, unsigned int length)>;

    MqttTransport(const char* ssid, const char* password, const char* brokerHost,
                  uint16_t brokerPort, const char* clientId, const char* cmdTopic,
                  const char* scanTopic, const char* statusTopic);

    void begin();

    // Non-blocking maintenance: reconnect Wi-Fi/broker as needed, service
    // PubSubClient. Call frequently from the main loop.
    void loop();

    bool connected();

    bool wifiConnected();
    bool mqttConnected();

    bool publishScan(const char* payload);    // -> car/scan
    bool publishStatus(const char* payload);  // -> car/status

    void setCommandCallback(CommandCallback callback);

private:
    static constexpr uint32_t kReconnectIntervalMs = 2000;

    void connectBroker();

    const char* ssid_;
    const char* password_;
    const char* brokerHost_;
    uint16_t brokerPort_;
    const char* clientId_;
    const char* cmdTopic_;
    const char* scanTopic_;
    const char* statusTopic_;

    WiFiClient wifiClient_;
    PubSubClient mqtt_;
    CommandCallback commandCb_;
    uint32_t lastReconnectMs_;
    bool wifiConnected_;
    bool mqttConnected_;
};
