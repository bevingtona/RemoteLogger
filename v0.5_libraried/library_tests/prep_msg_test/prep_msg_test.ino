/**
 * Prep message from prepopulated hourly data file (must match format of provided header)
 * Author: Rachel Pagdin
 * June 13, 2024
 */

#include <RemoteLogger.h>

String header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";

RemoteLogger logger(header);

void setup(){
    delay(500);
    Serial.begin(115200);
    while(!Serial);

    Serial.println("starting...");
    logger.begin();
}

void loop(){
    Serial.println(F("preparing message from hourly data..."));
    String msg = logger.prep_msg();

    Serial.print(F("message: ")); Serial.println(msg);
}