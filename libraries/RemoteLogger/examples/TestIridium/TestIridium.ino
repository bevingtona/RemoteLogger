/**
 * Test Iridium connection with output to Serial
 * Iridium test includes firmware version check, signal quality check, message send attempt, sync clock
 * Warning: this will attempt to send a message, which will use credits if successful
 * 
 * Author: Rachel Pagdin
 * June 17, 2024
 */

#include <RemoteLogger.h>       

String message = "this is a test ";

RemoteLogger logger{};

void setup(){
    delay(50);

    // start Serial to see feedback from irid_test
    Serial.begin(115200);
    logger.begin(); 

    delay(50);

    // build a message
    message = message + logger.rtc.now().timestamp();

    // test the Iridium modem 
    logger.irid_test(message);
}