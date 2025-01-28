#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define TFT_CS 5
#define TFT_RST 4
#define TFT_DC 2

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

#define ONE_WIRE_BUS 15  // Change to the pin you are using for DS18B20 data
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

unsigned long lastUpdateTime = 0;
String lastTimeDisplay = "";
const int timezoneOffset = 2;

float tripDistance = 0;  // Distance for the current trip
float totalDistance = 0; // Total distance over all sessions
unsigned long totalRuntimeMillis = 0;

const int EEPROM_SIZE = 16;  // Increased size to store both runtime and totalDistance

void setupEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  totalRuntimeMillis = EEPROM.readULong(0);
  EEPROM.get(8, totalDistance);
}

void updateEEPROMData(unsigned long currentSessionMillis) {
  totalRuntimeMillis += currentSessionMillis;
  EEPROM.writeULong(0, totalRuntimeMillis);
  EEPROM.put(8, totalDistance);
  EEPROM.commit();
}

void displayOdometer(float distanceKm) {
  static float lastOdometerDisplay = -1;
  if (fabs(distanceKm - lastOdometerDisplay) > 0.1) {  // Update only if a significant change
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%d km", distanceKm);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);
    tft.fillRect(5, tft.height() - 20, w, h, ILI9341_BLACK);  // Clear previous value
    tft.setCursor(5, tft.height() - 20);
    tft.print(buffer);
    lastOdometerDisplay = distanceKm;
  }
}

void displayTripOdometer(float distanceKm) {
  static float lastTripDisplay = -1;
  if (fabs(distanceKm - lastTripDisplay) > 0.1) {  // Update only if a significant change
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%.1f km", distanceKm);
    tft.setTextSize(3);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);
    int centerX = (tft.width() - w) / 2;
    int centerY = ((tft.height() + h) / 2) + 45;
    tft.fillRect(centerX, centerY - h, w, h, ILI9341_BLACK);  // Clear previous value
    tft.setCursor(centerX, centerY);
    tft.print(buffer);
    lastTripDisplay = distanceKm;
  }
}

void displaySpeed(double speed) {
  static String lastSpeedText = "";  // Track the last displayed speed
  char speedText[10];
  sprintf(speedText, "%d", (int)speed);
  String currentSpeedText = String(speedText);

  if (lastSpeedText != currentSpeedText) {
    int16_t x1, y1;
    uint16_t w, h;
    tft.setTextSize(10);
    tft.getTextBounds(speedText, 0, 0, &x1, &y1, &w, &h);
    tft.fillRect(0, (tft.height() - h) / 2 - 10, tft.width(), h + 20, ILI9341_BLACK);

    int centerX = (tft.width() - w) / 2;
    int centerY = (tft.height() - h) / 2;

    tft.setCursor(centerX, centerY);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.print(speedText);

    tft.setTextSize(2);
    tft.setCursor(centerX + w, centerY + 30);
    tft.print(" km/h");

    lastSpeedText = currentSpeedText;
  }
}

void displayTime() {
  if (gps.time.isValid()) {
    int hour = gps.time.hour() + timezoneOffset;
    if (hour >= 24) {
      hour -= 24;
    }
    char timeBuffer[6];
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d", hour, gps.time.minute());

    if (lastTimeDisplay != String(timeBuffer)) {
      tft.setTextSize(3);
      tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
      int16_t x1, y1;
      uint16_t w, h;
      tft.getTextBounds(timeBuffer, 0, 0, &x1, &y1, &w, &h);
      tft.fillRect((tft.width() - w) / 2, 5, w, h, ILI9341_BLACK);
      tft.setCursor((tft.width() - w) / 2, 5);
      tft.print(timeBuffer);
      lastTimeDisplay = String(timeBuffer);
    }
  }
}

