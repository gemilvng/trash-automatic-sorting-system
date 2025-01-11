// Library for using the arduino framework to program ESP32-S3
#include <Arduino.h>

// I2C module library
#include <Wire.h>
#include <math.h>

// Library for using 20x4 LCD with I2C module
#include <LiquidCrystal_I2C.h>

// Servo motor library for controlling the servo motor with ESP32
#include <ESP32Servo.h>

// Network library (Connect to Wi-Fi and send HTTP request)
#include <WiFi.h>
#include <HTTPClient.h>

/* LCD config
- Using 0x27 as I2C address
- Config the LCD to display in 20 columns and 4 rows
*/
LiquidCrystal_I2C lcd(0x27, 20, 4);

/* Servo motor config
- Using pin 8 as PWM transmitter servo motor to move the sorting pipe
- Using pin 21 as PWM transmitter servo motor to move the trash bin gate
- If Cardboard waste detected, the pipe will move to 70 degrees
- If Metal Can waste detected, the pipe will move to 90 degrees
- If Plastic Bottle waste detected, the pipe will move to 110 degrees
- Creating object instance for each servo motor: servoPipe and servoGate
*/
const int PIPE_PWM_PIN = 8;
const int GATE_PWM_PIN = 21;
Servo servoPipe;
Servo servoGate;
const int CARDBOARD = 70;
const int METAL_CAN_OR_INITIAL = 90;
const int PLASTIC_BOTTLE = 110;

/* Ultrasonic sensor config
- Mapping the trigger and echo pin for each ultrasonic sensor
- Pin 4 and 5 for sensor placed to measure the cardboard trash bin capacity
- Pin 6 and 7 for sensor placed to measure the metal can trash bin capacity
- Pin 1 and 2 for sensor placed to measure the plastic bottle trash bin capacity
- Declaring an array to store the capacity of each trash bin
*/
const int TRIG_PIN_0 = 4; const int ECHO_PIN_0 = 5;
const int TRIG_PIN_1 = 6; const int ECHO_PIN_1 = 7;
const int TRIG_PIN_2 = 1; const int ECHO_PIN_2 = 2;
int capacity[3];

/* Network config
- Include the Wi-Fi credentials stored in wifiCredentials.h
- Include the server URL stored in serverCredentials.h
- Setting up flag as boolean variable to control the HTTP request
    - doHTTPPOSTtrigger: to trigger the camera to capture the image
    - doHTTPGETprediction: to get the prediction result from the server
    - doHTTPPOSTcapacity: to update the capacity of the trash bin to the server
- Creating object instance of HTTPClient: clientESP32S3
- Declaring a string variable to store the HTTP payload
- Declaring a variable to store the encoded prediction result
*/
#include "wifiCredentials.h"
#include "serverCredentials.h"
bool doHTTPPOSTtrigger = false;
bool doHTTPGETprediction = false;
bool doHTTPPOSTcapacity = false;
HTTPClient clientESP32S3;
String HTTPpayloadJSON;
int predictionResult;

/* Interrupt config
- Interrupt handle to set the flag value when the button is pressed
- Button is used to update the value in server
- The value is used by ESP32-CAM to capture the image
*/
const int BUTTON_PIN = 47;
unsigned long button_time = 0;
unsigned long last_button_time = 0;

/* taskUltrasonicTXRX() function
- Function to handle the transmission and reception of the ultrasonic sensor
- Has two parameters: triggerPin and echoPin
- Has three local variables: duration, distance, and readingResult
- Emitting pulse for 10us via the triggerPin with digitalWrite() function
- Reading the bounce-back signal from the echoPin with pulseIn() function
- Calculating the capacity of the trash bin and store it in the capacity array
*/
void taskUltrasonicTXRX(int triggerPin, int echoPin) {
    long duration;
    float distance;
    float readingResult;

    digitalWrite(triggerPin, LOW); delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH); delayMicroseconds(10);
    digitalWrite(triggerPin, LOW); delayMicroseconds(2);
    duration = pulseIn(echoPin, HIGH);
    distance = (duration * 0.0343) / 2;
    switch (echoPin) {
        case 5:
        readingResult = round((1 - (distance / 50)) * 100);
        capacity[0] = (int)readingResult;
        break;
        case 7:
        readingResult = round((1 - (distance / 50)) * 100);
        capacity[1] = (int)readingResult;
        break;
        case 2:
        readingResult = round(( 1- (distance / 50)) * 100);
        capacity[2] = (int)readingResult;
        break;
    }
}

/* taskKinematics() function
- Function to handle the kinematics of the servo motor
- Has one parameter: trashType, store the encoded prediction result
    - 0: Cardboard
    - 1: Metal Can
    - 2: Plastic Bottle
- Implementing switch-case to handle the servo motor movement based on the trash type
- Display the type of trash detected on the LCD using lcd.print() function
- Move the pipe to the designated angle based on the trash type using servoPipe.write()
- Open the gate to allow the trash to fall into the trash bin using servoGate.write()
- Tell the user to put the trash in the pipe using lcd.print() function
- Close the gate after the trash is detected using servoGate.write()
*/
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

