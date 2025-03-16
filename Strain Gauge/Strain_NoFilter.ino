#include "HX711.h"

#define DATA_PIN 9   // HX711 Data (DOUT)
#define SCK_PIN 8    // HX711 Clock (SCK)
#define RATE_PIN 7   // HX711 RATE pin (Set HIGH for 80 Hz sampling)

HX711 scale;

// Constants
const float V_excitation = 5.0;  // Excitation voltage (V)
const float Gain = 128.0;        // HX711 default gain
const float ADC_Max = 16777216.0; // 2^24 = 16,777,216 (24-bit ADC)

// No filters or zero offset correction

void setup() {
    Serial.begin(115200);

    // **Set HX711 to 80 Hz**
    pinMode(RATE_PIN, OUTPUT);
    digitalWrite(RATE_PIN, HIGH);  // Set RATE pin HIGH for 80 Hz sampling

    scale.begin(DATA_PIN, SCK_PIN);
}

void loop() {
    if (scale.is_ready()) {
        long rawADC = scale.get_units(10);  // Average of 10 readings

        // Convert HX711 ADC output to millivolts (mV)
        float Vout = (rawADC * V_excitation * 1000) / (Gain * ADC_Max);  

        // **Output voltage to Serial Plotter (for tuning)**
        Serial.println(Vout, 6);  // Only voltage, ensures Serial Plotter works correctly

    } else {
        Serial.println("HX711 not ready");
    }

    delay(50);  // Faster refresh rate for 80 Hz sampling
}
