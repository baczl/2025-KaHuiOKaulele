#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BMP3XX bmp;
unsigned long lastPrintTime = 0;  // Track last time data was printed
const unsigned long printInterval = 500; // Log every 500ms (0.5 seconds)

// **SD Card Setup (Using SDIO on Teensy 4.1)**
File dataFile;

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("BMP388 + Teensy 4.1 SD Card Logger");

    // **Initialize BMP388 Sensor**
    if (!bmp.begin_I2C()) {   // hardware I2C mode
        Serial.println("Could not find a valid BMP3 sensor, check wiring!");
        while (1);
    }

    // Set up oversampling and filter initialization
    bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
    bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
    bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
    bmp.setOutputDataRate(BMP3_ODR_100_HZ);  // Set ODR to 100Hz

    // **Initialize SD Card (Using SDIO)**
    Serial.println("Initializing SD card...");
    if (!SD.begin(BUILTIN_SDCARD)) { // **Uses SDIO (not SPI)**
        Serial.println("SD Card failed or not present!");
        while (1); // Halt execution if SD fails
    }
    Serial.println("SD card initialized.");

    // **Create a unique log file**
    String dataFname = "log1.txt";
    char buf[16];
    int cnt = 1;
    dataFname.toCharArray(buf, 16);

    while (SD.exists(buf)) {
        cnt += 1;
        dataFname = "log" + String(cnt) + ".txt";
        dataFname.toCharArray(buf, 16);
    }

    Serial.println("Logging to: " + dataFname);
    dataFile = SD.open(buf, FILE_WRITE);
    if (!dataFile) {
        Serial.println("Error opening file.");
        while (1);
    }

    // **Write CSV header**
    dataFile.println("Time(ms),Temperature(C),Pressure(hPa),Altitude(m)");
    dataFile.close();
    Serial.println("Start logging...");
}

void loop() {
    unsigned long currentMillis = millis();

    if (!bmp.performReading()) {
        Serial.println("Failed to perform reading :(");
        return;
    }

    // Log data every 500ms (adjustable)
    if (currentMillis - lastPrintTime >= printInterval) {
        lastPrintTime = currentMillis;
        logToSD();
    }
}

// **Function to log BMP388 sensor data to SD card**
void logToSD() {
    File dataFile = SD.open("log1.txt", FILE_WRITE);
    if (dataFile) {
        dataFile.print(millis()); // Timestamp
        dataFile.print(",");
        dataFile.print(bmp.temperature, 2);
        dataFile.print(",");
        dataFile.print(bmp.pressure / 100.0, 2); // Convert to hPa
        dataFile.print(",");
        dataFile.println(bmp.readAltitude(SEALEVELPRESSURE_HPA), 2);
        dataFile.close();
        Serial.println("Logged to SD.");
    } else {
        Serial.println("Error writing to SD");
    }
}
