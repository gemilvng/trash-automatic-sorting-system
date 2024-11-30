#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>

/* LCD config */
LiquidCrystal_I2C lcd(0x27, 20, 4); // 0x27 as I2C address, 20 as columns, 4 as rows
/* End of LCD config */

/* Servo motor config */
const int PIPE_PWM_PIN = 8; // Servo motor for pipe
const int GATE_PWM_PIN = 21; // Servo motor for gate
Servo servoPipe; // Servo motor for pipe
Servo servoGate; // Servo motor for gate
// Angle to move the servo motor
const int CARDBOARD = 45;
const int METAL_CAN_OR_INITIAL = 90;
const int PLASTIC_BOTTLE = 135;
/* End of servo motor config */

/* Ultrasonic sensor config */
const int TRIG_PIN_1 = 4; const int ECHO_PIN_1 = 5; // 1st
const int TRIG_PIN_2 = 6; const int ECHO_PIN_2 = 7; // 2nd
const int TRIG_PIN_3 = 1; const int ECHO_PIN_3 = 2; // 3rd
float capacity[3]; // variable to hold the capacity of the trash bin
String capacityJSON;
/* End of ultrasonic sensor config */

/* Network config */
// Include the Wi-Fi credentials
#include "wifiCredentials.h"
// Include the cloud server URL
#include "serverCredentials.h"

// Flag to control the HTTP request
bool doHTTPPOSTcapacity = false;
bool doHTTPPOSTtrigger = false;
bool doHTTPGETpredict = false;

//Declaring global variable for storing IP address
IPAddress ipS3;

// Setting up HTTP Client
HTTPClient clientESP32S3;

// String for storing the HTTP Payload
String triggerJSON = "{\"status\":true}";
String httpPayload;

// Interrupt setup to trigger request to camera over network
const int BUTTON_PIN = 47;
unsigned long button_time = 0;
unsigned long last_button_time = 0;

/*Type of trash: 
    1 for cardboard, 
    2 for metal can, 
    3 for plastic bottle*/
int trashType;

// Ultrasonic sensor task
void taskUltrasonicTXRX(int triggerPin, int echoPin) {
    //local variables to hold the duration and distance
    long duration;
    float distance;

    // Start signal transmission and reception
    // Ensure the trigger is low at initial condition
    digitalWrite(triggerPin, LOW); delayMicroseconds(2);

    /* Emitting pulse for 10us, 
    as per HC-SR04 datasheet description about how it works */
    digitalWrite(triggerPin, HIGH); delayMicroseconds(10);
    digitalWrite(triggerPin, LOW); delayMicroseconds(2);
    duration = pulseIn(echoPin, HIGH); // Reads the bounce-back signal
    distance = (duration * 0.0343) / 2; // Calculate the distance
    switch (echoPin) {
        case 5:
        capacity[0] = (distance / 50) * 100;
        lcd.setCursor(10, 1); lcd.print(capacity[0]);
        break;
        case 7:
        capacity[1] = (distance / 50) * 100;
        lcd.setCursor(10, 2); lcd.print(capacity[1]);
        break;
        case 2:
        capacity[2] = (distance / 50) * 100;
        lcd.setCursor(10, 3); lcd.print(capacity[2]);
        break;
    }
}

// Task for handling servo movement
void taskKinematics(int trashType) {
    switch (trashType) {
        case 0:
            lcd.clear();
            lcd.setCursor(0, 0); lcd.print("Type: Cardboard");
            servoPipe.write(CARDBOARD);
            servoGate.write(90);
            lcd.setCursor(0, 1); lcd.print("Put the trash in!");
            delay(3000);
            servoGate.write(0);
            delay(2000);
            servoPipe.write(METAL_CAN_OR_INITIAL);
            break;
        case 1:
            lcd.clear();
            lcd.setCursor(0, 0); lcd.print("Type: Metal Can");
            servoPipe.write(METAL_CAN_OR_INITIAL);
            servoGate.write(90);
            lcd.setCursor(0, 1); lcd.print("Put the trash in!");
            delay(3000);
            servoGate.write(0);
            delay(2000);
            servoPipe.write(METAL_CAN_OR_INITIAL);
            break;
        case 2:
            lcd.clear();
            lcd.setCursor(0, 0); lcd.print("Type: Plastic Bottle");
            servoPipe.write(PLASTIC_BOTTLE);
            servoGate.write(90);
            lcd.setCursor(0, 1); lcd.print("Put the trash in!");
            delay(3000);
            servoGate.write(0);
            delay(2000);
            servoPipe.write(METAL_CAN_OR_INITIAL);
            break;
    }
}

