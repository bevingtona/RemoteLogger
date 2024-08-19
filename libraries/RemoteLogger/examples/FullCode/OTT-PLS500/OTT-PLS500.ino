/**
 * sample from OTT-PLS 500 (SDI-12)
 * https://www.ott.com/products/water-level-1/ott-pls-500-pressure-level-sensor-2494/
 * send messages with water level and temp every 4 hours
 * designed for use with TPL nano timer set to 15 minutes
 * 
 * Author: Rachel Pagdin
 * June 19, 2024
 */

#include <RemoteLogger.h>

const int dataPin = 12;             // pin for SDI-12 data bus on OTT-PLS
const int sensorAddress = 0;        // address for OTT-PLS on SDI-12    
SDI12 mySDI12(dataPin);             // data bus object

String header = "datetime,batt_v,memory,water_level_mm,water_temp_c,ott_status,ott_rh,ott_dew,ott_deg";
const byte num_params = 6;
float multipliers[num_params] = {1000, 10, 0, 0, 0, 0};             // multiplier of 0 indicates don't include in message
String letters = "AB";          // letters for message corresponding to water level and temp

RemoteLogger logger(header, num_params, multipliers, letters);

String take_measurement(){
    String msmt = String(logger.sample_batt_v()) + "," +
        logger.sample_memory() + "," +
        logger.sample_ott(mySDI12, sensorAddress);

    return msmt;
}

void setup(){
    // if you want to change any pins from the library defaults do it here (see docs for defaults)
    delay(50);

    logger.begin();     // start up the logger
    mySDI12.begin();    // start up the data bus for the HYDROS    
}

void loop(){
    delay(100);

    DateTime presentTime = logger.rtc.now();

    String sample = take_measurement();     // take measurement

    logger.write_to_csv(header, presentTime.timestamp() + "," + sample, "/DATA.csv");
    logger.increment_samples();         // number of samples since last write to hourly

    int samplesSinceHourly = logger.num_samples();      // number of samples since last write to hourly
    if (samplesSinceHourly == 4){        // it's been an hour --> write to hourly
        logger.write_to_csv(header, presentTime.timestamp() + "," + sample, "/HOURLY.csv");
        logger.reset_sample_counter();          // reset the since-hourly counter

        int hourlySamples = logger.num_hours();     // number of hours since a message was sent
        if (hourlySamples >= 4 & hourlySamples < 10) {          // more than 4 hours --> send a message
            String msg = logger.prep_msg();        // prepare the message to send

            int iridErr = logger.send_msg(msg);     // attempt to send the message
            if (iridErr == 0) {         // send successful
                logger.reset_hourly();
            }       
        } else if (samplesSinceHourly >= 10) {      // too much data to send --> delete hourly file
            logger.reset_hourly();
        }
    } else if (samplesSinceHourly > 4) {        // something went wrong --> reset counters
        logger.reset_sample_counter();
        logger.reset_hourly();
    }

    logger.tpl_done();      // trigger done pin on TPL --> cut power
}