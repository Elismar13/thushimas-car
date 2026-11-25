#pragma once

#include <stdint.h>
#include <string>

#include "CarControl.h"
#include "CarTypes.h"

enum class IntentParseStatus : uint8_t {
    Valid,
    Malformed,
    MissingField,
    OutOfRange,
};

struct IntentParseResult {
    IntentParseStatus status = IntentParseStatus::Malformed;
    ControlIntent intent;
};

// Parses the phone-facing JSON control intent into the safety core's domain
// types. It knows JSON, but no transport and no safety rules.
class ControlIntentParser {
public:
    IntentParseResult parse(const std::string& payload) const;
};

enum class ControlStatus : uint8_t {
    Accepted,
    Unauthorized,
    InvalidRequest,
    StaleSequence,
};

struct LinkInfo {
    int rssi = 0;
    std::string ip;
    uint32_t uptimeMs = 0;
};

struct EndpointContext {
    uint32_t nowMs = 0;
    CarInputs inputs;
    LinkInfo link;
};

struct StateSnapshot {
    CarState car;
    LinkInfo link;
    uint32_t batteryMv = 0;
    uint8_t batteryPct = 0;
};

struct ControlResponse {
    ControlStatus status = ControlStatus::InvalidRequest;
    StateSnapshot snapshot;
    std::string body;
};

// The transport-neutral seam for the phone-facing protocol. The protocol
// implementation sits behind this interface; HTTP and future adapters depend
// on it rather than on transport details.
class ControlEndpoint {
public:
    virtual ~ControlEndpoint() {}

    virtual ControlResponse handleControl(const std::string& token,
                                          const std::string& payload,
                                          const EndpointContext& context) = 0;

    virtual ControlResponse state(const std::string& token,
                                  const EndpointContext& context) = 0;
};

// Validates and sequences phone requests, then passes accepted intents to the
// safety core. This implementation owns no sockets and performs no I/O.
class ControlProtocol final : public ControlEndpoint {
public:
    explicit ControlProtocol(const std::string& token);

    ControlResponse handleControl(const std::string& token, const std::string& payload,
                                  const EndpointContext& context) override;

    ControlResponse state(const std::string& token,
                          const EndpointContext& context) override;

private:
    StateSnapshot snapshot(const LinkInfo& link) const;

    std::string token_;
    ControlIntentParser parser_;
    CarControl control_;
    bool hasLastSequence_ = false;
    uint64_t lastSequence_ = 0;
};
