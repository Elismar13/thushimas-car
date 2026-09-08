#pragma once

// Character-grid text display interface.
//
// The common denominator shared by a 16x2 I2C LCD, a 20x4 LCD, or an OLED:
// rows and columns of that can be placed and printed. Business logic (see
// lib/StatusDisplay) renders against this interface and is therefore agnostic
// to the physical display. Migrating to a larger or different display means
// dropping in a new implementation, not touching callers.
class TextDisplay {
public:
    virtual ~TextDisplay() = default;

    // Number of text rows and columns (1-indexed dimensions).
    virtual int rows() const = 0;
    virtual int cols() const = 0;

    // Place the cursor at (row, col); both 0-indexed within [0, rows/cols).
    virtual void setCursor(int row, int col) = 0;

    // Print raw text at the current cursor position.
    virtual void print(const char* text) = 0;

    // Print `text` starting at column 0 of `row` and clear the rest of the
    // line (text longer than the row is truncated). Convenience + caching
    // hook: implementations may skip writes that reconstruct no change.
    virtual void printLine(int row, const char* text) = 0;
};
