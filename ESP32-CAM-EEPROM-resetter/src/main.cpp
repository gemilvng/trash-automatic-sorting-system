#include <Arduino.h>
#include "EEPROM.h"

#define EEPROM_SIZE 1 // Define the size of the EEPROM memory

void resetEEPROM() {
    EEPROM.write(0, 0); // Write the reset value (0) to address 0
    EEPROM.commit(); // Commit the changes to save them to EEPROM memory
    Serial.println("EEPROM value reset to 0");
}

void setup() {
    delay(3000); // Spare some time to plug the USB cable :)
    int valueEEPROM;
    char input;
    Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE); // Initialize the EEPROM memory with the defined size
    valueEEPROM = EEPROM.read(0); // Read the value stored in address 0
    Serial.print("EEPROM value: ");
    Serial.println(valueEEPROM);
    Serial.println("Do you want to reset the EEPROM value? (y/n)");
    while (!Serial.available()) {}
    input = Serial.read();
    if (input == 'y'|| input == 'Y') {
        resetEEPROM(); // Reset the EEPROM memory
        valueEEPROM = EEPROM.read(0); // Read the value stored in address 0
        Serial.print("EEPROM value: "); Serial.println(valueEEPROM);
        Serial.println("EEPROM value reset successfully, terminating program ...");
    } else {
      Serial.println("EEPROM value not reset, terminating program ...");
    }
}

void loop() {
}