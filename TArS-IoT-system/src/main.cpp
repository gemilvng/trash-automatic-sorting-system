#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// LiquidCrystal_I2C lcd(I2C_ADDRESS, COLUMNS, ROWS)
LiquidCrystal_I2C lcd(0x27, 20, 4); // 0x27 as I2C address, 20 as columns, 4 as rows

// Servo motor setup
Servo servoPipe; // Servo motor for pipe
int angle; // Angle to move the servo motor

// Pin definition for ultrasonic sensor
const int TRIG_PIN_1 = 4; const int ECHO_PIN_1 = 5; // 1st
const int TRIG_PIN_2 = 6; const int ECHO_PIN_2 = 7; // 2nd
const int TRIG_PIN_3 = 1; const int ECHO_PIN_3 = 2; // 3rd

// Pin definition for Servo motor
const int PIPE_PWM_PIN = 8; // Servo motor for pipe

// Ultrasonic sensor task
void taskTransmission(int triggerPin) {
    // Start signal transmission and reception

    // Ensure the trigger is low at initial condition
    digitalWrite(triggerPin, LOW); delayMicroseconds(2);

    /* Emitting pulse for 10us, as per HC-SR04 datasheet
    description about how it works */
    digitalWrite(triggerPin, HIGH); delayMicroseconds(10);
    digitalWrite(triggerPin, LOW); delayMicroseconds(2); // Set pin low again
}

// Measurement task
float taskMeasurement(int echoPin) {
    long duration;
    float distance;
    duration = pulseIn(echoPin, HIGH); // Reads the bounce-back signal
    distance = (duration * 0.0343) / 2; // Calculate the distance
    return distance;
}

// Task that handle servo movement
void taskKinematics(int angle) {
    // only accept angle between 0 and 180, as per MG996R servo datasheet
    if (angle < 0 || angle > 180) {
        Serial0.println("Invalid angle");
    } else {
        servoPipe.write(angle);
        Serial0.println("servo moved!");
        delay(1500);
        servoPipe.write(0); // Return servo to initial position
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

    /* Ultrasonic sensor setup */
    /* pinMode(PIN, MODE). parameter are in integers but can use aliases */
    pinMode(TRIG_PIN_1, OUTPUT); pinMode(ECHO_PIN_1, INPUT);
    pinMode(TRIG_PIN_2, OUTPUT); pinMode(ECHO_PIN_2, INPUT);
    pinMode(TRIG_PIN_3, OUTPUT); pinMode(ECHO_PIN_3, INPUT);
    /* End of ultrasonic sensor setup */

    /* Servo motor setup */
    servoPipe.attach(PIPE_PWM_PIN); // Attach the servo motor to the pin
    //servoGate.attach(GATE_PWM_PIN); // Attach the servo motor to the pin
    /* End of servo motor setup */
    
    delay(3000); // Wait for 3s
    lcd.clear(); // Clear the LCD screen

    // Print the layout of data display
    lcd.setCursor(0, 0); lcd.print("Distance (in cm): ");
    lcd.setCursor(0, 1); lcd.print("sensor 1: ");
    lcd.setCursor(0, 2); lcd.print("sensor 2: ");
    lcd.setCursor(0, 3); lcd.print("sensor 3: ");
}

void loop() {
    /* Using Serial0 due to json file configuration of ESP32-S3 used in this project.
    Read TArS-IoT-System/platformio.ini for more information. */
    if (Serial0.available()) {
        String input = Serial0.readString();
        if (input.indexOf('\n') != -1) { // Only accept input after pressing enter.
            angle = input.toInt(); // Convert the input to integer
            Serial0.println("Moving servo to " + String(angle) + " degrees ...");
            taskKinematics(angle);
        }
        Serial0.flush(); // regularly empty the serial buffer
    }

    /* Start signal transmission and reception */ 

    // First transmission
    taskTransmission(TRIG_PIN_1);
    lcd.setCursor(10, 1); lcd.print(taskMeasurement(ECHO_PIN_1));

    // Second transmission
    taskTransmission(TRIG_PIN_2);
    lcd.setCursor(10, 2); lcd.print(taskMeasurement(ECHO_PIN_2));

    // Third transmission
    taskTransmission(TRIG_PIN_3);
    lcd.setCursor(10, 3); lcd.print(taskMeasurement(ECHO_PIN_3));

    /* End of signal transmission and reception */ 
}