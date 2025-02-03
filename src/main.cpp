#include "LinarADC.h"
#include <Arduino.h>
#include "EmonLib.h"    

// LinarADC adc(34, ".bin", 14, 26);
LinarADC adc;

EnergyMonitor emon1, emon2;

void setup(){

    Serial.begin(115200);
    delay(1000);

    adc.debugfcn = [](const char *txt) {
        Serial.printf(txt);
    };

    //  Load File. begin ADC
    if (!adc.begin()) {
        Serial.println("ADC error"); 
    }

    Serial.println("ADC OK");

    emon1.setADC(&adc);

    emon1.voltage(32, 221, 1.7);  // Voltage: input pin, calibration, phase_shift

    emon2.voltage(32, 221, 1.7); 

};

void loop(){
    emon1.calcVI(40,2000);
    emon2.calcVI(40,2000);
    // Извлекаем RMS-напряжение:
    double Vrms = emon1.Vrms;  // RMS-напряжение
    double Vrms1 = emon2.Vrms;
    float freq = emon1.frequency;;
    
    Serial.print("Voltage (Vrms): ");
    Serial.print(Vrms);
    Serial.println(" V");

    Serial.print("Frequency: ");
    Serial.print(freq);
    Serial.println(" Hz");

    Serial.print("Voltage Uncalibrated (Vrms): ");
    Serial.print(Vrms1);
    Serial.println(" V");

    delay(1000); 

}


