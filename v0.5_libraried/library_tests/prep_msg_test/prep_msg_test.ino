/**
 * Prep message from prepopulated hourly data file (must match format of provided header)
 * Author: Rachel Pagdin
 * June 13, 2024
 */

#include <RemoteLogger.h>

String header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";
const byte num_params = 3;        // number of sampled parameters 
float multipliers[num_params] = {1, 10, 1};         // multipliers for parameters (in order) to remove decimals
String letters = "ABC";         // letters for start of message, correspond to sampled parameters

RemoteLogger logger(header, multipliers, num_params, letters);

void setup(){
    delay(500);
    Serial.begin(115200);
    while(!Serial);

    Serial.println("starting...");
    logger.begin();

    Serial.println(F("preparing message from hourly data..."));
    String msg = logger.prep_msg();

    Serial.print(F("message: ")); Serial.println(msg);
}

void loop(){
    // Serial.println(F("preparing message from hourly data..."));
    // String msg = logger.prep_msg();

    // Serial.print(F("message: ")); Serial.println(msg);

}