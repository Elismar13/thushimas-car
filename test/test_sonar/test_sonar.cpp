#include <unity.h>

#include "ScanBuilder.h"

void test_bearing_sensor1_forward(void) {
    TEST_ASSERT_EQUAL_INT(0, bearingFor(0, 1));
    TEST_ASSERT_EQUAL_INT(90, bearingFor(90, 1));
    TEST_ASSERT_EQUAL_INT(180, bearingFor(180, 1));
}

void test_bearing_sensor2_opposite(void) {
    TEST_ASSERT_EQUAL_INT(180, bearingFor(0, 2));
    TEST_ASSERT_EQUAL_INT(270, bearingFor(90, 2));
}

void test_bearing_wraps_to_zero(void) {
    TEST_ASSERT_EQUAL_INT(0, bearingFor(180, 2));
}

void test_bearing_never_negative(void) {
    TEST_ASSERT_EQUAL_INT(180, bearingFor(0, 2));
    TEST_ASSERT_TRUE(bearingFor(0, 2) >= 0 && bearingFor(0, 2) < 360);
}

void test_builder_counts_points(void) {
    ScanBuilder builder;
    builder.add(0, 1, 100);
    builder.add(0, 2, 200);
    TEST_ASSERT_EQUAL_UINT32(2, builder.pointCount());
}

void test_builder_finish_sorts_and_stamps(void) {
    ScanBuilder builder;
    builder.add(180, 2, 100);  // bearing 360 -> 0
    builder.add(90, 1, 300);   // bearing 90
    builder.add(0, 2, 200);    // bearing 180
    builder.add(45, 1, 400);   // bearing 45

    ScanData scan = builder.finish(12345);

    TEST_ASSERT_EQUAL_UINT32(12345, scan.timestampMs);
    TEST_ASSERT_EQUAL_UINT32(4, scan.points.size());
    TEST_ASSERT_TRUE(scan.points[0].bearing <= scan.points[1].bearing);
    TEST_ASSERT_TRUE(scan.points[1].bearing <= scan.points[2].bearing);
    TEST_ASSERT_TRUE(scan.points[2].bearing <= scan.points[3].bearing);
    TEST_ASSERT_EQUAL_INT(0, scan.points[0].bearing);
    TEST_ASSERT_EQUAL_INT(45, scan.points[1].bearing);
    TEST_ASSERT_EQUAL_INT(90, scan.points[2].bearing);
    TEST_ASSERT_EQUAL_INT(180, scan.points[3].bearing);
}

void test_builder_resets_after_finish(void) {
    ScanBuilder builder;
    builder.add(0, 1, 100);
    ScanData scan = builder.finish(1);
    TEST_ASSERT_EQUAL_UINT32(1, scan.points.size());
    TEST_ASSERT_EQUAL_UINT32(0, builder.pointCount());
}

void test_full_sweep_produces_complete_dataset(void) {
    ScanBuilder builder;
    for (int angle = 0; angle <= 180; angle += 15) {
        builder.add(angle, 1, 1000);
        builder.add(angle, 2, 1000);
    }
    ScanData scan = builder.finish(1);
    TEST_ASSERT_EQUAL_UINT32(26, scan.points.size());
    TEST_ASSERT_EQUAL_INT(0, scan.points.front().bearing);
    TEST_ASSERT_EQUAL_INT(345, scan.points.back().bearing);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_bearing_sensor1_forward);
    RUN_TEST(test_bearing_sensor2_opposite);
    RUN_TEST(test_bearing_wraps_to_zero);
    RUN_TEST(test_bearing_never_negative);
    RUN_TEST(test_builder_counts_points);
    RUN_TEST(test_builder_finish_sorts_and_stamps);
    RUN_TEST(test_builder_resets_after_finish);
    RUN_TEST(test_full_sweep_produces_complete_dataset);
    return UNITY_END();
}
