# GPS Speedometer for Boat 🚤

![Alt text](images/general_view.jpg)

This project is a fully functional GPS speedometer designed for boats or other purposes. It displays real-time speed, direction, temperature, time, total odometer reading, total engine operating hours, and trip distance on an ILI9341 TFT display.

## Features
* Speed Monitoring: Real-time speed display in km/h.
* Trip and Total Odometers: Track current trip distance and total traveled distance.
* Runtime Tracker: Records and displays the total runtime of the engine.
* Temperature Display: Real-time water or ambient temperature measurement using a DS18B20 sensor.
* GPS Data: Displays location-based data, including time (with timezone adjustment) and heading direction.
* EEPROM Data Persistence: Saves total distance and runtime data to EEPROM for retrieval after a power cycle.

![Alt text](images/tft.jpg)

### Hardware Requirements
* Microcontroller: ESP32 WROOM 30 pins
* Display: ILI9341 TFT 2.8inch 240x320
* GPS Module: NEO-6M
* Temperature Sensor: DS18B20
* 4,7 kOhm pull-up resistor
* PCB breadboard 60x40mm
* RC filter: 0-50V, 2A
* DC-DC converter: MP1584 4.5-28V, 2A
* Other Components: AWG wires, power source, connectors, screws

![Alt text](images/hardware.jpg)

### Libraries Used
* Arduino.h
* Adafruit_GFX
* Adafruit_ILI9341
* TinyGPS++
* SoftwareSerial
* DallasTemperature
* OneWire
* EEPROM

#### How It Works
1. The GPS module continuously streams data, which is processed using the TinyGPS++ library.
2. Data like speed, direction, and time is extracted and displayed on the ILI9341 TFT.
3. The DS18B20 sensor measures temperature, updated every second.
4. Distance traveled and runtime are logged and updated persistently in EEPROM.
5. The display updates only when there's a significant change in the data, optimizing performance.

#### Setup and Installation
1. Install the required libraries in your Arduino IDE.
2. Connect the components as specified in the circuit diagram.
3. Upload the provided code to your ESP32.
4. Ensure the GPS module has a clear view of the sky for accurate data.
5. Power the device and start tracking!

Future Improvements
* Add support for external storage (e.g., SD card) to log trip data.
* Integrate Bluetooth or Wi-Fi for mobile app connectivity.
* Implement additional metrics like altitude and course history.
