#include "Ssd1306Display.h"

#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include <string.h>

Ssd1306Display::Ssd1306Display(uint8_t address, int width, int height,
                               int sdaPin, int sclPin)
    : oled_(new Adafruit_SSD1306(width, height, &Wire, -1)),
      rows_(height / kCharHeight),
      cols_(width / kCharWidth),
      address_(address),
      sdaPin_(sdaPin),
      sclPin_(sclPin),
      cursorRow_(0),
      cursorCol_(0) {
    if (rows_ > kMaxRows) {
        rows_ = kMaxRows;
    }
    if (cols_ > kMaxCols) {
        cols_ = kMaxCols;
    }
    for (int r = 0; r < kMaxRows; ++r) {
        lastLines_[r][0] = '\0';
    }
}

bool Ssd1306Display::begin() {
    Wire.begin(sdaPin_, sclPin_);
    if (!oled_->begin(SSD1306_SWITCHCAPVCC, address_)) {
        // Some modules strap the alternate address (0x3D vs 0x3C).
        const uint8_t alt = (address_ == 0x3C) ? 0x3D : 0x3C;
        if (!oled_->begin(SSD1306_SWITCHCAPVCC, alt)) {
            return false;
        }
        address_ = alt;
    }
    oled_->setTextColor(SSD1306_WHITE);
    oled_->setTextSize(1);
    oled_->clearDisplay();
    oled_->display();
    for (int r = 0; r < kMaxRows; ++r) {
        lastLines_[r][0] = '\0';
    }
    return true;
}

int Ssd1306Display::rows() const { return rows_; }
int Ssd1306Display::cols() const { return cols_; }

void Ssd1306Display::setCursor(int row, int col) {
    cursorRow_ = row;
    cursorCol_ = col;
    oled_->setCursor(col * kCharWidth, row * kCharHeight);
}

void Ssd1306Display::print(const char* text) {
    oled_->print(text);
}

void Ssd1306Display::printLine(int row, const char* text) {
    if (row < 0 || row >= rows_) {
        return;
    }

    char line[kMaxCols + 1];
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

    if (strcmp(line, lastLines_[row]) == 0) {
        return;  // Row unchanged; skip the I2C write.
    }
    strncpy(lastLines_[row], line, kMaxCols);
    lastLines_[row][kMaxCols] = '\0';

    setCursor(row, 0);
    oled_->print(line);
    oled_->display();
}
