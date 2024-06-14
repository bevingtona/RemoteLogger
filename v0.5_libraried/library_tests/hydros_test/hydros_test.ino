/**
 * Test sampling HYDROS21 sensor w/ RemoteLogger library v0.2
 * Author: Rachel Pagdin
 * June 12, 2024
 */

#include <RemoteLogger.h>

const int dataPin = 12;            //pin for SDI-12 data bus on Hydros
const int sensorAddress = 0;        //address for Hydros on SDI-12
SDI12 mySDI12(dataPin);             //data bus object

String header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";
// String sample;
// String msmt;
int ctr = 0;

RemoteLogger logger(header);

// String take_measurement(){
//     msmt = logger.sample_hydros_M(mySDI12, sensorAddress);
// }

void setup(){
  Serial.begin(9600);
  delay(50);

  Serial.println("starting up...");
  logger.begin();
  mySDI12.begin();
  delay(500);
}

void loop(){
  logger.blinky(3, 500, 500, 500);

  if(ctr % 5 == 0){
    Serial.println(logger.sample_hydros_M(mySDI12, sensorAddress));
  }
  else{ Serial.println(ctr); }
  ctr++;
  //Serial.println(sample);
  delay(2000);
}