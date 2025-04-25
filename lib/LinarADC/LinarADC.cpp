#include "LinarADC.h"


const char* LinarADC::formatMessage(const char *format, ...) {
    static char buffer[128];
    va_list args;                
    va_start(args, format);      
    vsnprintf(buffer, sizeof(buffer), format, args); 
    va_end(args);                

    return buffer;
}

void LinarADC::ledIndication(int pin, bool isLong) {
    if (pin == -1) return;
    int delayTime = isLong ? 2000 : 250;
    digitalWrite(pin, LOW);
    delay(delayTime);
    digitalWrite(pin, HIGH);
    delay(250);
}

bool LinarADC::triggerLed (const bool status){
    if (status) {
        ledIndication(led1Pin, false); 
        return true;
    } else {
        ledIndication(led2Pin, true);
        return false; 
    }
}

void LinarADC::deleteFile(fs::FS &fs, const char *path) {
    if (fs.remove(path)) {
        debugfcn(formatMessage("- File '%s' deleted\r\n", path));
    } else {
        debugfcn(formatMessage("- Failed to delete file '%s'\r\n", path));
    }
}

bool LinarADC::spiffsRun(){
    if (!SPIFFS.begin(true)) {
        debugfcn(formatMessage("SPIFFS Mount Failed\r\n"));
        ledIndication(led2Pin, true);
        return false;
    }
    return true;
}

bool LinarADC::openFile(){
    if (fileType == ".txt") {
        return readIntArrayFromTxt(SPIFFS, fullPath.c_str(), calibrationArray, 4096);
    } else if (fileType == ".json") {
        return readIntArrayFromJson(SPIFFS, fullPath.c_str(), calibrationArray, 4096);
    } else if (fileType == ".bin") {
        return readIntArrayFromBin(SPIFFS, fullPath.c_str(), calibrationArray, 4096);
    } else {
        debugfcn(formatMessage("- Unsupported file type\r\n"));
        return false;
    }
}

bool LinarADC::saveFile(){
    deleteFile(SPIFFS, fullPath.c_str());

    if (fileType == ".txt") {
        return writeFloatAsIntToTxt(SPIFFS, fullPath.c_str(), results, 4097);
    } else if (fileType == ".bin") {
        return writeFloatAsIntToBin(SPIFFS, fullPath.c_str(), results, 4097);
    } else if (fileType == ".json") {
        return writeFloatAsIntToJson(SPIFFS, fullPath.c_str(), results, 4097);
    } else {
        debugfcn(formatMessage("- Unsupported file type\r\n"));
        ledIndication(led2Pin, true);
        return false;
    }
}

bool LinarADC::writeFloatAsIntToTxt(fs::FS &fs, const char *path, float *array, size_t size) {
    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        debugfcn(formatMessage("- Failed to open file for writing\r\n"));
        return false;
    }
    for (size_t i = 0; i < size; i++) {
        int intValue = static_cast<int>(array[i]);
        file.print(intValue);      
        if (i < size - 1) file.print(",");
        file.flush();
    }
    file.close();
    debugfcn(formatMessage("- Float array saved as .txt\r\n"));
    return true;
}

bool LinarADC::writeFloatAsIntToBin(fs::FS &fs, const char *path, float *array, size_t size) {
    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        debugfcn(formatMessage("- Failed to open file for writing\r\n"));
        return false;
    }
    for (size_t i = 0; i < size; i++) {
        int converted = static_cast<int>(array[i]);
        file.write(reinterpret_cast<uint8_t *>(&converted), sizeof(int));
        file.flush();
    }
    file.close();
    debugfcn(formatMessage("- Float array saved as .bin\r\n"));
    return true;
}

bool LinarADC::writeFloatAsIntToJson(fs::FS &fs, const char *path, float *array, size_t size) {
    JsonDocument jsonCalibrationResults;

    JsonArray jsonArray = jsonCalibrationResults[fileName].to<JsonArray>();
    for (int i = 0; i < size; i++) {
        jsonArray.add(static_cast<int>(array[i]));
    }

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        debugfcn(formatMessage("- Failed to open file for writing\r\n"));
        file.close();
        return false;
    }
    serializeJson(jsonCalibrationResults, file);
    
    file.close();
    debugfcn(formatMessage("- Float array saved as .json\r\n"));
    return true;
}

