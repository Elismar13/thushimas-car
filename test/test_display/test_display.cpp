#include <string.h>
#include <unity.h>

#include "ScanBuilder.h"
#include "StatusDisplay.h"
#include "TextDisplay.h"

// Minimal TextDisplay stub: captures rendered lines into a 2x16 grid so the
// pure renderer can be tested on the native host without any LCD hardware.
class FakeDisplay : public TextDisplay {
public:
    FakeDisplay() { clear(); }

    int rows() const override { return 2; }
    int cols() const override { return 16; }
    void setCursor(int row, int col) override { (void)row; (void)col; }
    void print(const char* text) override { (void)text; }

    void printLine(int row, const char* text) override {
        int n = 0;
        while (text[n] && n < 16) {
            grid_[row][n] = text[n];
            ++n;
        }
        while (n < 16) {
            grid_[row][n] = ' ';
            ++n;
        }
        grid_[row][16] = '\0';
    }

    void clear() {
        for (int r = 0; r < 2; ++r) {
            for (int c = 0; c < 16; ++c) {
                grid_[r][c] = ' ';
            }
            grid_[r][16] = '\0';
        }
    }

    const char* line(int row) const { return grid_[row]; }

private:
    char grid_[2][17];
};

// 21x8 stub matching the SSD1306 128x64 text grid, for the multi-line layout.
class WideFakeDisplay : public TextDisplay {
public:
    WideFakeDisplay() { clear(); }

    int rows() const override { return 8; }
    int cols() const override { return 21; }
    void setCursor(int row, int col) override { (void)row; (void)col; }
    void print(const char* text) override { (void)text; }

    void printLine(int row, const char* text) override {
        int n = 0;
        while (text[n] && n < 21) {
            grid_[row][n] = text[n];
            ++n;
        }
        while (n < 21) {
            grid_[row][n] = ' ';
            ++n;
        }
        grid_[row][21] = '\0';
    }

    void clear() {
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 21; ++c) {
                grid_[r][c] = ' ';
            }
            grid_[r][21] = '\0';
        }
    }

    const char* line(int row) const { return grid_[row]; }

private:
    char grid_[8][22];
};

void test_nearest_ignores_no_echo(void) {
    ScanBuilder b;
    b.add(0, 1, 0);      // no echo
    b.add(90, 1, 800);   // bearing 90
    b.add(180, 2, 0);    // no echo
    b.add(270, 2, 500);  // bearing 450 % 360 = 90
    ScanData s = b.finish(1);

    int bearing = -1;
    TEST_ASSERT_EQUAL_UINT16(500, nearestObstacle(s, &bearing));
    TEST_ASSERT_EQUAL_INT(90, bearing);
}

void test_nearest_all_no_echo_returns_zero(void) {
    ScanBuilder b;
    b.add(0, 1, 0);
    b.add(0, 2, 0);
    ScanData s = b.finish(1);

    int bearing = -99;
    TEST_ASSERT_EQUAL_UINT16(0, nearestObstacle(s, &bearing));
}

void test_nearest_empty_scan_returns_zero(void) {
    ScanBuilder b;
    ScanData s = b.finish(1);
    TEST_ASSERT_EQUAL_UINT16(0, nearestObstacle(s, nullptr));
}

void test_display_status_page(void) {
    FakeDisplay disp;
    StatusDisplay sd(disp);
    StatusDisplay::State st;
    st.wifiConnected = true;
    st.mqttConnected = true;
    st.scanning = true;

    sd.begin();
    sd.tick(0, st);

    TEST_ASSERT_TRUE(strncmp(disp.line(0), "SCAN ON", 7) == 0);
    TEST_ASSERT_TRUE(strncmp(disp.line(1), "WIFI:ok MQTT:ok", 14) == 0);
}

void test_display_rotates_pages(void) {
    FakeDisplay disp;
    StatusDisplay sd(disp);
    StatusDisplay::State st;
    st.scanCount = 5;
    st.pointCount = 26;

    sd.begin();
    sd.tick(0, st);        // page 0 (status)
    sd.tick(3000, st);     // -> page 1 (scan)
    TEST_ASSERT_TRUE(strncmp(disp.line(0), "scan #5", 7) == 0);
    TEST_ASSERT_TRUE(strncmp(disp.line(1), "pts: 26", 7) == 0);

    sd.tick(6000, st);     // -> page 2 (nearest)
    TEST_ASSERT_TRUE(strncmp(disp.line(0), "near --mm", 9) == 0);
}

void test_display_nearest_page(void) {
    FakeDisplay disp;
    StatusDisplay sd(disp);
    StatusDisplay::State st;
    st.hasNearest = true;
    st.nearestMm = 1234;
    st.nearestBearing = 45;

    sd.begin();
    sd.tick(0, st);
    sd.tick(3000, st);
    sd.tick(6000, st);     // page 2 (nearest)

    TEST_ASSERT_TRUE(strncmp(disp.line(0), "near 1234mm", 11) == 0);
    TEST_ASSERT_TRUE(strncmp(disp.line(1), "@ 045deg", 8) == 0);
}

void test_display_full_status_page(void) {
    WideFakeDisplay disp;
    StatusDisplay sd(disp);
    StatusDisplay::State st;
    st.wifiConnected = true;
    st.mqttConnected = false;
    st.scanning = true;
    st.scanCount = 42;

    sd.begin();
    sd.tick(0, st);  // page 0 (status), full multi-line layout

    TEST_ASSERT_TRUE(strncmp(disp.line(0), "CAR STATUS", 10) == 0);
    TEST_ASSERT_TRUE(strncmp(disp.line(2), "MODE  : SCAN ON", 15) == 0);
    TEST_ASSERT_TRUE(strncmp(disp.line(3), "WIFI  : OK", 10) == 0);
    TEST_ASSERT_TRUE(strncmp(disp.line(4), "MQTT  : NO", 10) == 0);
    TEST_ASSERT_TRUE(strncmp(disp.line(5), "SCANS : 42", 10) == 0);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_nearest_ignores_no_echo);
    RUN_TEST(test_nearest_all_no_echo_returns_zero);
    RUN_TEST(test_nearest_empty_scan_returns_zero);
    RUN_TEST(test_display_status_page);
    RUN_TEST(test_display_rotates_pages);
    RUN_TEST(test_display_nearest_page);
    RUN_TEST(test_display_full_status_page);
    return UNITY_END();
}
