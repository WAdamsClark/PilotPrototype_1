// PeripheralFunctions.h

#ifndef _PERIPHERALFUNCTIONS_H
#define _PERIPHERALFUNCTIONS_H

/*========== Relevant Addresses and Commands ==========*/
const int led = D7;           // LED pin on Boron development board
const int sgPin = A0;         // Strain Gauge pin for analogRead
const int MPU_SLAVE_ADDR_1 = 0x68;      // I2C address of MPU-6050 if AD0 pin is set low (if set high, 0x69)
const int PWR_MGMT_1 = 0x6B;            // Power management register address for MPU-6050 wakeup calls
const int NRML_PWR = 0;                 // Send 0 for normal mode on wakeup
const int ACCEL_REG_ADDR = 0x3B;        // I2C address of ACCEL_XOUT_H

/*========== Communication Variables ==========*/
int result = 0;
int timestep = 100;                     // Determines how quickly main loop cycles [ms]

/*========== Data Variables ==========*/
// Constants
const float accelSensitivity2g = 16384;         // Sensitivity of the accelerometer at +/-2g [LSB/g]
const float gyroSensitivity250 = 131;           // Resolution of the gyroscope at +/-250deg/s [LSB/deg/s]
const float tempSensitivityUntrimmed = 340;     // Sensitivity of the temperature sensor when untrimmed at 340 [LSB/deg C]
const float tempOffset = 36.53;                 // Offset value needed for calculating temperature in deg C
float accelMagnitude = 0;                       // Holds magnitude of accelerometer gravity vector
const float accelCalibration_1[3] = {-0.010359028, 0.224212988, -0.025359311};  // Calibration values for accelerometer on slave device 1
const float gyroCalibration_1[3] = {3.396479224, 0.218546759, -1.138894073};    // Calibration values for gyroscope on slave device 1
const int num = 5;         // length of buffers used to hold previous measurements
float accelBuffer[3][num];  // Buffer to hold previous accelerometer measurements with [x][y], x holds values for each axis
float gyroBuffer[3][num];   // Buffer to hold previous gyroscope measurements with [x][y], x holds values for each axis
int y = 0;                  // Index variable to keep track of [y] place in accelBuffer and gyroBuffer
int startup = 0;            // State variable to determine if first time running through filter
float est[3];               // Vector to hold current angle estimates
float sumAccel = 0;         // Sum of all values in accelBuffer
float sumGyro = 0;          // Sum of all values in gyroBuffer
float avgAccel = 0;         // Average of all values in accelBuffer
float avgGyro = 0;          // Average of all values in gyroBuffer

/*========== Configure MPU-6050 ==========*/
void configSensor(int slaveAddress, int subAddress, int data){
    Wire.beginTransmission(slaveAddress);
    Serial.print("Transmission begin...");
    Serial.println("");
    Wire.write(subAddress);
    Serial.printf("writing - %d subAddress", subAddress);
    Serial.println("");
    Wire.write(data);
    Serial.printf("writing - %d data", data);
    Serial.println("");
    result = Wire.endTransmission(true);
    if(!result){
        Serial.print("Transmission successful!");
        Serial.println("");
    }
    else{
        Serial.print("Transmission failed.");
        Serial.println("");
        Serial.printf("As Wire.endTransmission returns a non-zero value i.e., %d", result);
        Serial.println("");
    }
    delay(200);
}

