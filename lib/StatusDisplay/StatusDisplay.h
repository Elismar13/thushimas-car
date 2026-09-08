#pragma once

#include <stdint.h>

#include "TextDisplay.h"

// Pure, hardware-free renderer that displays car status on any TextDisplay.
// Renders and auto-rotates a fixed set of 16-char pages on a timer. Reads
// state only; never issues movement or scan commands, so it cannot affect
// safety behavior.
//
// Unit-testable on the native environment: it never touches the LCD library,
// only the TextDisplay interface.
class StatusDisplay {
public:
    struct State {
        bool wifiConnected = false;
        bool mqttConnected = false;
        bool scanning = false;
        unsigned long scanCount = 0;
        int pointCount = 0;            // points in the most recent scan
        bool hasNearest = false;       // whether a valid nearest reading exists
        int nearestMm = 0;             // closest echo distance, mm
        int nearestBearing = 0;        // bearing of closest echo, degrees [0,360)
    };

    static constexpr uint32_t kRotateIntervalMs = 3000;

    explicit StatusDisplay(TextDisplay& display);

    // Render the current page immediately (also clears the screen on first use).
    void begin();

    // Advance the page on the rotation timer and refresh the active page.
    // Call frequently (e.g. every loop()); writes are skipped when unchanged.
    void tick(uint32_t nowMs, const State& state);

    // Number of rotatable pages.
    int pageCount() const;

    // Set the active page and render it. Pages are indices in [0, pageCount()).
    void setPage(int page);

    int currentPage() const { return currentPage_; }

private:
    void renderPage(int page, const State& state);

    TextDisplay& display_;
    int rows_;
    int cols_;
    int currentPage_;
    uint32_t lastRotateMs_;
    bool initialized_;
};
