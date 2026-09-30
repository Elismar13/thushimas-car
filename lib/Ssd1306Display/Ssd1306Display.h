#pragma once

#include <stdint.h>

#include "TextDisplay.h"

// Concrete TextDisplay backed by a 128x64 monochrome SSD1306 OLED over I2C.
// Uses the Adafruit_SSD1306 + Adafruit_GFX libraries.
//
// Wires to the same reserved I2C pins as the previous character LCD:
// SDA = GPIO21, SCL = GPIO22, GND, and VCC = 3.3 V (the panel is 3.3 V logic).
// A long I2C run may need ~4.7 k-ohm pull-ups on SDA/SCL.
//
// Text is laid out with the classic 6x8 GFX font, giving a 21x8 character grid
// on a 128x64 panel. The panel is buffered: printLine() pushes the framebuffer
// to the display and skips the I2C write when the row is unchanged.
class Ssd1306Display : public TextDisplay {
public:
    static constexpr int kDefaultSdaPin = 21;
    static constexpr int kDefaultSclPin = 22;

    // `address` is the SSD1306 7-bit I2C address (commonly 0x3C, sometimes 0x3D).
    Ssd1306Display(uint8_t address, int width, int height,
                   int sdaPin = kDefaultSdaPin, int sclPin = kDefaultSclPin);

    void begin();

    int rows() const override;
    int cols() const override;
    void setCursor(int row, int col) override;
    void print(const char* text) override;
    void printLine(int row, const char* text) override;

private:
    // Classic GFX font cell: 5x7 glyph plus 1 px spacing.
    static constexpr int kCharWidth = 6;
    static constexpr int kCharHeight = 8;
    // Upper bounds for the per-row change cache; the 128x64 panel needs 21x8.
    static constexpr int kMaxRows = 8;
    static constexpr int kMaxCols = 32;

    class Adafruit_SSD1306* oled_;
    int rows_;
    int cols_;
    uint8_t address_;
    int sdaPin_;
    int sclPin_;
    int cursorRow_;
    int cursorCol_;
    // Last text written to each row, so unchanged rows cost no I2C traffic.
    char lastLines_[kMaxRows][kMaxCols + 1];
};