/* taskHTTPPOSTtrigger() function
- Function to handle the HTTP POST request to trigger the camera
- Start the HTTP request by using .begin() method
- Constructing the HTTP payload in JSON format, to set the status to "true"
    - Fill the HTTP payload header with .addHeader() method
    - Fill the HTTPPayloadJSON String, the body of the request
- Send the HTTP request with .POST() method
- Implement error handling using if-else statement
- Function is called when the button is pressed
- End the HTTP request with .end() method
- Set this following flag value:
    - doHTTPPOSTtrigger: false, to ensure the function is called once
    - doHTTPGETprediction: true, to run the taskHTTPGETprediction() function
*/
void taskHTTPPOSTtrigger() {
    clientESP32S3.begin(addStatusURL);
    clientESP32S3.addHeader("Content-Type", "application/json");
    HTTPpayloadJSON = "{\"status\":true}";
    int httpResponseCode = clientESP32S3.POST(HTTPpayloadJSON);
    if (httpResponseCode == 201|| httpResponseCode == 200) {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Sending request");
        lcd.setCursor(0, 1); lcd.print("to server ...");
    } else if (httpResponseCode == 500 || httpResponseCode == 400) {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Server error");
        clientESP32S3.end();
        doHTTPPOSTtrigger = false;
        return;
    } else {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Network error");
        clientESP32S3.end();
        doHTTPPOSTtrigger = false;
        return;
    }
    clientESP32S3.end();
    doHTTPPOSTtrigger = false;
    doHTTPGETprediction = true;
}

/* taskHTTPGETprediction() function
- Function to handle the HTTP GET request to get the prediction result
- Start the HTTP request by using .begin() method
- Get the JSON payload from the server with .GET() method
- The JSON payload will be received in this format, stored in HTTPpayloadJSON:
    {
        "prediction_id": "a-prediction-id",
        "scan_id": "a-scan-id",
        "timestamp": "date-when-the-image-was-scanned",
        "detected_type": "metal/cardboard/plastic",
        "image_url": "image-url"
    }
- Implement search mechanism to find waste type by using indexOf() function
- Implement error handling using if-else statement
- Encode the prediction result based on the detected type of trash
    - 0: Cardboard
    - 1: Metal Can
    - 2: Plastic Bottle
- End the HTTP request with .end() method
- Set this following flag value:
    - doHTTPGETprediction: false, to ensure the function is called only once
    - doHTTPPOSTcapacity: true, to sort the waste and update the capacity of 1/3 trash bin
*/
void taskHTTPGETprediction() {
    clientESP32S3.begin(getPredictionURL);
    int httpResponseCode = clientESP32S3.GET();
    HTTPpayloadJSON = clientESP32S3.getString();

    if (httpResponseCode == 200 && HTTPpayloadJSON.indexOf("\"detected_type\"") != -1) {
        if (HTTPpayloadJSON.indexOf("\"paper\"") != -1) {
            predictionResult = 0;
        } else if (HTTPpayloadJSON.indexOf("\"metal\"") != -1) {
            predictionResult = 1;
        } else if (HTTPpayloadJSON.indexOf("\"plastic\"") != -1) {
            predictionResult = 2;
        }
    } else if (httpResponseCode == 500) {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Server error");
        clientESP32S3.end();
        doHTTPGETprediction = false;
        return;
    } else {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Request failed");
        clientESP32S3.end();
        doHTTPGETprediction = false;
        return;
    }
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Processing,");
    lcd.setCursor(0, 1); lcd.print("please wait ...");
    clientESP32S3.end();
    doHTTPGETprediction = false;
    doHTTPPOSTcapacity = true;
}

/* taskHTTPPOSTcapacity() function
- Function to handle the HTTP POST request to update the capacity of the trash bin
- Start the HTTP request by using .begin() method
- Constructing the HTTP payload in JSON format, to update the capacity of the trash bin
    - Fill the HTTP payload header with .addHeader() method
    - Fill the HTTPPayloadJSON String, the body of the request
        - bin_id: the ID of the trash bin
        - fullness_level_cm: the capacity of the trash bin
- Send the HTTP request with .POST() method
- Implement error handling using if-else statement
- End the HTTP request with .end() method
- Set this following flag value:
    - doHTTPPOSTcapacity: false, to ensure the function is called only once
*/
void taskHTTPPOSTcapacity(const char* binID, int capacity) {
    clientESP32S3.begin(updateCapacityURL);
    clientESP32S3.addHeader("Content-Type", "application/json");
    HTTPpayloadJSON = "{\"bin_id\": \"" + String(binID) + "\", \"fullness_level_cm\": " + String(capacity) + "}";
    int httpResponseCode = clientESP32S3.POST(HTTPpayloadJSON);
    if (httpResponseCode == 201) {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Data sent to cloud");
    } else if (httpResponseCode == 400) {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Invalid request");
    } else if (httpResponseCode == 500) {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("Server error");
    }
    clientESP32S3.end();
    doHTTPPOSTcapacity = false;
}

