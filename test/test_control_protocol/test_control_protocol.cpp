#include <string.h>

#include <unity.h>

#include "ControlProtocol.h"

EndpointContext freshContext() {
    EndpointContext context;
    context.inputs.carLinkFresh = true;
    return context;
}

void test_parser_accepts_a_complete_control_intent(void) {
    ControlIntentParser parser;
    const std::string payload =
        R"({"t":40,"s":-15,"arm":true,"latch":false,"seq":7})";

    IntentParseResult result = parser.parse(payload);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(IntentParseStatus::Valid),
                          static_cast<int>(result.status));
    TEST_ASSERT_EQUAL_INT(40, result.intent.requestedThrottle);
    TEST_ASSERT_EQUAL_INT(-15, result.intent.steeringAngle);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(ArmIntent::Arm),
                          static_cast<int>(result.intent.arm));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(LatchIntent::Clear),
                          static_cast<int>(result.intent.latch));
    TEST_ASSERT_EQUAL_UINT64(7, result.intent.sequence);
}

void test_parser_rejects_malformed_json(void) {
    ControlIntentParser parser;

    IntentParseResult result = parser.parse("{\"t\":40");

    TEST_ASSERT_EQUAL_INT(static_cast<int>(IntentParseStatus::Malformed),
                          static_cast<int>(result.status));
}

void test_parser_rejects_a_missing_field(void) {
    ControlIntentParser parser;
    const std::string payload = R"({"t":40,"s":-15,"arm":true,"seq":7})";

    IntentParseResult result = parser.parse(payload);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(IntentParseStatus::MissingField),
                          static_cast<int>(result.status));
}

void test_parser_rejects_a_field_with_the_wrong_type(void) {
    ControlIntentParser parser;
    const std::string payload =
        R"({"t":"40","s":-15,"arm":true,"latch":false,"seq":7})";

    IntentParseResult result = parser.parse(payload);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(IntentParseStatus::Malformed),
                          static_cast<int>(result.status));
}

void test_parser_rejects_a_negative_sequence_counter(void) {
    ControlIntentParser parser;
    const std::string payload =
        R"({"t":0,"s":0,"arm":false,"latch":false,"seq":-1})";

    IntentParseResult result = parser.parse(payload);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(IntentParseStatus::Malformed),
                          static_cast<int>(result.status));
}

void test_parser_rejects_an_out_of_range_throttle(void) {
    ControlIntentParser parser;
    const std::string payload =
        R"({"t":101,"s":0,"arm":true,"latch":false,"seq":7})";

    IntentParseResult result = parser.parse(payload);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(IntentParseStatus::OutOfRange),
                          static_cast<int>(result.status));
}

void test_parser_rejects_an_out_of_range_steering_angle(void) {
    ControlIntentParser parser;
    const std::string payload =
        R"({"t":0,"s":51,"arm":true,"latch":false,"seq":7})";

    IntentParseResult result = parser.parse(payload);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(IntentParseStatus::OutOfRange),
                          static_cast<int>(result.status));
}

void test_endpoint_accepts_an_authenticated_control_intent(void) {
    ControlProtocol protocol("shared-secret");
    EndpointContext context = freshContext();
    context.nowMs = 0;
    const std::string payload =
        R"({"t":40,"s":-15,"arm":true,"latch":false,"seq":1})";

    ControlResponse response =
        protocol.handleControl("shared-secret", payload, context);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(ControlStatus::Accepted),
                          static_cast<int>(response.status));
    TEST_ASSERT_TRUE(response.snapshot.car.armed);
    TEST_ASSERT_EQUAL_INT(40, response.snapshot.car.requestedThrottle);
    TEST_ASSERT_EQUAL_INT(-15, response.snapshot.car.motor.steeringAngle);
}

