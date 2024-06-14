/**
 * Test sketch for the Iridium modem w/ RemoteLogger library v0.2
 * Author: Rachel Pagdin
 * June 12, 2024
 */

#include <RemoteLogger.h>

String header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";      // header for CSV file
String message = "this is a test ";

RemoteLogger logger(header);        // custom library instance

void setup(){
    delay(500);

    Serial.begin(9600);

    logger.begin();     // start up the logger (set pins etc)

    delay(500);

    test_irid();
}

void loop(){

}

void test_irid(){
    message = message + logger.rtc.now().timestamp();       // add the current time to the message

    logger.irid_test(message);
}