#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LiquidCrystal_I2C lcd(I2C_ADDRESS, COLUMNS, ROWS)
LiquidCrystal_I2C lcd(0x27, 20, 4); // 0x27 as I2C address, 20 as columns, 4 as rows

// declaring pins for 1st ultrasonic sensor
const int TRIG_PIN_1 = 4; const int ECHO_PIN_1 = 5;
const int TRIG_PIN_2 = 6; const int ECHO_PIN_2 = 7;
const int TRIG_PIN_3 = 1; const int ECHO_PIN_3 = 2;

// declaring global variable
long duration_1, duration_2, duration_3;
float distance_1, distance_2, distance_3;


void setup() {
    delay(100); // Wait for 60ms noise to disappear

    // Initialize serial monitor to print the distance
    //Serial.begin(115200);
    //Serial.println("Starting the device");

    // I2C module of the LCD setup

    // Wire.begin(SDA, SCL). replace SDA and SCL with selected pin numbers.
    Wire.begin(10, 9);

    // lcd.begin(COLUMNS, ROWS)
    // replace COLUMNS and ROWS with the actual size of the LCD you're using
    lcd.begin(20, 4);
    lcd.backlight(); // Turn on the backlight

    // Print the starting message
    // lcd.setCursor(COLUMN, ROW). replace COLUMN and ROW, indexing starts from 0
    // Print the text starting from row 1, column 5
    lcd.setCursor(5, 1); lcd.print("Starting");
    lcd.setCursor(4, 2); lcd.print("the device");

    // Turn on red LED
    // pinMode(38, OUTPUT); // pinMode(PIN, MODE)
    // digitalWrite(38, HIGH); // digitalWrite(PIN, VALUE)

    // Ultrasonic sensor setup

    // pinMode(PIN, MODE)
    pinMode(TRIG_PIN_1, OUTPUT); pinMode(ECHO_PIN_1, INPUT); // parameter are integers but can use aliases
    pinMode(TRIG_PIN_2, OUTPUT); pinMode(ECHO_PIN_2, INPUT);
    pinMode(TRIG_PIN_3, OUTPUT); pinMode(ECHO_PIN_3, INPUT);
    //pinMode(TRIG_PIN_3, OUTPUT); pinMode(ECHO_PIN_3, INPUT);
    delay(3000); // Wait for 3s
    lcd.clear(); // Clear the LCD screen
    lcd.setCursor(0, 0); lcd.print("Distance (in cm): ");
    lcd.setCursor(0, 1); lcd.print("sensor 1: ");
    lcd.setCursor(0, 2); lcd.print("sensor 2: ");
    lcd.setCursor(0, 3); lcd.print("sensor 3: ");
}

void loop() {
    // Start signal transmission and reception

    // First transmission
    digitalWrite(TRIG_PIN_1, LOW); delayMicroseconds(2);
    digitalWrite(TRIG_PIN_1, HIGH); delayMicroseconds(10); // emitting pulse
    digitalWrite(TRIG_PIN_1, LOW); delayMicroseconds(2);
    duration_1 = pulseIn(ECHO_PIN_1, HIGH);
    distance_1 = (duration_1 * 0.0343) / 2;
    lcd.setCursor(10, 1); lcd.print(distance_1);
    delay(1000); // Wait for 1s

    // Second transmission
    digitalWrite(TRIG_PIN_2, LOW); delayMicroseconds(2);
    digitalWrite(TRIG_PIN_2, HIGH); delayMicroseconds(10);
    digitalWrite(TRIG_PIN_2, LOW); delayMicroseconds(2);
    duration_2 = pulseIn(ECHO_PIN_2, HIGH);
    distance_2 = (duration_2 * 0.0343) / 2;
    lcd.setCursor(10, 2); lcd.print(distance_2);
    delay(1000); // Wait for 1s

    // Third transmission
    digitalWrite(TRIG_PIN_3, LOW); delayMicroseconds(2);
    digitalWrite(TRIG_PIN_3, HIGH); delayMicroseconds(10);
    digitalWrite(TRIG_PIN_3, LOW); delayMicroseconds(2);
    duration_3 = pulseIn(ECHO_PIN_3, HIGH);
    distance_3 = (duration_3 * 0.0343) / 2;
    lcd.setCursor(10, 3); lcd.print(distance_3);
    delay(1000); // Wait for 1s
}