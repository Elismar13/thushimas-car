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
      lastReconnectMs_(0),
      wifiConnected_(false),
      mqttConnected_(false) {}

void MqttTransport::begin() {
    mqtt_.setServer(brokerHost_, brokerPort_);
    mqtt_.setCallback([this](char* topic, uint8_t* payload, unsigned int length) {
        if (commandCb_) {
            commandCb_(topic, payload, length);
        }
    });

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid_, password_);
    Serial.printf("[net] connecting to Wi-Fi \"%s\"...\n", ssid_);
}

void MqttTransport::loop() {
    bool wifiUp = WiFi.status() == WL_CONNECTED;
    if (wifiUp != wifiConnected_) {
        wifiConnected_ = wifiUp;
        if (wifiUp) {
            Serial.printf("[net] Wi-Fi connected (IP %s)\n",
                          WiFi.localIP().toString().c_str());
        } else {
            Serial.println("[net] Wi-Fi lost, reconnecting...");
        }
    }

    if (!wifiUp) {
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

    if (!mqttConnected_) {
        Serial.printf("[net] connecting to broker %s:%u...\n", brokerHost_, brokerPort_);
    }
    if (mqtt_.connect(clientId_)) {
        mqtt_.subscribe(cmdTopic_);
        mqttConnected_ = true;
        Serial.printf("[net] MQTT connected, subscribed to %s\n", cmdTopic_);
    } else {
        if (mqttConnected_) {
            mqttConnected_ = false;
        }
        Serial.printf("[net] MQTT connect failed (rc=%d), retrying\n", mqtt_.state());
    }
}

bool MqttTransport::publishScan(const char* payload) {
    if (!mqtt_.connected()) {
        Serial.println("[net] scan publish skipped: not connected");
        return false;
    }
    if (mqtt_.publish(scanTopic_, payload)) {
        return true;
    }
    Serial.printf("[net] scan publish FAILED (%u bytes; check MQTT_MAX_PACKET_SIZE)\n",
                  static_cast<unsigned>(strlen(payload)));
    return false;
}

bool MqttTransport::publishStatus(const char* payload) {
    if (!mqtt_.connected()) {
        Serial.println("[net] status publish skipped: not connected");
        return false;
    }
    if (mqtt_.publish(statusTopic_, payload)) {
        return true;
    }
    Serial.println("[net] status publish FAILED");
    return false;
}

void MqttTransport::setCommandCallback(CommandCallback callback) {
    commandCb_ = callback;
}
