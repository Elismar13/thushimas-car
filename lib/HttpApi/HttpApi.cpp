#include "HttpApi.h"

#include <WiFi.h>

namespace {
constexpr char kTokenHeader[] = "X-Control-Token";
constexpr uint16_t kHttpPort = 80;

int statusCodeFor(ControlStatus status) {
    switch (status) {
        case ControlStatus::Accepted:
            return 200;
        case ControlStatus::Unauthorized:
            return 401;
        case ControlStatus::StaleSequence:
            return 409;
        case ControlStatus::InvalidRequest:
        default:
            return 400;
    }
}
}  // namespace

HttpApi::HttpApi(ControlEndpoint& endpoint)
    : endpoint_(endpoint), server_(kHttpPort) {}

void HttpApi::begin() {
    const char* headers[] = {kTokenHeader};
    server_.collectHeaders(headers, 1);
    server_.on("/api/control", HTTP_POST, [this]() { handleControl(); });
    server_.on("/api/state", HTTP_GET, [this]() { handleState(); });
    server_.begin();
}

void HttpApi::loop() {
    server_.handleClient();
}

EndpointContext HttpApi::context() const {
    EndpointContext result;
    result.nowMs = millis();
    result.link.rssi = WiFi.RSSI();
    result.link.ip = WiFi.localIP().toString().c_str();
    result.link.uptimeMs = millis();
    return result;
}

void HttpApi::handleControl() {
    const String body = server_.arg("plain");
    const String token = server_.header(kTokenHeader);
    const std::string payload = body.c_str();
    const std::string providedToken = token.c_str();

    const ControlResponse response =
        endpoint_.handleControl(providedToken, payload, context());
    sendResponse(response);
}

void HttpApi::handleState() {
    const String token = server_.header(kTokenHeader);
    const std::string providedToken = token.c_str();

    const ControlResponse response = endpoint_.state(providedToken, context());
    sendResponse(response);
}

void HttpApi::sendResponse(const ControlResponse& response) {
    server_.send(statusCodeFor(response.status), "application/json",
                 response.body.c_str());
}
