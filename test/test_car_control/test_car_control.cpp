#include <unity.h>

#include "CarControl.h"

CarInputs freshCarLinkInputs() {
    CarInputs inputs;
    inputs.carLinkFresh = true;
    return inputs;
}

void test_car_is_stopped_and_disarmed_on_boot(void) {
    CarControl control;
    CarInputs inputs;

    CarState state = control.update(ControlIntent(), inputs, 0);

    TEST_ASSERT_FALSE(state.armed);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorState::Coast), static_cast<int>(state.motor.state));
    TEST_ASSERT_EQUAL_INT(0, state.requestedThrottle);
    TEST_ASSERT_EQUAL_INT(0, state.motor.actualThrottle);
    TEST_ASSERT_EQUAL_INT(0, state.motor.steeringAngle);
}

void test_disarmed_car_refuses_throttle_and_steering(void) {
    CarControl control;
    ControlIntent intent;
    intent.requestedThrottle = 50;
    intent.steeringAngle = 20;

    CarState state = control.update(intent, freshCarLinkInputs(), 0);

    TEST_ASSERT_FALSE(state.armed);
    TEST_ASSERT_EQUAL_INT(0, state.requestedThrottle);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorState::Coast),
                          static_cast<int>(state.motor.state));
    TEST_ASSERT_EQUAL_INT(0, state.motor.actualThrottle);
    TEST_ASSERT_EQUAL_INT(0, state.motor.steeringAngle);
}

void test_actual_throttle_ramps_to_full_in_three_hundred_milliseconds(void) {
    CarControl control;
    ControlIntent arm;
    arm.arm = ArmIntent::Arm;
    control.update(arm, freshCarLinkInputs(), 0);

    ControlIntent fullThrottle;
    fullThrottle.requestedThrottle = 100;

    CarState after50ms = control.update(fullThrottle, freshCarLinkInputs(), 50);
    TEST_ASSERT_EQUAL_INT(17, after50ms.motor.actualThrottle);

    CarState after300ms = control.update(fullThrottle, freshCarLinkInputs(), 300);
    TEST_ASSERT_EQUAL_INT(100, after300ms.motor.actualThrottle);
}

void test_actual_throttle_does_not_jump_after_a_long_update_gap(void) {
    CarControl control;
    ControlIntent arm;
    arm.arm = ArmIntent::Arm;
    control.update(arm, freshCarLinkInputs(), 0);

    ControlIntent fullThrottle;
    fullThrottle.requestedThrottle = 100;
    CarState state = control.update(fullThrottle, freshCarLinkInputs(), 1000);

    TEST_ASSERT_EQUAL_INT(17, state.motor.actualThrottle);
}

void test_high_frequency_updates_do_not_exceed_the_ramp_rate(void) {
    CarControl control;
    ControlIntent arm;
    arm.arm = ArmIntent::Arm;
    control.update(arm, freshCarLinkInputs(), 0);

    ControlIntent fullThrottle;
    fullThrottle.requestedThrottle = 100;
    CarState state;
    for (uint32_t nowMs = 1; nowMs <= 100; ++nowMs) {
        state = control.update(fullThrottle, freshCarLinkInputs(), nowMs);
    }

    TEST_ASSERT_EQUAL_INT(33, state.motor.actualThrottle);
}

void test_actual_throttle_ramps_down_to_zero(void) {
    CarControl control;
    ControlIntent arm;
    arm.arm = ArmIntent::Arm;
    control.update(arm, freshCarLinkInputs(), 0);

    ControlIntent fullThrottle;
    fullThrottle.requestedThrottle = 100;
    control.update(fullThrottle, freshCarLinkInputs(), 300);

    ControlIntent stop;
    CarState after50ms = control.update(stop, freshCarLinkInputs(), 350);
    TEST_ASSERT_EQUAL_INT(83, after50ms.motor.actualThrottle);

    CarState after300ms = control.update(stop, freshCarLinkInputs(), 650);
    TEST_ASSERT_EQUAL_INT(0, after300ms.motor.actualThrottle);
}

void test_actual_throttle_crosses_zero_without_skipping_it(void) {
    CarControl control;
    ControlIntent arm;
    arm.arm = ArmIntent::Arm;
    control.update(arm, freshCarLinkInputs(), 0);

    ControlIntent forward;
    forward.requestedThrottle = 10;
    control.update(forward, freshCarLinkInputs(), 50);

    ControlIntent reverse;
    reverse.requestedThrottle = -40;
    CarState atZero = control.update(reverse, freshCarLinkInputs(), 100);
    TEST_ASSERT_EQUAL_INT(0, atZero.motor.actualThrottle);

    CarState movingReverse = control.update(reverse, freshCarLinkInputs(), 150);
    TEST_ASSERT_EQUAL_INT(-17, movingReverse.motor.actualThrottle);
}

void test_armed_car_with_zero_throttle_coasts_and_stays_armed(void) {
    CarControl control;
    ControlIntent intent;
    intent.arm = ArmIntent::Arm;
    intent.steeringAngle = 12;

    CarState state = control.update(intent, freshCarLinkInputs(), 0);

    TEST_ASSERT_TRUE(state.armed);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorState::Coast),
                          static_cast<int>(state.motor.state));
    TEST_ASSERT_EQUAL_INT(0, state.motor.actualThrottle);
    TEST_ASSERT_EQUAL_INT(12, state.motor.steeringAngle);
}

void test_arming_applies_the_requested_control(void) {
    CarControl control;
    ControlIntent intent;
    intent.arm = ArmIntent::Arm;
    intent.requestedThrottle = 40;
    intent.steeringAngle = -15;

    CarState state = control.update(intent, freshCarLinkInputs(), 0);

    TEST_ASSERT_TRUE(state.armed);
    TEST_ASSERT_EQUAL_INT(40, state.requestedThrottle);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(MotorState::Drive),
                          static_cast<int>(state.motor.state));
    TEST_ASSERT_EQUAL_INT(-15, state.motor.steeringAngle);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_car_is_stopped_and_disarmed_on_boot);
    RUN_TEST(test_disarmed_car_refuses_throttle_and_steering);
    RUN_TEST(test_actual_throttle_ramps_to_full_in_three_hundred_milliseconds);
    RUN_TEST(test_actual_throttle_ramps_down_to_zero);
    RUN_TEST(test_actual_throttle_crosses_zero_without_skipping_it);
    RUN_TEST(test_armed_car_with_zero_throttle_coasts_and_stays_armed);
    RUN_TEST(test_arming_applies_the_requested_control);
    return UNITY_END();
}