void displayTotalRuntime() {
  static String lastRuntimeDisplay = "";  // Track the last displayed runtime
  unsigned long totalMinutes = totalRuntimeMillis / 60000;
  unsigned long hours = totalMinutes / 60;
  unsigned long minutes = totalMinutes % 60;

  char runtimeBuffer[6];
  snprintf(runtimeBuffer, sizeof(runtimeBuffer), "%02lu:%02lu", hours, minutes);
  String currentRuntimeDisplay = String(runtimeBuffer);

  if (currentRuntimeDisplay != lastRuntimeDisplay) {
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(runtimeBuffer, 0, 0, &x1, &y1, &w, &h);
    tft.fillRect(tft.width() - w - 5, tft.height() - 20, w, h, ILI9341_BLACK);
    tft.setCursor(tft.width() - w - 5, tft.height() - 20);
    tft.print(runtimeBuffer);
    lastRuntimeDisplay = currentRuntimeDisplay;
  }
}

void displayDirection() {
  if (gps.course.isValid()) {
    String direction;
    double course = gps.course.deg();

    if ((course >= 337.5) || (course < 22.5)) direction = "N";
    else if (course < 67.5) direction = "NE";
    else if (course < 112.5) direction = "E";
    else if (course < 157.5) direction = "SE";
    else if (course < 202.5) direction = "S";
    else if (course < 247.5) direction = "SW";
    else if (course < 292.5) direction = "W";
    else direction = "NW";

    static String lastDirectionDisplay = "";
    if (lastDirectionDisplay != direction) {
      tft.setTextSize(3);
      tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
      int16_t x1, y1;
      uint16_t w, h;
      tft.getTextBounds(direction.c_str(), 10, 10, &x1, &y1, &w, &h);
      tft.fillRect(5, 5, w + 20, h, ILI9341_BLACK);  // Clear previous direction
      tft.setCursor(5, 5);
      tft.print(direction);
      lastDirectionDisplay = direction;
    }
  }
}

void displayTemperature() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  static int lastTemperature = -999;
  static unsigned long lastTempUpdateTime = 0;
  int currentTemperature = static_cast<int>(round(tempC));

  if (abs(currentTemperature - lastTemperature) >= 1 || (millis() - lastTempUpdateTime >= 1000)) {
    char tempBuffer[10];
    snprintf(tempBuffer, sizeof(tempBuffer), "%dC", currentTemperature);
    tft.setTextSize(3);
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK); 

    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(tempBuffer, 0, 0, &x1, &y1, &w, &h);

    int rectWidth = 100;
    int rectHeight = 35;
    int x = tft.width() - rectWidth;
    int y = 0;
    tft.fillRect(x, y, rectWidth, rectHeight, ILI9341_BLACK);
    tft.setCursor(tft.width() - w - 5, 5);
    tft.print(tempBuffer);

    lastTemperature = currentTemperature;
    lastTempUpdateTime = millis();
  }
}

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);
  setupEEPROM();

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);

  sensors.begin();  // Initialize the temperature sensor
}

void loop() {
  static unsigned long lastLoopTime = millis();
  static unsigned long lastGeneralUpdate = millis();  // Track general update timing

  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  unsigned long currentMillis = millis();
  
  // Always display temperature, total runtime, and odometer (using totalDistance)
  displayTemperature();
  displayTotalRuntime();
  displayOdometer(totalDistance);

  if (gps.location.isUpdated()) {
    unsigned long elapsedMillis = currentMillis - lastUpdateTime;
    float distanceIncrement = gps.speed.kmph() * (elapsedMillis / 3600000.0);
    tripDistance += distanceIncrement;
    totalDistance += distanceIncrement;
    lastUpdateTime = currentMillis;

    displaySpeed(gps.speed.kmph());
    displayTripOdometer(tripDistance);
    displayTime();
    displayDirection();
  }

  // Save runtime data every 10 seconds
  if (currentMillis - lastLoopTime >= 10000) {
    updateEEPROMData(10000);
    lastLoopTime = currentMillis;
  }

  // General update interval for non-GPS data every 1 second (adjust as needed)
  if (currentMillis - lastGeneralUpdate >= 1000) {
    lastGeneralUpdate = currentMillis;
  }
}