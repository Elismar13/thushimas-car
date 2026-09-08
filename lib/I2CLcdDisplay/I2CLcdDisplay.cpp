#include "I2CLcdDisplay.h"

#include <LiquidCrystal_I2C.h>

I2CLcdDisplay::I2CLcdDisplay(uint8_t address, int cols, int rows)
    : lcd_(new LiquidCrystal_I2C(address, cols, rows)),
      rows_(rows),
      cols_(cols),
      address_(address),
      cursorRow_(0),
      cursorCol_(0) {}

void I2CLcdDisplay::begin() {
    lcd_->init();
    lcd_->backlight();
    lcd_->clear();
}

int I2CLcdDisplay::rows() const { return rows_; }
int I2CLcdDisplay::cols() const { return cols_; }

void I2CLcdDisplay::setCursor(int row, int col) {
    cursorRow_ = row;
    cursorCol_ = col;
    lcd_->setCursor(col, row);
}

void I2CLcdDisplay::print(const char* text) {
    lcd_->print(text);
}

void I2CLcdDisplay::printLine(int row, const char* text) {
    setCursor(row, 0);
    char line[cols_ + 1];
    int i = 0;
    while (text[i] && i < cols_) {
        line[i] = text[i];
        ++i;
    }
    while (i < cols_) {
        line[i] = ' ';
        ++i;
    }
    line[cols_] = '\0';
    lcd_->print(line);
}
