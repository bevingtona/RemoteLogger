
/**
* this would be good for an example code 

quiet/verbose mode (print or not)
data pin flexible
tests for examples

priorities:
take measurement (different for each sensor combo) --> how to manage?
- relay different for each 
- concatenating measurements 
- may not be necessary 
generalize take_measurement and prep_msg in lib

*/

#include "RemoteLogger.h"

String my_letter = "ABC"; //depends on sensors (what we're measuring) - order matters - see docs for header building
String my_header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";

// set these values based on the pins attached for the relay 
byte HYDROS_SET_PIN = 5;
byte HYDROS_UNSET_PIN = 6;

RemoteLogger rl(my_letter, my_header);

void setup() {
  // put your setup code here, to run once:
  pinMode(rl.ledPin, OUTPUT);

  rl.start_data_bus(); //start the SDI-12 data bus
}



void loop() {
  // put your main code here, to run repeatedly:
  rl.blinky(5, 100, 100, 300); //blinky test

  // take sample from hydros, print to console

  //set power to hydros21 (relay)
  digitalWrite(HYDROS_SET_PIN, HIGH); delay(50);
  digitalWrite(HYDROS_SET_PIN, LOW); delay(1000);

  String hydros_response = rl.sample_hydros_M();
  Serial.println(hydros_response);

  //cut power to hydros21 (relay)
  digitalWrite(HYDROS_UNSET_PIN, HIGH); delay(50);
  digitalWrite(HYDROS_UNSET_PIN, LOW); delay(50);

}
