// PeripheralFunctions.h

#ifndef _PERIPHERALFUNCTIONS_H
#define _PERIPHERALFUNCTIONS_H

/*========== Relevant Addresses and Commands ==========*/
const int Power = D2;                   // Power pin for strain gauges since they require consistent excitation and 3v3 is inconsistent
const int MPU_SLAVE_ADDR_1 = 0x68;      // I2C address of MPU-6050 if AD0 pin is set low (if set high, 0x69)
const int MPU_SLAVE_ADDR_2 = 0x69;      // I2C address of MPU-6050 if AD0 pin is set low (if set high, 0x69)
const int PWR_MGMT_1 = 0x6B;            // Power management register address for MPU-6050 wakeup calls
const int NRML_PWR = 0;                 // Send 0 for normal mode on wakeup
const int ACCEL_REG_ADDR = 0x3B;        // I2C address of ACCEL_XOUT_H

/*========== Communication Variables ==========*/
const float timestep = 10000;           // Determines how quickly main loop cycles [ms]
int result = 0;                         // Result to be returned at end of certain functions

/*========== Accelerometer and Gyroscope Sensor Data Variables ==========*/
const float accelSensitivity2g = 16384;         // Sensitivity of the accelerometer at +/-2g [LSB/g]
const float gyroSensitivity250 = 131;           // Resolution of the gyroscope at +/-250deg/s [LSB/deg/s]
const float tempSensitivityUntrimmed = 340;     // Sensitivity of the temperature sensor when untrimmed at 340 [LSB/deg C]
const float tempOffset = 36.53;                 // Offset value needed for calculating temperature in deg C
const float accelCalibration_1[3] = {0.00821783, 0.285622844, 0.160379276};  // Calibration values for accelerometer on slave device 1
const float gyroCalibration_1[3] = {3.396479224, 0.218546759, -1.138894073};    // Calibration values for gyroscope on slave device 1
const int accNum = 20;      // length of buffers used to hold previous measurements
float accelMagnitude = 0;                       // Holds magnitude of accelerometer gravity vector
float estimates[3];         // Vector to hold current angle estimates
float sumAccel[3];          // Sum of all values in accelBuffer
float sumGyro[3];           // Sum of all values in gyroBuffer
float avgAccel[3];          // Average of all values in accelBuffer
float avgGyro[3];           // Average of all values in gyroBuffer
float accelBuffer[3][accNum];   // Holds previous accelerometer readings for use when filtering
float gyroBuffer[3][accNum];    // Holds previous gyroscope readings for use when filtering

/*========== Strain Gauge Data Variables ==========*/
const float sensCoeff = 2.1;  // Coefficient for converting sgRawValue to real-world weight value (from datasheet)
const int sgPinLi1 = A4;        // Strain Gauge pin for analogRead
const int sgPinLi2 = A5;        // Strain Gauge pin for analogRead
const int sgPinLo1 = A3;        // Strain Gauge pin for analogRead
const int sgPinRi1 = A0;        // Strain Gauge pin for analogRead
const int sgPinRi2 = A1;        // Strain Gauge pin for analogRead
const int sgPinRo1 = A2;        // Strain Gauge pin for analogRead
const int sgNum = 20;           // Number of values to be held in strain gauge buffer of previous readings
int sumSG = 0;                  // Sum of all values in each strain gauge buffer

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

    // Calibrate Data
    accelMagnitude = sqrt(sq(accelData[0]) + sq(accelData[1]) + sq(accelData[2]));
    for( int i = 0; i < 3; i++){
        accelData[i] = accelData[i] + accelCalibration_1[i];  // Calibrate accelerometer data with offset
        gyroData[i] = gyroData[i] + gyroCalibration_1[i];     // Calibrate gyroscope data with offset
    }
    return result;
}

/*========== Filter accelerometer data using gyroscope data ==========*/
// When passing 2D arrays to a function, reference: https://www.tutorialspoint.com/Passing-two-dimensional-array-to-a-Cplusplus-function
void  filterData(float *accData, float accelBuffer[][accNum], float *gyrData, float gyroBuffer[][accNum], int &y, int &startup, float *angles){
    // Store new sensor values in buffers
    for(int i = 0; i < 3; i++){
            accelBuffer[i][y] = accData[i];   // Store newest accelData values in first line of buffer
            gyroBuffer[i][y] = gyrData[i];     // Store newest gyroData values in first line of buffer
        }
   
    // If this is the first time running the filter, there is no old data, so our best estimate is based on our current data
    if(startup == 0){
        // Store new accelBuffer values in estimate
        for(int i = 0; i < 3; i++){
            estimates[i] = accelBuffer[i][y];  // Use current accelerometer data to get current estimate
        }
        y++;            // Increment row in buffers
        startup = 1;    // Set to 1 so this section is not repeated
    }
    else{
        // Take the average of each row element in accelBuffer and gyroBuffer
        // Work through x, y, z elements
        for(int i = 0; i < 3; i++){
            // Reset sum buffers
            sumAccel[i] = 0;    
            sumGyro[i] = 0;

            // Sum all values in each column
            for(int x = 0; x < accNum; x++){
            sumAccel[i] += accelBuffer[i][x];
            sumGyro[i] += gyroBuffer[i][x];
            }

            // Now take the average
            avgAccel[i] = sumAccel[i]/((float) accNum);
            avgGyro[i] = sumGyro[i]/((float) accNum);

            // Take a weighted average of accelerometer and gyroscope values to account for drift and random variances
            //estimates[i] = (avgAccel[i]*0.98) + (avgGyro[i]*(timestep/1000))*0.02; 
            estimates[i] = avgAccel[i];
        }

        // Calculate angles based on estimate
        angles[0] = atan2(estimates[1], estimates[2]) * (180/3.14);     // Angle about x-axis
        angles[1] = atan2(estimates[0], estimates[2]) * (180/3.14);     // Angle about y-axis
        angles[2] = atan2(estimates[1], estimates[0]) * (180/3.14);     // Angle about z-axis

        y++;    // Increment row in accelBuffer and gyroBuffer to prepare for next iteration
        
        // If we reach the end of the buffers, reset to 0
        if(y >= accNum){
            y = 0;
        }
    }
}

/*========== Read in and average values from strain gauges ==========*/
void strainGaugeRead(int sgPin, int *sgBuff, int &sgX, int &sgVal){
    sgBuff[sgX] = analogRead(sgPin);    // Place new reading at current index in buffer for averaging
    sumSG = 0;                            // Reset sum value
    // For the entire buffer, iterate through and sum all values
    for(int i = 0; i < sgNum; i++){
        sumSG = sumSG + sgBuff[i];
    }
    sgX = sgX + 1;      // Increment index in buffer for next value saving
    // If we reach the end of the buffer, reset the value to the beginning
    if(sgX >= sgNum){
        sgX = 0;
    }
    sgVal = sumSG/sgNum;  // Divide sum by number of values in buffer to calculate average
}

#endif // _PERIPHERALFUNCTIONS_H