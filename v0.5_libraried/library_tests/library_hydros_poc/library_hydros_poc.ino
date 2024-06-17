/**
 * Hydros proof of concept for library v0.2 without string parsing
 * Author: Rachel Pagdin
 * June 17, 2024
*/

#include <RemoteLogger.h>

const int dataPin = 12;            //pin for SDI-12 data bus on Hydros
const int sensorAddress = 0;        //address for Hydros on SDI-12
SDI12 mySDI12(dataPin);             //data bus object

String header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";      // header for CSV file
const byte num_params = 3;        // number of sampled parameters 
float multipliers[num_params] = {1, 10, 1};         // multipliers for parameters (in order) to remove decimals
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
    // Serial.begin(115200);
    // while(!Serial);
    // delay(500); //just in case

    // Serial.println("starting...");  /** TODO: remove */
    logger.begin();     // start up the logger
    mySDI12.begin();    // start up data bus for hydros (user is reponsible for external sensors)

    /** TODO: test mode would go here */

}

void loop(void){
    delay(100);

    DateTime presentTime = logger.rtc.now();    // wake up, check time
    // Serial.print("present time: "); Serial.println(presentTime.timestamp());    /** TODO: remove */
    //logger.blinky(1,500,0,0);

    // take measurement
    // Serial.println("sampling...");  /** TODO: remove */
    String sample = take_measurement();
    //logger.blinky(1,500,0,0);
    
    // write to DATA.csv
    // Serial.println("writing to data file...");      /** TODO: remove */
    logger.write_to_csv(header, presentTime.timestamp() + "," + sample, "/DATA.csv");
    //logger.blinky(1,500,0,0);

    // increment sample tracker (since last write to hourly)
    logger.increment_samples();
    //logger.blinky(1,500,0,0);

    // determine whether or not to write to hourly
    int samplesSinceHourly = logger.num_samples();
    // Serial.print("samples since hourly write: "); Serial.println(samplesSinceHourly);   /** TODO: remove */
    if (samplesSinceHourly == 4) {    // it's been an hour --> write to hourly
        // Serial.println("writing to hourly...");     /** TODO: remove */
        //logger.blinky(2, 500, 200, 200);        // fast blink 2 times, writing to hourly
        logger.write_to_csv(header, presentTime.timestamp() + "," + sample, "/HOURLY.csv");
        logger.reset_sample_counter();      // reset the tracker - wrote to hourly

        // determine whether or not to send a message
        int hourlySamples = logger.num_hours();
        // Serial.print("hourly samples since send: "); Serial.println(hourlySamples);     /** TODO: remove */
        if (hourlySamples >= 4 & hourlySamples < 10) {   // more than 4 hours -- send message
            // Serial.println("preparing message...");     /** TODO: remove */
            //logger.blinky(6, 500, 200, 200);        // fast blink 6 times, sending a message
            String msg = logger.prep_msg();
            // Serial.println("attempting to send message...");        /** TODO: remove */
            int iridErr = logger.send_msg(msg);     

            if (iridErr == 0) {     // successful send -- reset counters
                // Serial.println("send successful!");     /** TODO: remove */
                logger.reset_sample_counter();
                logger.reset_hourly();
            }
            // else{
            //     Serial.println("couldn't send :(");     /** TODO: remove */
            // }

        } else if (hourlySamples >= 10) {  // more than 10 hours of data -- too big, delete
            logger.reset_sample_counter();
            logger.reset_hourly();
        }

    } else if (samplesSinceHourly > 4) {    // something went wrong -- reset counters
        logger.reset_sample_counter();
        logger.reset_hourly();
    }

    // Serial.println("cycle done"); Serial.println("---------------------------");        /** TODO: remove */

    // for testing: 
    logger.blinky(3, 500, 500, 1000);   // blink 3 times to simulate tpl done

    // trigger done pin on TPL
    logger.tpl_done();

    delay(10000);
}