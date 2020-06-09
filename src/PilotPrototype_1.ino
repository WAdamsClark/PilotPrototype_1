/*
 * Project PilotPrototype_1
 * Description: Particle App for reading data from accelerometer, strain gauge sensors and pushing to Particle Cloud
 * Author: Adams Clark, Thomas Spencer
 * Date: 6/9/2020
 */
// Thomas Is TESTING HERE

int led = D7;   // LED pin on Boron development board
int sgPin = A0; // Strain Gauge pin for analogRead
int sgRawValue = 0; // Raw value obtained by performing analogRead(sgPin)
char weightStr[30]; // String to which sgRawValue is cast for use when pushing events to Particle Cloud
float sensCoeff = 2.1;  // Coefficient for converting sgRawValue to real-world weight value (from datasheet)

void setup() {
  Serial.begin(9600); // Initiate serial communication at 9600 BAUD

  // Note: reading from analog pins does not require pinMode()
  pinMode(led, OUTPUT);   // set the ledPin as output
  
  Particle.variable("containerWeight", sgRawValue); // Declare Particle.variable to access value from the cloud
  sprintf(weightStr, "%d", sgRawValue);
  //Serial.println(containerWeight);
  
  Particle.variable("StringWeight", weightStr);
  Particle.publish("dumpster-loading", weightStr, PRIVATE);
  
  Particle.function("led", ledToggle);
}

void loop() {
  sgRawValue = analogRead(sgPin);  // read the analogPin
  
  // AnalogRead ranges from 0-4095, so we must convert to a voltage between 0-3.3 and multiply by sensitivity coefficient
  analogvalue = 2.1*((analogvalue/4095)*3.3);
  
  // Update string variables
  sprintf(weightStr, "%d", analogvalue);
  
  // Publish data as event to Particle Cloud
  Particle.publish("dumpster-loading", weightStr, PRIVATE);   
  
  // Print data to the serial monitor
  Serial.printlnf("testing");

  delay(5000);
}

/*========== ledToggle toggles the LED at on or off depending on the given command ==========*/
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