// taskFlagSetter() function, to set the flag value when the button is pressed
// read: https://lastminuteengineers.com/handling-esp32-gpio-interrupts-tutorial/
void IRAM_ATTR taskFlagSetter() {
  button_time = millis();
  if (button_time - last_button_time > 2000) {
    doHTTPPOSTtrigger = true;
    last_button_time = button_time;
  }
}

// taskDisplay() function, to display the data layout on the LCD
void taskDisplay() {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Capacity (%): ");
    lcd.setCursor(0, 1); lcd.print("Cardboard: ");
    lcd.setCursor(11, 1); lcd.print(String(capacity[0]));
    lcd.setCursor(0, 2); lcd.print("Metal Can: ");
    lcd.setCursor(11, 2); lcd.print(String(capacity[1]));
    lcd.setCursor(0, 3); lcd.print("Plastic: ");
    lcd.setCursor(11, 3); lcd.print(String(capacity[2]));
}

/* setup() function
- Function to initialize the device
- Initialize the I2C configuration using Wire.begin() method
- Initialize the LCD configuration using lcd.begin() method
- Configure pins for the ultrasonic sensor using pinMode() function
- Configure pins for the servo motor PWM transmitter ussing .attach() method
- Configure interrupt for the button using pinMode() and attachInterrupt() function
- Initialize the Wi-Fi connection using WiFi.begin() method
- Clear the display before print any new string using lcd.clear() method
- Implement error handling using while loop
    - in case the Wi-Fi connection is not established, the loop will keep going
    - loop breaks when the Wi-Fi connection is established
- Measuring the capacity of of each trash bin once the device is powered on and online
- Display the data layout on the LCD using taskDisplay() function
*/
void setup() {
    delay(100);

    Wire.begin(10, 9);
    lcd.begin(20, 4);
    lcd.backlight();

    lcd.setCursor(5, 1); lcd.print("Starting");
    lcd.setCursor(4, 2); lcd.print("the device");

    pinMode(TRIG_PIN_0, OUTPUT); pinMode(ECHO_PIN_0, INPUT);
    pinMode(TRIG_PIN_1, OUTPUT); pinMode(ECHO_PIN_1, INPUT);
    pinMode(TRIG_PIN_2, OUTPUT); pinMode(ECHO_PIN_2, INPUT);

    servoPipe.attach(PIPE_PWM_PIN); 
    servoGate.attach(GATE_PWM_PIN);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), taskFlagSetter, FALLING);

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

    taskUltrasonicTXRX(TRIG_PIN_0, ECHO_PIN_0);
    taskUltrasonicTXRX(TRIG_PIN_1, ECHO_PIN_1);
    taskUltrasonicTXRX(TRIG_PIN_2, ECHO_PIN_2);
    taskDisplay();
}

/* loop() function
- Function to run the device, repeatedly
- Implementing error handling using if-else statement
    - in case the device is offline, it will reconnect to Wi-Fi network before doing anything else
    - only executing the HTTP request task when the Wi-Fi is connected
- Ensure chained, serial execution of the task by checking the flag value in each if-else statement
- Hardcoded 45s delay due to poor performance of server's ML inference
    - Sending HTTP GET request too soon will result in getting previous prediction result
*/
void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        if (doHTTPPOSTtrigger == true) {
            taskHTTPPOSTtrigger();
            delay(45000);
        } else if (doHTTPGETprediction == true) {
            taskHTTPGETprediction();
            delay(1000);
        } else if (doHTTPPOSTcapacity == true) {
            switch (predictionResult) {
                case 0:
                    taskKinematics(0);
                    taskUltrasonicTXRX(TRIG_PIN_0, ECHO_PIN_0);
                    taskHTTPPOSTcapacity(cardboardBinID, capacity[0]);
                    taskDisplay();
                    break;
                case 1:
                    taskKinematics(1);
                    taskUltrasonicTXRX(TRIG_PIN_1, ECHO_PIN_1);
                    taskHTTPPOSTcapacity(metalCanBinID, capacity[1]);
                    taskDisplay();
                    break;
                case 2:
                    taskKinematics(2);
                    taskUltrasonicTXRX(TRIG_PIN_2, ECHO_PIN_2);
                    taskHTTPPOSTcapacity(plasticBinID, capacity[2]);
                    taskDisplay();
                    break;
            }
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
    }
}