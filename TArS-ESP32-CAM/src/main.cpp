#include <Arduino.h>
#include <WiFi.h>

// Include Wi-Fi credentials file, this is not a library
#include "wifi_credentials.h"

// Define pin for Wi-Fi connection indicator
#define INDICATOR_PIN 33

// Declaring global variable for storing IP address
IPAddress camera_IP;

// Defining dummy task
void task1() {
    Serial.println("Excecuting Dummy Task 1.");
    delay(3000); // Small delay so the simulation can be monitored via serial monitor
}

void task2() {
    Serial.println("Excecuting Dummy Task 2.");
    delay(3000);
}

void task3() {
    Serial.println("Excecuting Dummy Task 3.");
    delay(3000);
}

void setup() {
    pinMode(INDICATOR_PIN, OUTPUT); // Set Indicator LED pin as output

    /* Turn off the Indicator LED by set HIGH as parameter.
    This pin behavior is different than the usual. */
    digitalWrite(INDICATOR_PIN, HIGH);

    Serial.begin(115200); // Initialize serial communication
    WiFi.begin(ssid, password); // Attempting to connect to Wi-Fi network
    while (WiFi.status() != WL_CONNECTED) {
        delay(3000);
        Serial.println("Connecting to Wi-Fi...");
    }

    /* Blinking Indicator LED three times to indicate
    successfull connection attempt after booting*/
    for (int i = 0; i < 3; i++) {
        digitalWrite(INDICATOR_PIN, LOW); // Turn on Indicator LED
        delay(1000);
        digitalWrite(INDICATOR_PIN, HIGH); // Turn off Indicator LED
        delay(1000);
    }

    camera_IP = WiFi.localIP(); // Store local IP address to global variable
    // Print the IP address of the camera when connection is established
    Serial.println("Connected. camera IP address: " + camera_IP.toString());
    digitalWrite(INDICATOR_PIN, LOW); // Turn on Indicator LED, Wi-Fi is connected
}

void loop() {
    // Dummy task to simulate camera operation

    /* Ensure All task is executed while connected
    to Wi-Fi network */
    if (WiFi.status() == WL_CONNECTED) {
        task1();
    } else {
        // Attempt to reconnect to Wi-Fi network
        do {
            digitalWrite(INDICATOR_PIN, HIGH); // Turn off LED, Wi-Fi is disconnected
            Serial.println("Task 1 Failed. Reconnecting to Wi-Fi...");
            delay(3000);
        } while (WiFi.status() != WL_CONNECTED);
        digitalWrite(INDICATOR_PIN, LOW); // Turn on Indicator LED after reconnection
        Serial.println("Connected. camera IP address: " + camera_IP.toString());
        task1();
    }

    if (WiFi.status() == WL_CONNECTED) {
        task2();
    } else {
        do {
            digitalWrite(INDICATOR_PIN, HIGH);
            Serial.println("Task 2 Failed. Reconnecting to Wi-Fi...");
            delay(3000);
        } while (WiFi.status() != WL_CONNECTED);
        digitalWrite(INDICATOR_PIN, LOW);
        Serial.println("Connected. camera IP address: " + camera_IP.toString());
        task2();
    }

    if (WiFi.status() == WL_CONNECTED) {
        task3();
    } else {
        do {
            digitalWrite(INDICATOR_PIN, HIGH);
            Serial.println("Task 3 Failed. Reconnecting to Wi-Fi...");
            delay(3000);
        } while (WiFi.status() != WL_CONNECTED);
        digitalWrite(INDICATOR_PIN, LOW);
        Serial.println("Connected. camera IP address: " + camera_IP.toString());
        task3();
    }
}