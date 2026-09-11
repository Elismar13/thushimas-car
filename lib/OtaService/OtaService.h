#pragma once

#include <ArduinoOTA.h>

#include <functional>

// Thin OTA service around the ESP32 ArduinoOTA client (UDP 3232, the
// "Arduino OTA" protocol that PlatformIO's `espota` uploader speaks). Runs
// alongside MQTT without conflict: MQTT is TCP 1883, OTA is UDP 3232.
//
// During an update the caller's loop() keeps running; the firmware should stop
// the scanner via setOnStart() so no sweep happens mid-flash. After the image
// is written the ESP32 reboots and the app reconnects MQTT as usual.
class OtaService {
public:
    OtaService(const char* hostname);

    // Register ArduinoOTA. MUST be called only after Wi-Fi/lwip is up: the
    // ArduinoOTA client binds its UDP socket (3232) immediately, which panics
    // the ESP32 if the network stack is not initialized yet. Idempotent; safe
    // to defer until the first successful Wi-Fi connection.
    void begin();

    // Service the OTA client. Call frequently from the main loop. No-op until
    // begin() has run.
    void handle();

    // Invoked once when an update begins (e.g. stop any active scanning).
    void setOnStart(std::function<void()> callback);

private:
    const char* hostname_;
    std::function<void()> onStartCb_;
    bool started_ = false;
};