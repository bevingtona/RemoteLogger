/**
 * change utlity pins from default for Feather M0 Adalogger
 * pin setting is available through public setter methods
 * check docs for default pin settings
 * 
 * Warning: some pin functions have firmware requirements -- check requirements before changing pins
 * 
 * Pins for sensors are externally set by the user and have no defaults
 * 
 * Author: Rachel Pagdin
 * June 17, 2024
 */

#include <RemoteLogger.h>

const byte led = 13; 

RemoteLogger logger{};

void setup(){
    // set pin with predefined variable
    logger.setLedPin(led);

    // set pin with literal
    logger.setIridSlpPin(A2);
    logger.setSDSelectPin(10);

    // change any pins you want to change before calling begin()
    logger.begin();
}