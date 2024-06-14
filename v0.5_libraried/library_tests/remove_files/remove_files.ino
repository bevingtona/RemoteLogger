/**
 * Sketch to remove all datalogging and tracking files on SD cards (wipe for testing)
 * Author: Rachel Pagdin
 * June 13, 2024
 */

#include <RemoteLogger.h>

RemoteLogger logger{};

void setup(){
    logger.begin();

    logger.wipe_files();
}

void loop(){
    logger.blinky(3,200,200, 1000);
}