void test_endpoint_rejects_an_invalid_token_without_changing_the_car(void) {
    ControlProtocol protocol("shared-secret");
    EndpointContext context = freshContext();
    const std::string payload =
        R"({"t":100,"s":0,"arm":true,"latch":false,"seq":1})";

    ControlResponse response =
        protocol.handleControl("wrong-secret", payload, context);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(ControlStatus::Unauthorized),
                          static_cast<int>(response.status));
    TEST_ASSERT_FALSE(response.snapshot.car.armed);
    TEST_ASSERT_EQUAL_INT(0, response.snapshot.car.requestedThrottle);
    TEST_ASSERT_EQUAL_INT(0, response.snapshot.car.motor.actualThrottle);
}

void test_endpoint_has_no_valid_token_when_none_is_configured(void) {
    ControlProtocol protocol("");
    EndpointContext context = freshContext();
    const std::string payload =
        R"({"t":0,"s":0,"arm":false,"latch":false,"seq":1})";

    ControlResponse response = protocol.handleControl("", payload, context);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(ControlStatus::Unauthorized),
                          static_cast<int>(response.status));
}

void test_endpoint_rejects_malformed_intent_without_changing_the_car(void) {
    ControlProtocol protocol("shared-secret");
    EndpointContext context = freshContext();
    const std::string valid =
        R"({"t":40,"s":0,"arm":true,"latch":false,"seq":1})";
    protocol.handleControl("shared-secret", valid, context);

    ControlResponse response =
        protocol.handleControl("shared-secret", "{\"t\":40", context);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(ControlStatus::InvalidRequest),
                          static_cast<int>(response.status));
    TEST_ASSERT_TRUE(response.snapshot.car.armed);
    TEST_ASSERT_EQUAL_INT(40, response.snapshot.car.requestedThrottle);
}

void test_endpoint_rejects_out_of_range_intent_without_changing_the_car(void) {
    ControlProtocol protocol("shared-secret");
    EndpointContext context = freshContext();
    const std::string valid =
        R"({"t":40,"s":0,"arm":true,"latch":false,"seq":1})";
    protocol.handleControl("shared-secret", valid, context);

    const std::string outOfRange =
        R"({"t":101,"s":0,"arm":true,"latch":false,"seq":2})";
    ControlResponse response =
        protocol.handleControl("shared-secret", outOfRange, context);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(ControlStatus::InvalidRequest),
                          static_cast<int>(response.status));
    TEST_ASSERT_EQUAL_INT(40, response.snapshot.car.requestedThrottle);
}

void test_endpoint_drops_stale_and_replayed_sequences(void) {
    ControlProtocol protocol("shared-secret");
    EndpointContext context = freshContext();
    const std::string first =
        R"({"t":40,"s":0,"arm":true,"latch":false,"seq":10})";
    protocol.handleControl("shared-secret", first, context);

    const std::string stale =
        R"({"t":100,"s":0,"arm":true,"latch":false,"seq":9})";
    ControlResponse staleResponse =
        protocol.handleControl("shared-secret", stale, context);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(ControlStatus::StaleSequence),
                          static_cast<int>(staleResponse.status));
    TEST_ASSERT_EQUAL_INT(40, staleResponse.snapshot.car.requestedThrottle);

    ControlResponse replayResponse =
        protocol.handleControl("shared-secret", first, context);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(ControlStatus::StaleSequence),
                          static_cast<int>(replayResponse.status));
    TEST_ASSERT_EQUAL_INT(40, replayResponse.snapshot.car.requestedThrottle);

    const std::string next =
        R"({"t":20,"s":0,"arm":false,"latch":false,"seq":11})";
    ControlResponse nextResponse =
        protocol.handleControl("shared-secret", next, context);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(ControlStatus::Accepted),
                          static_cast<int>(nextResponse.status));
    TEST_ASSERT_EQUAL_INT(20, nextResponse.snapshot.car.requestedThrottle);
}

