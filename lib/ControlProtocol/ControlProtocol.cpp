#include "ControlProtocol.h"

#include <ArduinoJson.h>

namespace {

const char* batteryStateName(BatteryState state) {
    switch (state) {
        case BatteryState::Low:
            return "low";
        case BatteryState::Critical:
            return "critical";
        case BatteryState::Ok:
        default:
            return "ok";
    }
}

void serializeSnapshot(const StateSnapshot& snapshot, std::string& body) {
    StaticJsonDocument<1024> document;
    document["protocolVersion"] = "1.0";
    document["armed"] = snapshot.car.armed;
    document["latched"] = snapshot.car.latched;
    document["latchReason"] = snapshot.car.latchReason;
    document["estop"] = snapshot.car.estop;
    document["interlock"] = snapshot.car.interlock;
    document["distanceMm"] = snapshot.car.obstacleDistanceMm;
    document["batteryState"] = batteryStateName(snapshot.car.batteryState);
    document["batteryMv"] = snapshot.batteryMv;
    document["batteryPct"] = snapshot.batteryPct;
    document["actualThrottle"] = snapshot.car.motor.actualThrottle;
    document["requestedThrottle"] = snapshot.car.requestedThrottle;
    document["steering"] = snapshot.car.motor.steeringAngle;
    document["rssi"] = snapshot.link.rssi;
    document["ip"] = snapshot.link.ip;
    document["uptimeMs"] = snapshot.link.uptimeMs;

    char buffer[1024];
    const size_t length = serializeJson(document, buffer, sizeof(buffer));
    body.assign(buffer, length);
}

void serializeError(ControlStatus status, std::string& body) {
    StaticJsonDocument<64> document;
    switch (status) {
        case ControlStatus::Unauthorized:
            document["error"] = "unauthorized";
            break;
        case ControlStatus::StaleSequence:
            document["error"] = "stale_sequence";
            break;
        case ControlStatus::InvalidRequest:
        case ControlStatus::Accepted:
        default:
            document["error"] = "invalid_request";
            break;
    }

    char buffer[96];
    const size_t length = serializeJson(document, buffer, sizeof(buffer));
    body.assign(buffer, length);
}

bool tokenMatches(const std::string& expected, const std::string& provided) {
    if (expected.empty() || expected.size() != provided.size()) {
        return false;
    }

    unsigned char difference = 0;
    for (size_t index = 0; index < expected.size(); ++index) {
        difference |= static_cast<unsigned char>(expected[index]) ^
                      static_cast<unsigned char>(provided[index]);
    }
    return difference == 0;
}

}  // namespace

IntentParseResult ControlIntentParser::parse(const std::string& payload) const {
    IntentParseResult result;
    StaticJsonDocument<256> document;

    if (deserializeJson(document, payload)) {
        return result;
    }

    if (!document.containsKey("t") || !document.containsKey("s") ||
        !document.containsKey("arm") || !document.containsKey("latch") ||
        !document.containsKey("seq")) {
        result.status = IntentParseStatus::MissingField;
        return result;
    }

    if (!document["t"].is<int>() || !document["s"].is<int>() ||
        !document["arm"].is<bool>() || !document["latch"].is<bool>() ||
        !document["seq"].is<uint64_t>()) {
        result.status = IntentParseStatus::Malformed;
        return result;
    }

    result.status = IntentParseStatus::Valid;
    result.intent.requestedThrottle = document["t"].as<int>();
    result.intent.steeringAngle = document["s"].as<int>();

    if (result.intent.requestedThrottle < -100 ||
        result.intent.requestedThrottle > 100 ||
        result.intent.steeringAngle < -50 ||
        result.intent.steeringAngle > 50) {
        result.status = IntentParseStatus::OutOfRange;
        return result;
    }
    result.intent.arm = document["arm"].as<bool>() ? ArmIntent::Arm : ArmIntent::None;
    result.intent.latch = document["latch"].as<bool>() ? LatchIntent::Set
                                                       : LatchIntent::Clear;
    result.intent.sequence = document["seq"].as<uint64_t>();
    return result;
}

ControlProtocol::ControlProtocol(const std::string& token) : token_(token) {}

ControlResponse ControlProtocol::handleControl(const std::string& token,
                                               const std::string& payload,
                                               const EndpointContext& context) {
    ControlResponse response;

    if (!tokenMatches(token_, token)) {
        response.status = ControlStatus::Unauthorized;
        response.snapshot = snapshot(context.link);
        serializeError(response.status, response.body);
        return response;
    }

    const IntentParseResult parsed = parser_.parse(payload);
    if (parsed.status != IntentParseStatus::Valid) {
        response.status = ControlStatus::InvalidRequest;
        response.snapshot = snapshot(context.link);
        serializeError(response.status, response.body);
        return response;
    }

    if (hasLastSequence_ && parsed.intent.sequence <= lastSequence_) {
        response.status = ControlStatus::StaleSequence;
        response.snapshot = snapshot(context.link);
        serializeError(response.status, response.body);
        return response;
    }

    control_.update(parsed.intent, context.inputs, context.nowMs);
    lastSequence_ = parsed.intent.sequence;
    hasLastSequence_ = true;

    response.status = ControlStatus::Accepted;
    response.snapshot = snapshot(context.link);
    serializeSnapshot(response.snapshot, response.body);
    return response;
}

ControlResponse ControlProtocol::state(const std::string& token,
                                      const EndpointContext& context) {
    ControlResponse response;
    response.snapshot = snapshot(context.link);

    if (!tokenMatches(token_, token)) {
        response.status = ControlStatus::Unauthorized;
        serializeError(response.status, response.body);
        return response;
    }

    response.status = ControlStatus::Accepted;
    serializeSnapshot(response.snapshot, response.body);
    return response;
}

StateSnapshot ControlProtocol::snapshot(const LinkInfo& link) const {
    StateSnapshot result;
    result.car = control_.state();
    result.link = link;
    return result;
}
