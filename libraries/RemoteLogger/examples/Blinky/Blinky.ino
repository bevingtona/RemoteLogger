/**
 * blinks the built-in LED on Feather M0 Adalogger
 * 
 * Author: Rachel Pagdin
 * June 17, 2024
 */

#include <RemoteLogger.h>

RemoteLogger logger{};

void setup(){
    logger.begin();
}

void loop(){
    // blink built-in LED 3 times, 500ms on, 200ms off, 1s between
    logger.blinky(3, 500, 200, 1000);
}