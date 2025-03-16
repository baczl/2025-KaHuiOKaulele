#include "HX711.h"

#define DATA_PIN 9   // HX711 Data (DOUT)
#define SCK_PIN 8    // HX711 Clock (SCK)

HX711 scale;

// Constants
const float V_excitation = 5.0;  // Excitation voltage (V)
const float Gain = 128.0;        // HX711 default gain
const float ADC_Max = 16777216.0; // 2^24 = 16,777,216 (24-bit ADC)
const float GaugeFactor = 2.0;   // Typical strain gauge factor (2.0 - 2.2)

// Offset Values
const float fixedVoltageOffset = 19.531248;  // Fixed offset in millivolts (mV)
const long fixedADCOffset = (fixedVoltageOffset * Gain * ADC_Max) / (V_excitation * 1000);  // Convert to ADC units

// Filter Parameters
#define FILTER_SIZE 10  // Number of readings for moving average
float strainValues[FILTER_SIZE] = {0};
int filterIndex = 0;

// Zero Offset Calibration
float zeroOffset = 0;  
bool zeroed = false;

// Moving Average Filter Function
float movingAverage(float newVal) {
    strainValues[filterIndex] = newVal;
    filterIndex = (filterIndex + 1) % FILTER_SIZE;
    
    float sum = 0;
    for (int i = 0; i < FILTER_SIZE; i++) {
        sum += strainValues[i];
    }
    return sum / FILTER_SIZE;
}

void setup() {
    Serial.begin(115200);
    scale.begin(DATA_PIN, SCK_PIN);

    // **Zero Offset Calibration**
    Serial.println("Calibrating Zero Offset...");
    float sum = 0;
    for (int i = 0; i < 50; i++) {  // Average over 50 readings
        if (scale.is_ready()) {
            sum += scale.get_units();
        }
        delay(10);
    }
    zeroOffset = sum / 50;  // Dynamic zero offset based on readings
    Serial.print("Zero Offset (ADC Units): ");
    Serial.println(zeroOffset, 6);
}

void loop() {
    if (scale.is_ready()) {
        long rawADC = scale.get_units(10);  // Average of 10 readings

        // Apply fixed voltage offset correction (in ADC units)
        rawADC -= fixedADCOffset;

        // Apply dynamic zero offset correction
        rawADC -= zeroOffset;

        // Convert HX711 ADC output to millivolts (mV)
        float Vout = (rawADC * V_excitation * 1000) / (Gain * ADC_Max);  

        // Convert voltage to strain (ε)
        float strain = (Vout / V_excitation) / GaugeFactor;

        // **Apply Moving Average Filter**
        float filteredStrain = movingAverage(strain);

        // **Output strain to Serial Monitor (Smoothed)**
        Serial.print("Strain: ");
        Serial.println(filteredStrain, 9);  // Display with high precision

        // **Output BOTH voltage & strain to Serial Plotter**
        Serial.print(Vout, 6);  // Voltage in mV
        Serial.print(",");      // Separate values with a comma
        Serial.println(filteredStrain, 9);  // Smoothed strain

    } else {
        Serial.println("HX711 not ready");
    }

    delay(50);  // Smooth plotting and monitoring
}
