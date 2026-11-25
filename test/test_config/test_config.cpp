#include <string.h>

#include <unity.h>

#include "config.example.h"

void test_config_example_provides_wifi_settings(void) {
    TEST_ASSERT_NOT_NULL(WIFI_SSID);
    TEST_ASSERT_NOT_NULL(WIFI_PASSWORD);
    TEST_ASSERT_TRUE(strlen(WIFI_SSID) > 0);
    TEST_ASSERT_TRUE(strlen(WIFI_PASSWORD) > 0);
}

void test_config_example_provides_car_hostname(void) {
    TEST_ASSERT_NOT_NULL(CAR_HOSTNAME);
    TEST_ASSERT_TRUE(strlen(CAR_HOSTNAME) > 0);
}

void test_config_example_provides_control_token(void) {
    TEST_ASSERT_NOT_NULL(CONTROL_TOKEN);
    TEST_ASSERT_TRUE(strlen(CONTROL_TOKEN) > 0);
}

void test_config_example_has_no_broker_settings(void) {
#ifdef MQTT_BROKER_HOST
    TEST_FAIL_MESSAGE("the phone-controlled car must not configure an MQTT broker");
#else
    TEST_PASS();
#endif
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_config_example_provides_wifi_settings);
    RUN_TEST(test_config_example_provides_car_hostname);
    RUN_TEST(test_config_example_provides_control_token);
    RUN_TEST(test_config_example_has_no_broker_settings);
    return UNITY_END();
}