bool LinarADC::readIntArrayFromJson(fs::FS &fs, const char *path, int *array, size_t size) {
    debugfcn(formatMessage("Reading JSON file and converting data to int array: %s\r\n", path));

    File file = fs.open(path, FILE_READ);
    if (!file) {
        debugfcn(formatMessage("- failed to open file for reading\r\n"));
        return false;
    }

    JsonDocument jsonCalibrationResults;

    DeserializationError error = deserializeJson(jsonCalibrationResults, file);
    if (error) {
        debugfcn(formatMessage("- failed to parse JSON file: %s\r\n", error.c_str()));
        file.close();
        return false;
    }

    JsonArray jsonArray = jsonCalibrationResults[fileName].as<JsonArray>();

    size_t index = 0;

    for (JsonVariant jsonValue : jsonArray) {
        value = jsonValue.as<int>();
        attempts = 0;
        if (index < size) {
            while (array[index] != value) {
                array[index] = value; 
                attempts++;
                if (attempts >= maxAttempts) {
                    debugfcn(formatMessage("- failed to read int at index %d after multiple attempts\r\n", index));
                    file.close();
                    return false;
                }
            }
            array[index++];
        } else {
            debugfcn(formatMessage("- array size exceeded, some values were not read\r\n"));
        break;
        }
    }

    file.close();
    debugfcn(formatMessage("- JSON file successfully read and data saved to int array\r\n"));
    return true;
}

bool LinarADC::readIntArrayFromBin(fs::FS &fs, const char *path, int *array, size_t maxSize) {
    debugfcn(formatMessage("Reading int array from binary file: %s\r\n", path));

    File file = fs.open(path, FILE_READ);
    if (!file || file.isDirectory()) {
        debugfcn(formatMessage("- failed to open file for reading\r\n"));
        return false;
    }

    for (size_t index = 0; index < maxSize && file.available(); ++index) {
        value = 0;
        size_t bytesRead = 0;
        attempts = 0;

        while (bytesRead != sizeof(int)) {
            bytesRead = file.read((uint8_t *)&value, sizeof(int));
            attempts++;
        }
        while (array[index] != value) {
            array[index] = value;
            attempts++;
        }               
        if (attempts >= maxAttempts) {
            debugfcn(formatMessage("- failed to read int at index %d after multiple attempts\r\n", index));
            file.close();
            return false;
        }     
    }

    file.close();
    debugfcn(formatMessage("- int array read from binary file one by one using for\r\n"));
    return true;
}   

bool LinarADC::readIntArrayFromTxt(fs::FS &fs, const char *path, int *array, size_t maxSize) {
    debugfcn(formatMessage("Reading int array from a .txt file: %s\r\n", path));

    File file = fs.open(path);
    if (!file || file.isDirectory()) {
        debugfcn(formatMessage("- failed to open file for reading\r\n"));
        return false;
    }

    String data = file.readString();  
    file.close();

    size_t index = 0;

    char *token = strtok((char *)data.c_str(), ",");
    while (token != nullptr && index < maxSize) {
        attempts = 0;
        value = atoi(token); 
        while (array[index] != value) {
            array[index] = value; 
            attempts++;
        
            if (attempts >= maxAttempts) {
                debugfcn(formatMessage("- failed to read int at index %d after multiple attempts\r\n", index));
                return false;
            }
        }
        token = strtok(nullptr, ",");
        index++;
    }
    debugfcn(formatMessage("- int array read from a .txt file\r\n"));
    return true;
}

