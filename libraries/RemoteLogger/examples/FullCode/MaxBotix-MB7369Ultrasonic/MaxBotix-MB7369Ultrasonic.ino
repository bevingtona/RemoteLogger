/**
 * sample from MaxBotix MB7369 ultrasonic ranger
 * https://maxbotix.com/pages/hrxl-maxsonar-wr-datasheet
 * send messages with snow level every 4 hours
 * designed for use with TPL nano timer set to 15 minutes
 * 
 * Author: Rachel Pagdin
 * June 19, 2024
 */

#include <RemoteLogger.h>

const int ultrasonicPowerPin = 11;          // power for MaxBotix
const int triggerPin = 10;                  // trigger to start sending ultrasonic
const int pulseInputPin = 12;               // input pin to read pulse (must be PWM compatible -- all pins but A1, A5 on Feather M0 Adalogger)

String header = "datetime,batt_v,memory,snow_depth_mm";
const byte num_params = 1;
float multipliers[num_params] = {1};
String letters = "E";           // snow depth -- A for water level

RemoteLogger logger(header, num_params, multipliers, letters);

String take_measurement(){
    String msmt = String(logger.sample_batt_v()) + "," +
        logger.sample_memory() + "," + 
        logger.sample_ultrasonic(ultrasonicPowerPin, triggerPin, pulseInputPin);
    
    return msmt;
}

void setup(){
    delay(50);

    logger.begin();
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