void taskHTTPPOSTtoCloud() {
    capacityJSON = "{\"Metal\": " + String(capacity[0]) + ", \"Plastic\": " + String(capacity[1]) + ", \"Cardboard\": " + String(capacity[2]) + "}";
    clientESP32S3.begin(predictURL);
    clientESP32S3.addHeader("Content-Type", "application/json");
    int httpResponseCode = clientESP32S3.POST(capacityJSON);
    if (httpResponseCode == 200) {
        httpPayload = clientESP32S3.getString();
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Data sent to cloud.");
    } else if (httpResponseCode == 500) {
        httpPayload = clientESP32S3.getString();
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print(httpPayload);
    }
    clientESP32S3.end();
}

void taskHTTPPOSTtrigger() {
    clientESP32S3.begin(addStatusURL);
    clientESP32S3.addHeader("Content-Type", "application/json");
    int httpResponseCode = clientESP32S3.POST(triggerJSON);
    if (httpResponseCode == 201) {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Indentifying,");
        lcd.setCursor(0, 1); lcd.print("please wait...");
    } else if (httpResponseCode == 500 || httpResponseCode == 400) {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Server error.");
    } else {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Network error.");
    }
    clientESP32S3.end();
    doHTTPPOSTtrigger = false;
}

void taskHTTPGETprediction() {
    clientESP32S3.begin(predictURL);
    int httpResponseCode = clientESP32S3.GET();
    if (httpResponseCode == 200) {

    }
}

// Task for handling button input
// reading: https://lastminuteengineers.com/handling-esp32-gpio-interrupts-tutorial/
void IRAM_ATTR taskFlagSetter() {
  button_time = millis();
  if (button_time - last_button_time > 2000) {
    doHTTPPOSTtrigger = true; // Set flag to trigger the HTTPPOSTtrigger request
    last_button_time = button_time;
  }
}

// Task for displaying layout of data
void taskDisplay() {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Capacity (%): ");
    lcd.setCursor(0, 1); lcd.print("Metal: ");
    lcd.setCursor(0, 2); lcd.print("Cardboard: ");
    lcd.setCursor(0, 3); lcd.print("Plastic: ");
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

    /*Flag setter setup*/
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), taskFlagSetter, FALLING);
    /* End of flag setter setup*/

    /*Wi-Fi setup*/
    WiFi.begin(ssid, password);
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Wi-Fi status: ");
    lcd.setCursor(0, 1);
    lcd.print("Connecting...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
    }
    lcd.setCursor(0, 2);
    lcd.print("Wi-Fi Connected!");
    delay(1000);
    // Print the layout of data display
    taskDisplay();

    // TXRX for the first time
    taskUltrasonicTXRX(TRIG_PIN_1, ECHO_PIN_1);
    taskUltrasonicTXRX(TRIG_PIN_2, ECHO_PIN_2);
    taskUltrasonicTXRX(TRIG_PIN_3, ECHO_PIN_3);
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        if (doHTTPPOSTtrigger == true) {
            taskHTTPPOSTtrigger();
        }
    } else {
        do {
            lcd.clear();
            lcd.setCursor(0, 0); lcd.print("Reconnecting ...");
            delay(3000);
        } while (WiFi.status() != WL_CONNECTED);
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Wi-Fi Connected!");
        delay(1000);
        lcd.clear();
        taskDisplay();
        lcd.setCursor(10, 1); lcd.print(capacity[0]);
        lcd.setCursor(10, 2); lcd.print(capacity[1]);
        lcd.setCursor(10, 3); lcd.print(capacity[2]);
    }

    /* Using Serial0 due to json file configuration of ESP32-S3 used in this project.
    Read TArS-IoT-System/platformio.ini for more information. */
}