void LinarADC::generateLut(){
    debugfcn(formatMessage("Test Linearity "));
    for (int j = 0; j < 500; j++) {
        if (j % 100 == 0) {
            debugfcn(formatMessage("."));
            ledIndication(led1Pin, false);
        }
        for (int i = 0; i < 256; i++) {
            dac_output_voltage(DAC_CHANNEL_1, (i & 0xff));
            delayMicroseconds(100);
            results[i * 16] = 0.9 * results[i * 16] + 0.1 * analogRead(adcPinCalib);
        }
    }

    debugfcn(formatMessage("\r\n"));
    debugfcn(formatMessage("Calculate interpolated values ..\r\n"));
    results[4096] = 4095.0;
    for (int i = 0; i < 256; i++) {
        for (int j = 1; j < 16; j++) {
            results[i * 16 + j] = results[i * 16] + (results[(i + 1) * 16] - results[i * 16]) * (float)j / 16.0;
        }
    }

    debugfcn(formatMessage("Generating LUT ..\r\n"));
    for (int i=0; i<4096; i++) {
        results[i]=0.5 + results[i];
    }

    results[4096]=4095.5000;
    for (int i=0; i<4096; i++) {
        for (int j=0; j<5; j++) {
            res2[i*5+j] = results[i] + (results[(i+1)] - results[i]) * (float)j / (float)10.0;
        }
    }

    for (int i=1; i<4096; i++) {
        int index;
        float minDiff=99999.0;
        for (int j=0; j<(5*4096); j++) {
            float diff=fabs((float)(i) - res2[j]);
            if(diff<minDiff) {
                minDiff=diff;
                index=j;
            }
        }
        results[i]=(float)index;
    }

    for (int i=0; i<(4096); i++) {
        results[i]/=5;
    }

    results[0] = 0;                    // always noise 
    delete[] res2;
    res2 = nullptr;
}

bool LinarADC::calibration(){
    float mseCalibrated;
    float mseRaw;
    int rawReading;

    debugfcn(formatMessage("Testing the file..\r\n"));

    if (!triggerLed(openFile())) return false;

    printLUT(calibrationArray);

    for (int i=1; i<250; i++) {
        dac_output_voltage(DAC_CHANNEL_1, i); 
        delayMicroseconds(100);
        rawReading = analogRead(adcPinCalib);
        mseRaw += (i*16 - rawReading) * (i*16 - rawReading);
        mseCalibrated += (i*16 - calibrationArray[rawReading]) * (i*16 - calibrationArray[rawReading]);     
    }

    mseCalibrated = ((sqrt(mseCalibrated / 249) / 3968) * 100);
    mseRaw = ((sqrt(mseRaw / 249) / 3968) * 100);

    if ( mseCalibrated > 1){       //  3968 array data range (maxValue-minValue)
        debugfcn(formatMessage("Calibration error!\r\n"));
        debugfcn(formatMessage("Mean squared value error is more than 1 %\r\n"));
        deleteFile(SPIFFS, fullPath.c_str());
        return false;
    }
    
    else{
        debugfcn(formatMessage("Uncalibrated mean squared error: '%f' % \r\n", mseRaw ));     //  3968 array data range (maxValue-minValue)
        debugfcn(formatMessage("Calibrated mean squared error: '%f' % \r\n", mseCalibrated));
        ledIndication(led1Pin, true);
        return true;
    }
}

bool LinarADC::save(dac_channel_t dacChannel) {

    //setup
    dac_output_enable(dacChannel);
    dac_output_voltage(dacChannel, 0);
    analogReadResolution(12);
    delay(1000);
    if (!spiffsRun()) return false;
  
    //generate calibration values
    generateLut();
    printLUT(results);

    if (!triggerLed(saveFile())) return false;
  
    delete[] results;
    results = nullptr;
    
    if (!triggerLed(calibration())) return false;   
    return true;
}

bool LinarADC::begin(){

    analogReadResolution(12);
    delay(100);

    if (!spiffsRun()) return false;

    if (!openFile()) {
        debugfcn(formatMessage("- Calibration file not found or invalid, using formula\r\n"));
        return useCalibration = false;
    }

    if (calibrationArray[1000] == 0) { // If the array is zero, then NOT OK 
        debugfcn(formatMessage("- Calibration file not found or invalid, using formula\r\n"));
        return useCalibration = false;
    }
    
    return useCalibration = true;
}

int LinarADC::read(const int adcPinRead){
    int readValue = analogRead(adcPinRead);
    if (useCalibration) {
        return calibrationArray[readValue];
    } else {
        return int(4096 * (-0.000000000000016 * pow(readValue, 4)
                        + 0.000000000118171 * pow(readValue, 3)
                        - 0.000000301211691 * pow(readValue, 2)
                        + 0.001109019271794 * readValue
                        + 0.034143524634089) / 3.3);
    }
    return 0; 
}  
