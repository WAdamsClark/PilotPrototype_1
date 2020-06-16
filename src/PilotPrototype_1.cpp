/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/adams/OneDrive/Documents/GitHub/ParticleWorkbench/PilotPrototype_1/src/PilotPrototype_1.ino"
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
#include "PeripheralFunctions.h"  // Holds all peripheral functions for reading in data from MPU-6050, etc.

/*========== Device System Settings ==========*/
//SYSTEM_MODE(SEMI_AUTOMATIC);  // Uncomment this line when attempting to test locally 
void setup();
void loop();
#line 22 "c:/Users/adams/OneDrive/Documents/GitHub/ParticleWorkbench/PilotPrototype_1/src/PilotPrototype_1.ino"
SYSTEM_MODE(AUTOMATIC);       // Comment out this line when attempting to test locally
SystemSleepConfiguration config;    // Create instantiation of sleep configuration for use when making calls to sleep

/*========== Particle Data Variables ==========*/
// MPU-6050 strings for Particle Cloud Events
char angleStr1[50];
char angleStr2[50];
// Strain gauge strings for Particle Cloud events
char sgLi1[5];
char sgLi2[5];
char sgLo1[5];
char sgRi1[5];
char sgRi2[5];
char sgRo1[5];

/*========== Strain Gauge Data Variables ==========*/
// Buffers to hold previous strain gauge readings
int sgBuffLi1[sgNum];   
int sgBuffLi2[sgNum];
int sgBuffLo1[sgNum];
int sgBuffRi1[sgNum];
int sgBuffRi2[sgNum];
int sgBuffRo1[sgNum];
// Output values from filtering strain gauge readings
int sgValLi1 = 0;
int sgValLi2 = 0;
int sgValLo1 = 0;
int sgValRi1 = 0;
int sgValRi2 = 0;
int sgValRo1 = 0;
// Reference values for position in strain gauge buffers
int sgXLi1 = 0;
int sgXLi2 = 0;
int sgXLo1 = 0;
int sgXRi1 = 0;
int sgXRi2 = 0;
int sgXRo1 = 0;

/*========== MPU-6050 Data Variables ==========*/
// Angles about each axis determined from accelerometer
float angles1[3];
float angles2[3];
// Vector for raw accelerometer data
float accelData1[3];
float accelData2[3];
// Vector for raw gyroscope data
float gyroData1[3];
float gyroData2[3];
// Variable for temperature data [deg C]
float temp_C1 = 0;
float temp_C2 = 0;
// Buffer to hold previous accelerometer measurements with [x][y], and x holding values for each axis
float accelBuffer1[3][accNum];
float accelBuffer2[3][accNum];
// Buffer to hold previous gyroscope measurements with [x][y], and x holds values for each axis
float gyroBuffer1[3][accNum];
float gyroBuffer2[3][accNum];
// Index variable to keep track of [y] place in accelBuffer and gyroBuffer
int x1 = 0;
int x2 = 0;
// State variable to determine if first time running through filter
int startup1 = 0;
int startup2 = 0;

