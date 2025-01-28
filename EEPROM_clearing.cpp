#include <Arduino.h>
#include <SPI.h>
#include <HardwareSerial.h>
#include <EEPROM.h>
#include <Wire.h>

#define EEPROM_SIZE 512  // Define the EEPROM size (adjust as needed)

void setup() {
  Serial.begin(115200);
  
  // Initialize EEPROM
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("Failed to initialize EEPROM");
    return;
  }
  
  // Clear EEPROM by writing 0 to all locations
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);  // Set each byte to 0
  }
  
  // Commit changes to EEPROM
  EEPROM.commit();
  Serial.println("EEPROM cleared");
}

void loop() {
  // Do nothing here
}
