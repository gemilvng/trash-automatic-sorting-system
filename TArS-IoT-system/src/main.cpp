#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "wifi_credentials.h"

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 20, 4); // 0x27 as I2C address, 20 as columns, 4 as rows

// Servo motor setup
const int PIPE_PWM_PIN = 8; // Servo motor for pipe
const int GATE_PWM_PIN = 21; // Servo motor for gate
Servo servoPipe; // Servo motor for pipe
Servo servoGate; // Servo motor for gate
int angle; // Angle to move the servo motor

// Ultrasonic sensor setup
const int TRIG_PIN_1 = 4; const int ECHO_PIN_1 = 5; // 1st
const int TRIG_PIN_2 = 6; const int ECHO_PIN_2 = 7; // 2nd
const int TRIG_PIN_3 = 1; const int ECHO_PIN_3 = 2; // 3rd
long duration; // variable to hold signal measurement duration
float distance; // variable to hold distance measurement

// HTTP-related setup
bool buttonFlag = 0;
HTTPClient esp32s3HTTPClient;

// Button setup
const int BUTTON_PIN = 47; // Button pin
const int debounceDelay = 250; // Debounce delay
unsigned long lastDebounceTime = 0; // Last debounce time
unsigned long buttonPressStartTime = 0; // Last button press time

// Ultrasonic sensor task
void taskUltrasonicTXRX(int triggerPin, int echoPin, bool flag) {
    if (flag == 1) { // Read: when button is pressed ...
        return;
    } else if (flag == 0) { // Read: when button is not pressed ...
        // Start signal transmission and reception

        // Ensure the trigger is low at initial condition
        digitalWrite(triggerPin, LOW); delayMicroseconds(2);

        /* Emitting pulse for 10us, as per HC-SR04 datasheet
        description about how it works */
        digitalWrite(triggerPin, HIGH); delayMicroseconds(10);
        digitalWrite(triggerPin, LOW); delayMicroseconds(2);
        duration = pulseIn(echoPin, HIGH); // Reads the bounce-back signal
        distance = (duration * 0.0343) / 2; // Calculate the distance
        switch (echoPin) {
            case 5:
                lcd.setCursor(10, 1); lcd.print(distance);
                break;
            case 7:
                lcd.setCursor(10, 2); lcd.print(distance);
                break;
            case 2:
                lcd.setCursor(10, 3); lcd.print(distance);
                break;
        }
    }
}

// Task for handling servo movement
void taskKinematics(int angle) {
    // only accept angle between 0 and 180, as per MG996R servo datasheet
    if (angle < 0 || angle > 180) {
        Serial0.println("Invalid angle");
    } else {
        servoPipe.write(angle); servoGate.write(90); // Move the servo motor
        Serial0.println("Put your trash now!");
        delay(3000); servoGate.write(0); // Close the gate
        delay(2000); servoPipe.write(0); // Return servo to initial position
    }
}

void taskHTTPRequest() {
    esp32s3HTTPClient.begin("http://172.20.10.14/takephoto");
    int httpResponseCode = esp32s3HTTPClient.GET();
    if (httpResponseCode > 0) {
        String payload = esp32s3HTTPClient.getString();
        angle = payload.toInt();
        taskKinematics(angle);
        Serial0.println(httpResponseCode);
        Serial0.println(payload);
    } else {
        Serial0.println("Error on HTTP request");
    }
    esp32s3HTTPClient.end();
    buttonFlag = 0;
}

// Task for handling button input
// reading: https://lastminuteengineers.com/handling-esp32-gpio-interrupts-tutorial/
void taskButton(int buttonPin) {
    if (digitalRead(buttonPin) == LOW) {
        if ((millis() - lastDebounceTime) > debounceDelay) {
            lastDebounceTime = millis();
            buttonFlag = 1;
            taskHTTPRequest();
        }
    }
}

void setup() {
    delay(100); // Wait for 60ms noise to disappear

    // Initialize serial monitor to print the distance
    Serial0.begin(115200);

    /* I2C module of the LCD setup */
    //Wire.begin(SDA, SCL). replace SDA and SCL with selected pin numbers.
    Wire.begin(10, 9);

    // lcd.begin(COLUMNS, ROWS). replace COLUMNS and ROWS 
    // with the actual size of the LCD you're using
    lcd.begin(20, 4);
    lcd.backlight(); // Turn on the backlight

    lcd.setCursor(5, 1); lcd.print("Starting");
    lcd.setCursor(4, 2); lcd.print("the device");
    /* End I2C module of the LCD setup */

    /*Button setup*/
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    /* End of button setup*/

    /*Wi-Fi setup*/
    WiFi.begin(ssid, password);
    lcd.clear();
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        lcd.setCursor(0, 0);
        lcd.println("Connecting...");
    }
    lcd.setCursor(1, 0);
    lcd.println("Connected");
    lcd.clear();

    /* Ultrasonic sensor setup */
    /* pinMode(PIN, MODE). parameter are in integers but can use aliases */
    pinMode(TRIG_PIN_1, OUTPUT); pinMode(ECHO_PIN_1, INPUT);
    pinMode(TRIG_PIN_2, OUTPUT); pinMode(ECHO_PIN_2, INPUT);
    pinMode(TRIG_PIN_3, OUTPUT); pinMode(ECHO_PIN_3, INPUT);
    /* End of ultrasonic sensor setup */

    /* Servo motor setup */
    servoPipe.attach(PIPE_PWM_PIN); // Attach the servo motor to the pin
    servoGate.attach(GATE_PWM_PIN); // Attach the servo motor to the pin
    /* End of servo motor setup */

    // Print the layout of data display
    lcd.setCursor(0, 0); lcd.print("Distance (in cm): ");
    lcd.setCursor(0, 1); lcd.print("sensor 1: ");
    lcd.setCursor(0, 2); lcd.print("sensor 2: ");
    lcd.setCursor(0, 3); lcd.print("sensor 3: ");
}

void loop() {
    /* Using Serial0 due to json file configuration of ESP32-S3 used in this project.
    Read TArS-IoT-System/platformio.ini for more information. */
    
    taskButton(BUTTON_PIN);

    /* Start signal transmission and reception */

    // First transmission
    taskUltrasonicTXRX(TRIG_PIN_1, ECHO_PIN_1, buttonFlag);

    // Second transmission
    taskUltrasonicTXRX(TRIG_PIN_2, ECHO_PIN_2, buttonFlag);

    // Third transmission
    taskUltrasonicTXRX(TRIG_PIN_3, ECHO_PIN_3, buttonFlag);

    /* End of signal transmission and reception */ 
}