/**
 * data logger made with SWARM modem
 * uses RemoteLogger library
 * logs temperature, relative humidity %, and barometric pressure 
 * 
 * Author: Rachel Pagdin
 * July 17, 2024
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <RemoteLogger.h>

// #define SEALEVELPRESSURE_HPA (1013.25)          // for reading altitude if you want to
#define SD_CS 4
#define LED 8
#define BATT 9

String header = "datetime,battery,memory,temp_c,rh_prct,bar_press_hpa";
const byte num_params = 3;
float multipliers[num_params] = {100,100,100};          // each parameter has two decimal places
String letters = "FGH";                                 // H: barometric pressure

RemoteLogger logger(header, num_params, multipliers, letters);
Adafruit_BME280 bme;            // I2C (can also use SPI - see library examples)

void setup(){
    delay(500);

    Serial.begin(115200);
    while(!Serial);
    Serial.println(F("SWARM with BME280 test"));

    // logger.begin();
    while(!SD.begin(SD_CS)){
        Serial.println(F("can't find SD card"));
        delay(10);
    }
    pinMode(LED, OUTPUT);
    pinMode(BATT, INPUT);

    while(!bme.begin()){
        Serial.println(F("can't find BME280 sensor"));
    }
}


void loop(){
    delay(100);

    DateTime presentTime = DateTime(__DATE__, __TIME__);        // arbitrary date for testing /** TODO: add RTC */

    String sample = take_measurement();
    Serial.print(presentTime.timestamp()); Serial.print(F("  --  ")); Serial.println(sample);

    // write to DATA.csv
    logger.write_to_csv(header, presentTime.timestamp() + "," + sample, "/DATA.csv");
    
    logger.increment_samples();

    // determine whether or not to write to hourly
    int samplesSinceHourly = logger.num_samples();
    if (samplesSinceHourly == 4) {    // it's been an hour --> write to hourly
        logger.write_to_csv(header, presentTime.timestamp() + "," + sample, "/HOURLY.csv");
        logger.reset_sample_counter();      // reset the tracker - wrote to hourly

        // determine whether or not to send a message
        int hourlySamples = logger.num_hours();
        if (hourlySamples >= 4 & hourlySamples < 10) {   // more than 4 hours -- send message
            String msg = logger.prep_msg();
            Serial.println(msg);
            // int iridErr = logger.send_msg(msg);     

            // if (iridErr == 0) {     // successful send -- reset counters

            //     logger.reset_sample_counter();
            //     logger.reset_hourly();
            // }
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
    // logger.tpl_done();

    delay(10000);
}




/* DATA LOGGING */

String take_measurement(){
    String msmt = String(logger.sample_batt_v()) + "," +
        logger.sample_memory() + "," +
        sample_bme280();

    return msmt;
}

String sample_bme280(){
    String sample = String(bme.readTemperature()) + "," + 
        String(bme.readHumidity()) + "," + 
        String(bme.readPressure());

    return sample;
}