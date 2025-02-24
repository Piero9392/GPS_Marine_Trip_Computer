/* EEPROM Clear
Sets all of the bytes of the EEPROM to 0.
*/

#include <EEPROM.h>
int val = 4096; // Write the size of EEPROM of your board in bytes (from 0 to all 4096 bytes of the EEPROM)
// If Arduino UNO then replace it with 512
void setup() {
  Serial.begin(9600);
  EEPROM.begin(val); 
  for (int i = 0; i < val; i++) {
    EEPROM.write(i, 0);
    Serial.println(i);
  }
  EEPROM.end();
  Serial.println("EEPROM Clear");
}

void loop() {
} 