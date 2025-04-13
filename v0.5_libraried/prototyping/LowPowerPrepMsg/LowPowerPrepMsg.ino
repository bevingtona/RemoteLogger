/**
* test low power mode prep message
* Author: Rachel Pagdin
* August 23, 2024
*/

#include <RemoteLogger.h>
#include <SD.h>

#define SD_CS 4

String header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";      // header for CSV file
const byte num_params = 3;        // number of sampled parameters 
float multipliers[num_params] = {1, 10, 1};         // multipliers for parameters (in order) to remove decimals for messages
String letters = "ABC";         // letters for start of message, correspond to sampled parameters

RemoteLogger logger(header, num_params, multipliers, letters);

void setup() {
  // put your setup code here, to run once:
  delay(50);
  Serial.begin(115200);
  while(!Serial);

  while(!SD.begin(SD_CS)){
    Serial.println(F("SD card error"));
  }
  Serial.println(F("SD card start successful!"));

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(F("enter sim battery voltage"));
  while(Serial.available() == 0);   // wait for user input
  float batt = Serial.parseFloat();
  Serial.print(F("you entered battery voltage: ")); Serial.print(batt); Serial.println(F(" V"));

  if (batt >= 4){   // regular operation 
    Serial.println(F("normal operation - preparing message"));
    String msg = logger.prep_msg();
    Serial.print(F("message: ")); Serial.println(msg);
  } else {
    Serial.println(F("low power mode - preparing message"));
    String msg = logger.low_pwr_prep_msg();
    Serial.print(F("message: ")); Serial.println(msg); 
  }
}
