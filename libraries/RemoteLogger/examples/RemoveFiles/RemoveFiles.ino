/**
 * remove basic files from SD card: data file, hourly file, and tracking file
 * Warning: will delete the files - make sure you have saved what you want externally
 * 
 * Author: Rachel Pagdin
 * June 17, 2024
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