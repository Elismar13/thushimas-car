#include "StatusDisplay.h"

#include <stdio.h>
#include <string.h>

StatusDisplay::StatusDisplay(TextDisplay& display)
    : display_(display),
      rows_(display.rows()),
      cols_(display.cols()),
      currentPage_(0),
      lastRotateMs_(0),
      initialized_(false) {}

int StatusDisplay::pageCount() const {
    return 3;
}

void StatusDisplay::begin() {
    initialized_ = false;
    renderPage(currentPage_, State{});
}

void StatusDisplay::setPage(int page) {
    if (page < 0) {
        page = 0;
    }
    if (page >= pageCount()) {
        page = pageCount() - 1;
    }
    currentPage_ = page;
    initialized_ = false;
    renderPage(currentPage_, State{});
}

void StatusDisplay::tick(uint32_t nowMs, const State& state) {
    if (!initialized_) {
        initialized_ = true;
        lastRotateMs_ = nowMs;
        renderPage(currentPage_, state);
        return;
    }
    if (nowMs - lastRotateMs_ >= kRotateIntervalMs) {
        currentPage_ = (currentPage_ + 1) % pageCount();
        lastRotateMs_ = nowMs;
        renderPage(currentPage_, state);
    }
}

void StatusDisplay::renderPage(int page, const State& state) {
    char line1[32];
    char line2[32];

    switch (page) {
        case 0: {
            // Status page.
            const char* mode = state.scanning ? "SCAN ON" : "IDLE";
            snprintf(line1, sizeof(line1), "%s", mode);
            line1[cols_] = '\0';
            snprintf(line2, sizeof(line2), "%s%s",
                     state.wifiConnected ? "WIFI:ok " : "WIFI:no  ",
                     state.mqttConnected ? "MQTT:ok" : "MQTT:no ");
            break;
        }
        case 1: {
            // Scan progress page.
            snprintf(line1, sizeof(line1), "scan #%lu", state.scanCount);
            snprintf(line2, sizeof(line2), "pts: %d", state.pointCount);
            break;
        }
        case 2:
        default: {
            // Nearest obstacle page.
            if (state.hasNearest) {
                snprintf(line1, sizeof(line1), "near %dmm", state.nearestMm);
                snprintf(line2, sizeof(line2), "@ %03ddeg", state.nearestBearing);
            } else {
                snprintf(line1, sizeof(line1), "near --mm");
                snprintf(line2, sizeof(line2), "no echo");
            }
            break;
        }
    }

    line1[cols_] = '\0';
    line2[cols_] = '\0';
    display_.printLine(0, line1);
    display_.printLine(1, line2);
}
