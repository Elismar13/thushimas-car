#include "MqttTransport.h"

MqttTransport::MqttTransport(const char* ssid, const char* password,
                             const char* brokerHost, uint16_t brokerPort,
                             const char* clientId, const char* cmdTopic,
                             const char* scanTopic, const char* statusTopic)
    : ssid_(ssid),
      password_(password),
      brokerHost_(brokerHost),
      brokerPort_(brokerPort),
      clientId_(clientId),
      cmdTopic_(cmdTopic),
      scanTopic_(scanTopic),
      statusTopic_(statusTopic),
      mqtt_(wifiClient_),
      lastReconnectMs_(0) {}

void MqttTransport::begin() {
    mqtt_.setServer(brokerHost_, brokerPort_);
    mqtt_.setCallback([this](char* topic, uint8_t* payload, unsigned int length) {
        if (commandCb_) {
            commandCb_(topic, payload, length);
        }
    });

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid_, password_);
}

void MqttTransport::loop() {
    if (WiFi.status() != WL_CONNECTED) {
        // Reconnect Wi-Fi at most every kReconnectIntervalMs; stay offline (and
        // thus stopped) in between.
        uint32_t now = millis();
        if (now - lastReconnectMs_ >= kReconnectIntervalMs) {
            lastReconnectMs_ = now;
            WiFi.reconnect();
        }
        return;
    }

    if (!mqtt_.connected()) {
        connectBroker();
        return;
    }

    mqtt_.loop();
}

bool MqttTransport::connected() {
    return WiFi.status() == WL_CONNECTED && mqtt_.connected();
}

void MqttTransport::connectBroker() {
    uint32_t now = millis();
    if (now - lastReconnectMs_ < kReconnectIntervalMs) {
        return;
    }
    lastReconnectMs_ = now;
    if (mqtt_.connect(clientId_)) {
        mqtt_.subscribe(cmdTopic_);
    }
}

bool MqttTransport::publishScan(const char* payload) {
    return mqtt_.connected() && mqtt_.publish(scanTopic_, payload);
}

bool MqttTransport::publishStatus(const char* payload) {
    return mqtt_.connected() && mqtt_.publish(statusTopic_, payload);
}

void MqttTransport::setCommandCallback(CommandCallback callback) {
    commandCb_ = callback;
}
