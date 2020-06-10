/*
 * Project PilotPrototype_1
 * Description: Particle App for reading data from accelerometer, strain gauge sensors and pushing to Particle Cloud
 * Author: Adams Clark, Thomas Spencer
 * Date: 6/9/2020
 * 
 * 
 * Notes: 
 * In the future, we will likely want to have some calibration code that we can run on a bunch of IMU devices at once to get
 * offsets for accelerometer and gyroscope readings that can then be stored in an external text file and referenced by each 
 * device while in the field. This will alow us to not have to push firmware that is individually written with hard-coded calibration
 * values, instead having each device run the same firmware which checks the calibration file for that individual device's calibration
 * values. Additionally, this file will likely include geometry data for the container that each individual device is mounted on.
 */

/*========== Header Includes ==========*/
#include "Wire.h"                 // This library allows I2C communication
#include "Math.h"                 // This library allows certain mathematical functions to be used (i.e., sqrt, sq, etc.)
#include "PeripheralFunctions.h"  // Holds all peripheral functions for reading in data from MPU-6050, etc.

/*========== Device System Settings ==========*/
SYSTEM_MODE(SEMI_AUTOMATIC);

/*========== Data Variables ==========*/
int sgRawValue = 0;   // Raw value obtained by performing analogRead(sgPin)
char weightStr[30];     // String to which sgRawValue is cast for use when pushing events to Particle Cloud
float sgWeight = 0;     // Weight value calculated using strain gauge reading
float sensCoeff = 2.1;  // Coefficient for converting sgRawValue to real-world weight value (from datasheet)
float accelData[3];     // Vector for accelerometer raw data
float gyroData[3];      // Vector for gyroscope raw data
float temp_C = 0;       // Variable for temperature data [deg C]


/*========== Setup ==========*/
void setup() {
  Serial.begin(9600); // Initiate serial communication at 9600 BAUD
  Wire.begin();       // Initiate Wire library
  delay(100);         // delay 100 milliseconds for startup
  
  // Configure sensor with default settings
  configSensor(MPU_SLAVE_ADDR_1, PWR_MGMT_1, NRML_PWR);

  // Set up GPIO
  pinMode(led, OUTPUT);   // LED pin as output
  // Note: reading from analog pins does not require pinMode()
  
  // Particle Cloud Variables, Functions, and Publishing
  Particle.variable("containerWeight", sgRawValue); // Declare Particle.variable to access value from the cloud
  sprintf(weightStr, "%d", sgRawValue);
  Particle.variable("StringWeight", weightStr);
  Particle.publish("dumpster-loading", weightStr, PRIVATE);
  Particle.function("led", ledToggle);
}

/*========== Main Loop ===========*/
void loop() {
  // Analog read and conversion
  sgRawValue = analogRead(sgPin);  // read the analogPin
  sgWeight = 2.1*(((float) sgRawValue/4095)*3.3); // AnalogRead ranges from 0-4095, so we must convert to a voltage between 0-3.3 and multiply by sensitivity coefficient
  
  // Particle Event Publishing
  sprintf(weightStr, "%d", sgRawValue);  // Update string variables
  Particle.publish("dumpster-loading", weightStr, PRIVATE); // Publish data as event to Particle Cloud
  
  // Read in data from MPU-6050
  result = sensorRead(MPU_SLAVE_ADDR_1, accelData, temp_C, gyroData);  
/*
  // Print data to the serial monitor
  // Strain gauge
  Serial.printlnf("%d", sgRawValue);

  // Accelerometer
  Serial.printlnf("%f", accelData[0]);
  Serial.printlnf("%f", accelData[1]);
  Serial.printlnf("%f", accelData[2]);
  Serial.println();

  // Gyroscope
  Serial.printlnf("%f", gyroData[0]);
  Serial.printlnf("%f", gyroData[1]);
  Serial.printlnf("%f", gyroData[2]);
  Serial.println();
*/
Serial.printlnf("%f", accelData[2]);
  delay(100);  // Wait one millisecond
}