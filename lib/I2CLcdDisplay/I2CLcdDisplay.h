#pragma once

#include <stdint.h>

#include "TextDisplay.h"

// Concrete TextDisplay backed by an HD44780-style character LCD with a PCF8574
// I2C backpack (the common 16x2 + "I2C LCM" module). Uses the
// marcoschwartz/LiquidCrystal_I2C library.
//
// Wires to the reserved I2C pins: SDA = GPIO21, SCL = GPIO22, VCC = 5 V logic
// rail, GND. A long I2C run may need ~4.7 k-ohm pull-ups on SDA/SCL.
class I2CLcdDisplay : public TextDisplay {
public:
    // `address` is the PCF8574 7-bit address (commonly 0x27).
    I2CLcdDisplay(uint8_t address, int cols, int rows);

    void begin();

    int rows() const override;
    int cols() const override;
    void setCursor(int row, int col) override;
    void print(const char* text) override;
    void printLine(int row, const char* text) override;

private:
    class LiquidCrystal_I2C* lcd_;
    int rows_;
    int cols_;
    uint8_t address_;
    int cursorRow_;
    int cursorCol_;
};
