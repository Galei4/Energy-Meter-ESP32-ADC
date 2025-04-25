
## LinarADC Library

## Overview

The LinarADC library is designed to handle Analog-to-Digital Converter (ADC) operations on the ESP32 microcontroller. It supports raw and calibrated ADC readings, linearization via Look-Up Tables (LUTs), and saving calibration results in various formats using SPIFFS. The library is also integrated with a modified version of the EmonLib to allow high-accuracy RMS voltage and frequency measurements.

Key Features
	- **Raw and Calibrated ADC Readings: Perform raw ADC readings or use calibrated values based on configuration.
	- **Integration with Modified EmonLib: Directly measure AC voltage and frequency using EmonLib with LinarADC as its ADC backend.
	- **Calibration Results Storage: Save and retrieve calibration data using SPIFFS in .txt, .json, or .bin formats.
	- **LED Indication: Configurable LED pins provide visual feedback during calibration and reading.
	- **Look-Up Table (LUT) Linearization: Converts raw ADC readings into calibrated values for improved accuracy.
	- **Flexible Debugging: Easily log internal operations using a custom debug function.

## Usage

### Initialization

You can initialize the LinarADC class with or without parameters:

```cpp
LinarADC adc(34, ".bin", 2, 4, "CalibrationResults");
```

Or use the default constructor and set values via methods or external configuration.

With EmonLib Integration

You can pass a pointer to the LinarADC object into the modified EmonLib to enable calibrated readings:

```cpp
EnergyMonitor emon1;
emon1.setADC(&adc);
emon1.voltage(32, 221, 1.7);
```

This allows EmonLib to use LinarADC::read() instead of the standard analogRead.

### Saving Calibration Data

To perform calibration and save the results to SPIFFS:

```cpp
adc.save();
```

This will:
	1.	Enable DAC for test output.
	2.	Generate and store calibration LUT.
	3.	Save to SPIFFS and validate via MSE.

### Starting the ADC

To load calibration and prepare the ADC:

```cpp
adc.begin();
```

This function will:
	•	Set up ADC and resolution.
	•	Attempt to read and validate the calibration file.
	•	Default to polynomial mapping if file is invalid.

### Reading ADC Values

To get a calibrated ADC value:
```cpp
int value = adc.read(34);
```
Returns a calibrated or polynomial-mapped value depending on file availability.

Example

```cpp
#include "LinarADC.h"
#include <Arduino.h>
#include "EmonLib.h"    

LinarADC adc;
EnergyMonitor emon1, emon2;

void setup() {
    Serial.begin(115200);
    delay(1000);

    adc.debugfcn = [](const char *txt) {
        Serial.printf(txt);
    };

    if (!adc.begin()) {
        Serial.println("ADC error"); 
    } else {
        Serial.println("ADC OK");
    }

    emon1.setADC(&adc);
    emon1.voltage(32, 221, 1.7);  // RMS Voltage: input pin, calibration constant, phase shift
    emon2.voltage(32, 221, 1.7); 
}

void loop() {
    emon1.calcVI(40, 2000);
    emon2.calcVI(40, 2000);

    Serial.printf("Voltage (Vrms): %.2f V\n", emon1.Vrms);
    Serial.printf("Frequency: %.2f Hz\n", emon1.frequency);
    Serial.printf("Voltage Uncalibrated (Vrms): %.2f V\n", emon2.Vrms);

    delay(1000);
}
```

## File Formats

The library supports the following formats for calibration data:
	•	.txt: Plain text CSV-style values.
	•	.json: JSON structure with calibration array.
	•	.bin: Compact binary data for efficiency.

## Error Handling
	•	Short Green LED Blink: Successful operation.
	•	Long Red LED Blink: Error in calibration or file handling.

## Dependencies
	•	Arduino.h: Core Arduino functionality
	•	driver/dac.h: DAC control on ESP32
	•	FS.h, SPIFFS.h: File system operations
	•	ArduinoJson.h: For reading/writing .json files
	•	EmonLib.h: Modified EmonLib for calibrated ADC input

## Author

Linar Galeev

