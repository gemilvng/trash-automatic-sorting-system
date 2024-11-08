#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LiquidCrystal_I2C lcd(I2C_ADDRESS, COLUMNS, ROWS)
LiquidCrystal_I2C lcd(0x27, 20, 4); // 0x27 as I2C address, 20 as columns, 4 as rows

void setup() {
    delay(100); // Wait for 60ms noise to disappear
    Wire.begin(10, 9); // Wire.begin(SDA, SCL). replacing SDA and SCL with selected pin number
    lcd.begin(20, 4); // lcd.begin(COLUMNS, ROWS). replacing COLUMNS and ROWS with the actual size of LCD
    lcd.backlight(); // Turn on the backlight
    lcd.setCursor(1, 1); // lcd.setCursor(COLUMN, ROW). replacing COLUMN and ROW, indexing starts from 0
    lcd.print("Hello, World!"); // Print the text
}

void loop() {
}