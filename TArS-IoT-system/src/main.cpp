#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LiquidCrystal_I2C lcd(I2C_ADDRESS, COLUMNS, ROWS)
LiquidCrystal_I2C lcd(0x27, 20, 4); // 0x27 as I2C address, 20 as columns, 4 as rows

// declaring pins for 1st ultrasonic sensor
const int TRIG_PIN_1 = 4;
const int ECHO_PIN_1 = 5;

// declaring global variable
// long duration;
// float distance;


void setup() {
    delay(100); // Wait for 60ms noise to disappear

    // LCD setup
    Wire.begin(10, 9); // Wire.begin(SDA, SCL). replacing SDA and SCL with selected pin number
    lcd.begin(20, 4); // lcd.begin(COLUMNS, ROWS). replacing COLUMNS and ROWS with the actual size of LCD
    lcd.backlight(); // Turn on the backlight
    lcd.setCursor(5, 1); // lcd.setCursor(COLUMN, ROW). replacing COLUMN and ROW, indexing starts from 0
    lcd.print("Starting"); // Print the text
    delay(2000); // Wait for 2s
    lcd.setCursor(4, 2);
    lcd.print("the device");
    // Ultrasonic sensor setup
    pinMode(TRIG_PIN_1, OUTPUT); // pinMode(PIN, MODE)
    pinMode(ECHO_PIN_1, INPUT); // parameter are integers but can use aliases
    delay(6000); // Wait for 5s
    lcd.clear(); // Clear the LCD screen
}

void loop() {
    // Start signal transmission and reception
    digitalWrite(TRIG_PIN_1, LOW); // digitalWrite(PIN, VALUE)
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN_1, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN_1, LOW);
    long duration = pulseIn(ECHO_PIN_1, HIGH); // pulseIn(PIN, VALUE). Catch the echo signal
    float distance = (duration * 0.0343) / 2; // Calculate the distance in cm
    
    // Print the result to the LCD
    lcd.setCursor(0, 0);
    lcd.print("Distance (in cm): ");
    lcd.setCursor(0, 1);
    lcd.print(distance);
}