void test_accepted_response_contains_the_state_snapshot(void) {
    ControlProtocol protocol("shared-secret");
    EndpointContext context = freshContext();
    context.link.rssi = -55;
    context.link.ip = "192.168.1.20";
    context.link.uptimeMs = 1234;
    const std::string payload =
        R"({"t":40,"s":-15,"arm":true,"latch":false,"seq":1})";

    ControlResponse response =
        protocol.handleControl("shared-secret", payload, context);

    TEST_ASSERT_NOT_NULL(strstr(response.body.c_str(), "\"protocolVersion\":\"1.0\""));
    TEST_ASSERT_NOT_NULL(strstr(response.body.c_str(), "\"armed\":true"));
    TEST_ASSERT_NOT_NULL(strstr(response.body.c_str(), "\"latched\":false"));
    TEST_ASSERT_NOT_NULL(strstr(response.body.c_str(), "\"estop\":false"));
    TEST_ASSERT_NOT_NULL(strstr(response.body.c_str(), "\"interlock\":false"));
    TEST_ASSERT_NOT_NULL(strstr(response.body.c_str(), "\"distanceMm\":0"));
    TEST_ASSERT_NOT_NULL(strstr(response.body.c_str(), "\"batteryState\":\"ok\""));
    TEST_ASSERT_NOT_NULL(strstr(response.body.c_str(), "\"actualThrottle\":0"));
    TEST_ASSERT_NOT_NULL(strstr(response.body.c_str(), "\"requestedThrottle\":40"));
    TEST_ASSERT_NOT_NULL(strstr(response.body.c_str(), "\"steering\":-15"));
    TEST_ASSERT_NOT_NULL(strstr(response.body.c_str(), "\"rssi\":-55"));
    TEST_ASSERT_NOT_NULL(strstr(response.body.c_str(), "\"ip\":\"192.168.1.20\""));
    TEST_ASSERT_NOT_NULL(strstr(response.body.c_str(), "\"uptimeMs\":1234"));
}

void test_state_endpoint_requires_the_shared_token(void) {
    ControlProtocol protocol("shared-secret");
    EndpointContext context = freshContext();

    ControlResponse response = protocol.state("", context);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(ControlStatus::Unauthorized),
                          static_cast<int>(response.status));
}

void test_state_endpoint_returns_the_snapshot_to_an_authenticated_caller(void) {
    ControlProtocol protocol("shared-secret");
    EndpointContext context = freshContext();

    ControlResponse response = protocol.state("shared-secret", context);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(ControlStatus::Accepted),
                          static_cast<int>(response.status));
    TEST_ASSERT_NOT_NULL(strstr(response.body.c_str(), "\"armed\":false"));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_parser_accepts_a_complete_control_intent);
    RUN_TEST(test_parser_rejects_malformed_json);
    RUN_TEST(test_parser_rejects_a_missing_field);
    RUN_TEST(test_parser_rejects_a_field_with_the_wrong_type);
    RUN_TEST(test_parser_rejects_a_negative_sequence_counter);
    RUN_TEST(test_parser_rejects_an_out_of_range_throttle);
    RUN_TEST(test_parser_rejects_an_out_of_range_steering_angle);
    RUN_TEST(test_endpoint_accepts_an_authenticated_control_intent);
    RUN_TEST(test_endpoint_rejects_an_invalid_token_without_changing_the_car);
    RUN_TEST(test_endpoint_has_no_valid_token_when_none_is_configured);
    RUN_TEST(test_endpoint_rejects_malformed_intent_without_changing_the_car);
    RUN_TEST(test_endpoint_rejects_out_of_range_intent_without_changing_the_car);
    RUN_TEST(test_endpoint_drops_stale_and_replayed_sequences);
    RUN_TEST(test_accepted_response_contains_the_state_snapshot);
    RUN_TEST(test_state_endpoint_requires_the_shared_token);
    RUN_TEST(test_state_endpoint_returns_the_snapshot_to_an_authenticated_caller);
    return UNITY_END();
}