/*========== Perform Accelerometer, Temperature, and Gyroscope data read from MPU-6050 ==========*/
int sensorRead(int slaveAddress, float *accelData, float &temp, float *gyroData){
    Wire.beginTransmission(slaveAddress);           // Address desired slave device to read from                
    Wire.write(ACCEL_REG_ADDR);                     // Indicate target address for reading in data
    Wire.endTransmission(false);                    // Indicate we will send a restart signal (next line) to read in data
    Wire.requestFrom(slaveAddress, 14, true);       // request 14 bytes of data

    // Read in MSB, bitshift by eight, switch positions with LSB, and store in temporary variable
    int xAccRaw = (int) Wire.read()<<8 | Wire.read();  
    int yAccRaw = (int) Wire.read()<<8 | Wire.read();
    int zAccRaw = (int) Wire.read()<<8 | Wire.read();
    int tempRaw = (int) Wire.read()<<8 | Wire.read();
    int xGyrRaw = (int) Wire.read()<<8 | Wire.read();
    int yGyrRaw = (int) Wire.read()<<8 | Wire.read();
    int zGyrRaw = (int) Wire.read()<<8 | Wire.read();

    result = Wire.endTransmission(true);

    // Determine if raw value is intended to be read as positive or negative
    // Accelerometer
    if(xAccRaw < 32768){
        xAccRaw = -xAccRaw;
    }
    else{
        xAccRaw = -(xAccRaw - 65536);
    }
    if(yAccRaw < 32768){
        yAccRaw = -yAccRaw;
    }
    else{
        yAccRaw = -(yAccRaw - 65536);
    }
    if(zAccRaw < 32768){
        zAccRaw = -zAccRaw;
    }
    else{
        zAccRaw = -(zAccRaw - 65536);
    }

    // Gyroscope
    if(xGyrRaw < 32768){
        xGyrRaw = -xGyrRaw;
    }
    else{
        xGyrRaw = -(xGyrRaw - 65536);
    }
    if(yGyrRaw < 32768){
        yGyrRaw = -yGyrRaw;
    }
    else{
        yGyrRaw = -(yGyrRaw - 65536);
    }
    if(zGyrRaw < 32768){
        zGyrRaw = -zGyrRaw;
    }
    else{
        zGyrRaw = -(zGyrRaw - 65536);
    }
    
    xGyrRaw = -xGyrRaw;     // Needed to align accelerometer and gyroscope axes (x originally inverted)
    
    // Convert raw data into real-world value, cast as a float, and store in external variable
    accelData[0] = ((float) xAccRaw)/accelSensitivity2g;    
    accelData[1] = ((float) yAccRaw)/accelSensitivity2g;
    accelData[2] = ((float) zAccRaw)/accelSensitivity2g;
    temp = ((float) tempRaw)/tempSensitivityUntrimmed;
    temp = temp + tempOffset;
    gyroData[0] = ((float) xGyrRaw)/gyroSensitivity250;
    gyroData[1] = ((float) yGyrRaw)/gyroSensitivity250;
    gyroData[2] = ((float) zGyrRaw)/gyroSensitivity250;

    // Normalize and Calibrate Data
    accelMagnitude = sqrt(sq(accelData[0]) + sq(accelData[1]) + sq(accelData[2]));
    for( int i = 0; i < 3; i++){
        accelData[i] = accelData[i]/accelMagnitude;             // Normalize accelerometer data
        accelData[i] = accelData[i] + accelCalibration_1[i];    // Calibrate accelerometer data with offset
        gyroData[i] = gyroData[i] + gyroCalibration_1[i];     // Calibrate gyroscope data with offset
    }
    return result;
}

/*========== Filter accelerometer data using gyroscope data ==========*/
void  filterData(float *accelData, float *gyroData, int &startup){
    // Store new sensor values in buffers
    for(int i = 0; i < 3; i++){
            accelBuffer[i][y] = accelData[i];   // Store newest accelData values in first line of buffer
            gyroBuffer[i][y] = gyroData[i];     // Store newest gyroData values in first line of buffer
        }
   
    // If this is the first time running the filter, there is no old data, so our best estimate is based on our current data
    if(startup == 0){
        // Store new accelBuffer values in estimate
        for(int i = 0; i < 3; i++){
            est[i] = accelBuffer[i][y];  // Use current accelerometer data to get current estimate
        }
        y++;            // Increment row in buffers
        startup = 1;    // Set to 1 so this section is not repeated
    }
    else{
        // Take the average of accelBuffer and gyroBuffer
        // Sum all values in each
        for(int i = 0; i < num; i++){
            sumAccel += accelBuffer[i][y];
            sumGyro += gyroBuffer[i][y];
        }
        avgAccel = sumAccel/num;
        avgGyro = sumGyro/num;

        y++;    // Increment row in buffers
        
        // If we reach the end of the buffers, reset to 0
        if(x >= num){
            x = 0;
        }
    }
}

/*========== Toggle the LED on or off depending on the given command ==========*/
int ledToggle(String command) {
    if (command == "on") {
        digitalWrite(led, HIGH);
        return 1;
    }
    else if (command == "off") {
        digitalWrite(led, LOW);
        return 0;
    }
    else {
        return -1;
    }
}

#endif // _PERIPHERALFUNCTIONS_H