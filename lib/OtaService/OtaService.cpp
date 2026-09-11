#include "OtaService.h"

OtaService::OtaService(const char* hostname) : hostname_(hostname) {}

void OtaService::begin() {
    if (started_) {
        return;
    }
    started_ = true;
    ArduinoOTA.setHostname(hostname_);
    ArduinoOTA.onStart([this]() {
        if (onStartCb_) {
            onStartCb_();
        }
    });
    ArduinoOTA.begin();
}

void OtaService::handle() {
    if (!started_) {
        return;
    }
    ArduinoOTA.handle();
}

void OtaService::setOnStart(std::function<void()> callback) {
    onStartCb_ = callback;
}