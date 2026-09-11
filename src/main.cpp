#include <Arduino.h>
#include <ArduinoJson.h>

#include "config.h"
#include "I2CLcdDisplay.h"
#include "MqttTransport.h"
#include "SonarScan.h"
#include "StatusDisplay.h"

static constexpr uint8_t kHeadServoPin = 17;
static constexpr uint8_t kSteerServoPin = 16;

static constexpr uint8_t kTrig1Pin = 25;
static constexpr uint8_t kEcho1Pin = 26;
static constexpr uint8_t kTrig2Pin = 32;
static constexpr uint8_t kEcho2Pin = 33;

static constexpr int kScanStepDegrees = 15;
static constexpr uint32_t kSettleMs = 300;

// Steering servo travel, limited by the physical steering linkage. 90° = wheels
// straight. STEER angle maps 1:1: STEER 0 -> 90°, STEER +50 -> 140°, -50 -> 40°.
static constexpr int kSteerMinAngle = 40;
static constexpr int kSteerMaxAngle = 140;

// On-car status display (16x2 HD44780 via PCF8574 I2C backpack).
static constexpr uint8_t kLcdAddress = 0x27;
static constexpr int kLcdCols = 16;
static constexpr int kLcdRows = 2;

static constexpr const char* kCmdTopic = "car/cmd";
static constexpr const char* kScanTopic = "car/scan";
static constexpr const char* kStatusTopic = "car/status";

SonarScan scanner(kHeadServoPin, kTrig1Pin, kEcho1Pin, kTrig2Pin, kEcho2Pin);
ServoControl steerServo(kSteerServoPin, kSteerMinAngle, kSteerMaxAngle);
MqttTransport mqtt(WIFI_SSID, WIFI_PASSWORD, MQTT_BROKER_HOST, MQTT_BROKER_PORT,
                   MQTT_CLIENT_ID, kCmdTopic, kScanTopic, kStatusTopic);
I2CLcdDisplay lcd(kLcdAddress, kLcdCols, kLcdRows);
StatusDisplay display(lcd);

static uint32_t scanCount = 0;
static int lastPointCount = 0;
static uint16_t lastNearestMm = 0;
static int lastNearestBearing = 0;

void publishScan(const ScanData& scan) {
    DynamicJsonDocument doc(2048);
    doc["type"] = "scan";
    doc["protocolVersion"] = "1.0";
    doc["sensor"] = "ultrasonic";
    doc["timestamp"] = scan.timestampMs;

    JsonArray points = doc.createNestedArray("points");
    for (const ScanPoint& point : scan.points) {
        JsonObject obj = points.createNestedObject();
        obj["bearing"] = point.bearing;
        obj["distance"] = point.distanceMm;
        obj["sensor"] = (point.sensor == 1) ? "ultrasonic_1" : "ultrasonic_2";
    }

    char buffer[2048];
    size_t length = serializeJson(doc, buffer, sizeof(buffer));
    if (length == 0) {
        return;
    }
    if (mqtt.publishScan(buffer)) {
        scanCount++;
        lastPointCount = static_cast<int>(scan.points.size());
        lastNearestMm = nearestObstacle(scan, &lastNearestBearing);
        Serial.printf("scan %lu published (%u points)\n",
                      static_cast<unsigned long>(scan.timestampMs),
                      static_cast<unsigned>(scan.points.size()));
    }
}

void publishStatus() {
    StaticJsonDocument<192> doc;
    doc["type"] = "status";
    doc["protocolVersion"] = "1.0";
    doc["timestamp"] = millis();
    doc["scanning"] = scanner.scanning();
    doc["scanCount"] = scanCount;
    // Steering relative to straight: 0 = centered, +50 = 140° hard stop, etc.
    doc["steeringAngle"] = steerServo.angle() - 90;

    char buffer[192];
    serializeJson(doc, buffer, sizeof(buffer));
    mqtt.publishStatus(buffer);
}

void handleCommand(const char* topic, const uint8_t* payload, unsigned int length) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
        Serial.printf("command parse error: %s\n", error.c_str());
        return;
    }

    const char* command = doc["command"] | "";
    Serial.printf("command received: %s\n", command);

    if (strcmp(command, "START_SCAN") == 0) {
        scanner.start();
        publishStatus();
    } else if (strcmp(command, "STOP_SCAN") == 0) {
        scanner.stop();
        publishStatus();
    } else if (strcmp(command, "STEER") == 0) {
        // STEER angle is relative to straight (0 = centered); clamp to
        // [-(kSteerMaxAngle-90), +(kSteerMaxAngle-90)] = [-50, +50].
        int angle = doc["angle"] | 0;
        steerServo.setAngle(90 + angle);
        Serial.printf("steer set to %d deg (servo %d)\n", angle, steerServo.angle());
        publishStatus();
    } else if (strcmp(command, "PING") == 0) {
        publishStatus();
    }
}

void setup() {
    Serial.begin(115200);
    scanner.begin();
    scanner.setStepDegrees(kScanStepDegrees);
    scanner.setSettleMs(kSettleMs);

    steerServo.begin();  // parks at 90° (wheels straight) at boot

    lcd.begin();
    display.begin();

    mqtt.setCommandCallback(handleCommand);
    mqtt.begin();

    Serial.println("sonar scan service ready");
}

void loop() {
    mqtt.loop();

    if (mqtt.connected()) {
        ScanData scan;
        if (scanner.tick(millis(), scan)) {
            publishScan(scan);
        }
    } else if (scanner.scanning()) {
        // Never scan (or publish) without a live link; stay idle while offline.
        scanner.stop();
    }

    StatusDisplay::State st;
    st.wifiConnected = mqtt.wifiConnected();
    st.mqttConnected = mqtt.connected();
    st.scanning = scanner.scanning();
    st.scanCount = scanCount;
    st.pointCount = lastPointCount;
    st.hasNearest = lastNearestMm > 0;
    st.nearestMm = lastNearestMm;
    st.nearestBearing = lastNearestBearing;
    display.tick(millis(), st);
}
