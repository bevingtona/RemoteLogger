/**
 * sample from Hydros21 
 * https://metergroup.com/products/hydros-21/ 
 * send messages with water level, water temp, electrical conductivity every 4 hours
 * designed for use with TPL nano timer set to 15 minutes
 * 
 * Author: Rachel Pagdin
 * June 17, 2024
*/

#include <RemoteLogger.h>

const int dataPin = 12;             //pin for SDI-12 data bus on Hydros (can attach to any digital pin)
const int sensorAddress = 0;        //address for Hydros on SDI-12 (factory default is 0)
SDI12 mySDI12(dataPin);             //data bus object

String header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";      // header for CSV file
const byte num_params = 3;        // number of sampled parameters 
float multipliers[num_params] = {1, 10, 1};         // multipliers for parameters (in order) to remove decimals for messages
String letters = "ABC";         // letters for start of message, correspond to sampled parameters

RemoteLogger logger(header, num_params, multipliers, letters);        // custom library instance

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

void loop(void){
    delay(100);

    DateTime presentTime = logger.rtc.now();    // wake up, check time

    // take measurement
    String sample = take_measurement();
    
    // write to DATA.csv
    logger.write_to_csv(header, presentTime.timestamp() + "," + sample, "/DATA.csv");

    // increment sample tracker (since last write to hourly)
    logger.increment_samples();

    // determine whether or not to write to hourly
    int samplesSinceHourly = logger.num_samples();
    if (samplesSinceHourly == 4) {    // it's been an hour --> write to hourly
        logger.write_to_csv(header, presentTime.timestamp() + "," + sample, "/HOURLY.csv");
        logger.reset_sample_counter();      // reset the tracker - wrote to hourly

        // determine whether or not to send a message
        int hourlySamples = logger.num_hours();
        if (hourlySamples >= 4 && hourlySamples < 10) {   // more than 4 hours -- send message
            String msg = logger.prep_msg();
            int iridErr = logger.send_msg(msg);     

            if (iridErr == 0) {     // successful send -- reset counters

                logger.reset_sample_counter();
                logger.reset_hourly();
            }
        } else if (hourlySamples >= 10) {  // more than 10 hours of data -- too big, delete
            logger.reset_sample_counter();
            logger.reset_hourly();
        }

    } else if (samplesSinceHourly > 4) {    // something went wrong -- reset counters
        logger.reset_sample_counter();
        logger.reset_hourly();
    }

    // for visual done: 
    logger.blinky(3, 500, 500, 1000);   // blink 3 times to simulate tpl done

    // trigger done pin on TPL
    logger.tpl_done();
}