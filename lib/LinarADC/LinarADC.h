#pragma once

#include <Arduino.h>
#include <driver/dac.h>
#include "FS.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>

/**
 * @class LinarADC
 * @brief A class for handling ADC of ESP32 operations with optional calibration and result storage.
 * 
 * The `LinarADc` class is designed to manage ADC readings, 
 * handle optional calibration processes, and save results to memory in various formats 
 * (".txt", ".json", ".bin"). It provides utilities for LED status indication, calibration,
 *  and storing processed data for further analysis.
 *
 * Key Features:
 * - Perform raw ADC readings or calibrated readings based on the configuration.
 * - Save results of calibration in memory for later use.
 * - Configurable ADC pin and LEDs for visual feedback.
 *
 * Example usage:
 * @code
 * LinarADc adc(34, 2, 4); // Initialize with ADC pin 34, LED1 pin 2, and LED2 pin 4
 * adc.save(); // Do the calibration and save the file as "/CalibrationResults.bin"
 * adc.begin(); // Tries to read the file "/CalibrationResults.bin" and runs ADC
 * adc.read(); //  If the file is read successfully then use values from there, 
 *             //  if not use a polynomial
 * @endcode
 */
class LinarADC {
private:
    bool useCalibration = false;
    
    //System pins 
    int led1Pin;            ///< Pin number for the first LED indicator (green)
    int led2Pin;            ///< Pin number for the second LED indicator (red)
    int adcPinCalib;        ///< ADC pin used for calibration.

    // Work with file
    String fileName;        ///< Name of the file to save results (without extension).
    String fileType;        ///< File type/extension for the saved results (e.g., ".txt").
    String fullPath;        ///< Full file path generated from fileName and fileType.

    // Dynamic arrays to work with calibration values
    float *results;         ///< Array for storing ADC results
    float *res2;            ///< Secondary array for additional processing
    int *calibrationArray;  ///< Array for storing calibration data.

    //For reading 
    const int maxAttempts = 1000; ///< Max attempts to read from a file a variable
    int attempts;                 ///< Number of attempts  
    int value;                    ///< Auxiliary variable to read from the file  
    
    template <typename T>
    void printLUT (T *array){
        debugfcn(formatMessage("const float ADC_LUT[4096] = {"));
        for (int i=0; i<4096; i++) {
            debugfcn(formatMessage("%f,", array[i])); debugfcn(formatMessage(","));
            if ((i%15)==0) debugfcn(formatMessage("\r\n"));
        }
        debugfcn(formatMessage("};\r\n"));
    }

    const char* formatMessage(const char *format, ...);
    void ledIndication(int pin, bool isLong);
    bool triggerLed (const bool status);
    void deleteFile(fs::FS &fs, const char *path);
    bool spiffsRun();
    bool openFile();
    bool saveFile();
    bool writeFloatAsIntToTxt(fs::FS &fs, const char *path, float *array, size_t size);
    bool writeFloatAsIntToBin(fs::FS &fs, const char *path, float *array, size_t size);
    bool writeFloatAsIntToJson(fs::FS &fs, const char *path, float *array, size_t size);
    bool readIntArrayFromJson(fs::FS &fs, const char *path, int *array, size_t size);
    bool readIntArrayFromBin(fs::FS &fs, const char *path, int *array, size_t maxSize);
    bool readIntArrayFromTxt(fs::FS &fs, const char *path, int *array, size_t maxSize);
    void generateLut();
    bool calibration();


public:
    /**
     * @brief Constructor to initialize the LinarADc object.
     * 
     * @param adcPinCalib ADC pin used for calibration.
     * @param led1Pin Pin number for the first LED indicator.
     * @param led2Pin Pin number for the second LED indicator.
     * @param file    Name of the file to be saved/read.
     * @param type    Type of the file to be saved/read (".json", ".txt", ".bin").
     */
    LinarADC(int adcCalibration = 34, String type = ".bin", int led1 = -1, int led2 = -1, String file = "CalibrationResults")
        :adcPinCalib(adcCalibration), fileType(type), led1Pin(led1), led2Pin(led2), fileName(file) {
                  
        results = new float[4097];
        if (results == nullptr) {
            debugfcn(formatMessage("Memory allocation failed for results array!\r\n"));
            ledIndication(led2Pin, true);
        }
        
        res2 = new float[4096 * 5];
        if (res2 == nullptr) {
            debugfcn(formatMessage("Memory allocation failed for res2 array!\r\n"));
            ledIndication(led2Pin, true);
        }

        calibrationArray = new int[4096];
        if (calibrationArray == nullptr) {
            debugfcn(formatMessage("Memory allocation failed for results array!\r\n"));
            ledIndication(led2Pin, true);
        }
        memset(results, 0, sizeof(float) * 4097);
        memset(res2, 0, sizeof(float) * 4096 * 5);
        memset(calibrationArray, 0, sizeof(int) * 4096); 

        debugfcn = [](const char *txt) {};

        fullPath = "/" + fileName + fileType;

        pinMode(led1Pin, OUTPUT);
        pinMode(led2Pin, OUTPUT);
        digitalWrite(led1Pin, HIGH);
        digitalWrite(led2Pin, HIGH);
 
    }

    ~LinarADC() {
    if (results != nullptr) {
        delete[] results;
        results = nullptr;
    }
    if (res2 != nullptr) {
        delete[] res2;
        res2 = nullptr;
    }
    if (calibrationArray != nullptr) {
        delete[] calibrationArray;
        calibrationArray = nullptr;
    }
    }

    void (*debugfcn)(const char *txt);
    bool save(dac_channel_t dacChannel = DAC_CHANNEL_1);
    bool begin();
    int read(const int adcPinRead);
};


