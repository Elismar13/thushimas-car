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
    if (rows_ <= 2) {
        renderCompact(page, state);
    } else {
        renderFull(page, state);
    }
}

void StatusDisplay::renderCompact(int page, const State& state) {
    char line1[32];
    char line2[32];

    switch (page) {
        case 0: {
            // Status page.
            const char* mode = state.scanning ? "SCAN ON" : "IDLE";
            snprintf(line1, sizeof(line1), "%s", mode);
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

    const int last = (cols_ < static_cast<int>(sizeof(line1)) - 1)
                         ? cols_
                         : static_cast<int>(sizeof(line1)) - 1;
    line1[last] = '\0';
    line2[last] = '\0';
    display_.printLine(0, line1);
    display_.printLine(1, line2);
}

void StatusDisplay::renderFull(int page, const State& state) {
    constexpr int kMaxLines = 8;
    constexpr int kLineBuffer = 32;

    char lines[kMaxLines][kLineBuffer];
    for (int r = 0; r < kMaxLines; ++r) {
        lines[r][0] = '\0';
    }

    // Separator ruler under each page title, matching the display width.
    char sep[kLineBuffer];
    const int sepLen = (cols_ < kLineBuffer - 1) ? cols_ : kLineBuffer - 1;
    for (int i = 0; i < sepLen; ++i) {
        sep[i] = '=';
    }
    sep[sepLen] = '\0';

    switch (page) {
        case 0: {
            // Status page.
            snprintf(lines[0], kLineBuffer, "CAR STATUS");
            snprintf(lines[1], kLineBuffer, "%s", sep);
            snprintf(lines[2], kLineBuffer, "MODE  : %s",
                     state.scanning ? "SCAN ON" : "IDLE");
            snprintf(lines[3], kLineBuffer, "WIFI  : %s",
                     state.wifiConnected ? "OK" : "NO");
            snprintf(lines[4], kLineBuffer, "MQTT  : %s",
                     state.mqttConnected ? "OK" : "NO");
            snprintf(lines[5], kLineBuffer, "SCANS : %lu", state.scanCount);
            break;
        }
        case 1: {
            // Scan progress page.
            snprintf(lines[0], kLineBuffer, "SCAN");
            snprintf(lines[1], kLineBuffer, "%s", sep);
            snprintf(lines[2], kLineBuffer, "COUNT : %lu", state.scanCount);
            snprintf(lines[3], kLineBuffer, "POINTS: %d", state.pointCount);
            break;
        }
        case 2:
        default: {
            // Nearest obstacle page.
            snprintf(lines[0], kLineBuffer, "NEAREST");
            snprintf(lines[1], kLineBuffer, "%s", sep);
            if (state.hasNearest) {
                snprintf(lines[2], kLineBuffer, "DIST  : %d mm", state.nearestMm);
                snprintf(lines[3], kLineBuffer, "BEAR  : %03d deg",
                         state.nearestBearing);
            } else {
                snprintf(lines[2], kLineBuffer, "DIST  : -- mm");
                snprintf(lines[3], kLineBuffer, "BEAR  : ---");
            }
            break;
        }
    }

    const int n = (rows_ < kMaxLines) ? rows_ : kMaxLines;
    for (int r = 0; r < n; ++r) {
        display_.printLine(r, lines[r]);
    }
}
