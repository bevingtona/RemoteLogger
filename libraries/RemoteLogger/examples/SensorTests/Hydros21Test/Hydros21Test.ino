/**
 * sample continuously from Hydros21 and display to Serial
 * 
 * Author: Rachel Pagdin
 * June 17, 2024
 */

#include <RemoteLogger.h>

// set up the SDI-12 data bus for the Hydros sensor
const int dataPin = 12;
const int sensorAddress = 0;
SDI12 mySDI12(dataPin);

RemoteLogger logger{};

void setup(){
    delay(50);
    Serial.begin(115200);           // start Serial for display
    delay(50);

    logger.begin();                 // start up the logger
    mySDI12.begin();                // start up the data bus
}

void loop(){
    String sample = logger.sample_hydros_M(mySDI12, sensorAddress);
    Serial.println(sample);

    delay(3000);        // sample every 3 seconds
}