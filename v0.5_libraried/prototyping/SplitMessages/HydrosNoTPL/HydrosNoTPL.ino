/**
 * sample from Hydros no TPL -- for generating data files for testing
 * write to hourly every power cycle (simulated TPL set to 1 hr)
 * 
 * Author: Rachel Pagdin
 * August 1, 2024
 */


#include <RemoteLogger.h>

const int dataPin = 12;             //pin for SDI-12 data bus on Hydros (can attach to any digital pin)
const int sensorAddress = 0;        //address for Hydros on SDI-12 (factory default is 0)
SDI12 mySDI12(dataPin);             //data bus object

String header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";      // header for CSV file
const byte num_params = 3;        // number of sampled parameters 
float multipliers[num_params] = {1, 10, 1};         // multipliers for parameters (in order) to remove decimals for messages
String letters = "ABC";         // letters for start of message, correspond to sampled parameters

RemoteLogger logger(header, num_params, multipliers, letters); 

String take_measurement(){
    String msmt = String(logger.sample_batt_v()) + "," +
        logger.sample_memory() + "," +
        logger.sample_hydros_M(mySDI12, sensorAddress);

    return msmt;
}

void setup(void){
    // if you want to change any pins from the preset do it here (see docs for preset)
    delay(500);
    
    logger.begin();     // start up the logger
    mySDI12.begin();    // start up data bus for hydros (user is reponsible for external sensors)

}

void loop(){
    delay(100);

    DateTime presentTime = logger.rtc.now();

    String sample = take_measurement();

    logger.write_to_csv(header, presentTime.timestamp() + "," + sample, "/HOURLY.csv");

    logger.blinky(3, 500, 500, 1000);       // show
}