/*========== Setup ==========*/
void setup() {
  Serial.begin(9600); // Initiate serial communication at 9600 BAUD
  Wire.begin();       // Initiate Wire library
  delay(100);         // delay 100 milliseconds for startup
  
  // Configure MPU-6050's with default settings
  configSensor(MPU_SLAVE_ADDR_1, PWR_MGMT_1, NRML_PWR);
  configSensor(MPU_SLAVE_ADDR_2, PWR_MGMT_1, NRML_PWR);

  // Set up GPIO
  pinMode(Power, OUTPUT);   // Power pin for strain gauges
  // Note: reading from analog pins does not require pinMode()
  digitalWrite(Power, HIGH);
  
  // Particle Cloud Variables, Functions, and Publishing  
  // Update strain gauge variables
  sprintf(sgLi1, "%d", sgValLi1);
  sprintf(sgLi2, "%d", sgValLi2);
  sprintf(sgLo1, "%d", sgValLo1);
  sprintf(sgRi1, "%d", sgValRi1);
  sprintf(sgRi2, "%d", sgValRi2);
  sprintf(sgRo1, "%d", sgValRo1);
  Particle.publish("sgLi1", sgLi1, PRIVATE);
  Particle.publish("sgLi2", sgLi2, PRIVATE);
  Particle.publish("sgLo1", sgLo1, PRIVATE);
  Particle.publish("sgRi1", sgRi1, PRIVATE);
  Particle.publish("sgRi2", sgRi2, PRIVATE);
  Particle.publish("sgRo1", sgRo1, PRIVATE); 
  Particle.variable("sgLi1", sgValLi1); 
  Particle.variable("sgLi2", sgValLi2); 
  Particle.variable("sgLo1", sgValLo1); 
  Particle.variable("sgRi1", sgValRi1); 
  Particle.variable("sgRi2", sgValRi2); 
  Particle.variable("sgRo1", sgValRo1); 

  // Update angle variables
  sprintf(angleStr1, "%f, %f, %f", angles1[0], angles1[1], angles1[2]);
  sprintf(angleStr2, "%f, %f, %f", angles2[0], angles2[1], angles2[2]);
  Particle.variable("angles1", angleStr1);
  Particle.variable("angles2", angleStr2);
  Particle.publish("angleStr1", angleStr1, PRIVATE);
  Particle.publish("angleStr2", angleStr2, PRIVATE);
}

/*========== Main Loop ===========*/
void loop(){
  // Get 20 readings at a time; average and filter data
  for(int i = 0; i < 20; i++){
    // Analog read and conversion
    strainGaugeRead(sgPinLi1, sgBuffLi1, sgXLi1, sgValLi1);
    strainGaugeRead(sgPinLi2, sgBuffLi2, sgXLi2, sgValLi2);
    strainGaugeRead(sgPinLo1, sgBuffLo1, sgXLo1, sgValLo1);
    strainGaugeRead(sgPinRi1, sgBuffRi1, sgXRi1, sgValRi1);
    strainGaugeRead(sgPinRi2, sgBuffRi2, sgXRi2, sgValRi2);
    strainGaugeRead(sgPinRo1, sgBuffRo1, sgXRo1, sgValRo1);  
    
    // Read in data from MPU-6050's
    result = sensorRead(MPU_SLAVE_ADDR_1, accelData1, temp_C1, gyroData1);
    result = sensorRead(MPU_SLAVE_ADDR_2, accelData2, temp_C2, gyroData2);

    // Filter data and determine global angular orientation
    filterData(accelData1, accelBuffer1, gyroData1, gyroBuffer2, x1, startup1, angles1); 
    filterData(accelData2, accelBuffer2, gyroData2, gyroBuffer2, x2, startup2, angles2);
  }

  // Particle Event Publishing
  // Update strain gauge variables
  sprintf(sgLi1, "%d", sgValLi1);
  sprintf(sgLi2, "%d", sgValLi2);
  sprintf(sgLo1, "%d", sgValLo1);
  sprintf(sgRi1, "%d", sgValRi1);
  sprintf(sgRi2, "%d", sgValRi2);
  sprintf(sgRo1, "%d", sgValRo1);
  Particle.publish("sgLi1", sgLi1, PRIVATE);
  Particle.publish("sgLi2", sgLi2, PRIVATE);
  Particle.publish("sgLo1", sgLo1, PRIVATE);
  Particle.publish("sgRi1", sgRi1, PRIVATE);
  Particle.publish("sgRi2", sgRi2, PRIVATE);
  Particle.publish("sgRo1", sgRo1, PRIVATE);
  
  // Update angle variables
  sprintf(angleStr1, "%f, %f, %f", angles1[0], angles1[1], angles1[2]);
  sprintf(angleStr2, "%f, %f, %f", angles2[0], angles2[1], angles2[2]);
  Particle.publish("angleStr1", angleStr1, PRIVATE);
  Particle.publish("angleStr2", angleStr2, PRIVATE); 

  delay(timestep);  // Wait ten seconds

  /*
  // Sleep config requires pin+time - D3 (with nothing connected) and rising edge, for 5 minutes (300s) with SLEEP_NETWORK_STANDBY
  config.mode(SystemSleepMode::STOP)
        .gpio(D3, RISING)
        .duration(300s)
        .network(NETWORK_INTERFACE_CELLULAR);
  SystemSleepResult result = System.sleep(config);
  */
}