/**
 * Hydros proof of concept for library v0.2
 * Author: Rachel Pagdin
 * June 6, 2024
*/

#include <RemoteLogger.h>

const int dataPin = 12;            //pin for SDI-12 data bus on Hydros
const int sensorAddress = 0;        //address for Hydros on SDI-12
SDI12 mySDI12(dataPin);             //data bus object

String header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";

RemoteLogger logger(header);

String take_measurement(){
    String msmt = String(logger.sample_batt_v()) + "," +
        logger.sample_memory() + "," +
        logger.sample_hydros_M(mySDI12, sensorAddress);

    return msmt;
}

void setup(void){
    // if you want to change any pins from the preset do it here (see docs for preset)
    delay(500); //just in case

    logger.begin();     // start up the logger
    mySDI12.begin();    // start up data bus (user is reponsible for external sensors)

    /** TODO: test mode would go here */

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

        // determine whether or not to send a message
        int hourlySamples = logger.num_hours();
        if (hourlySamples >= 4 & hourlySamples < 10) {   // more than 4 hours -- send message
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

    // trigger done pin on TPL
    logger.tpl_done();

    delay(50);
}