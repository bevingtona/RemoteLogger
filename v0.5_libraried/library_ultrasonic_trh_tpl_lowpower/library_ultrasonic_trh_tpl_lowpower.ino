/**
 * sample from MaxBotix ultrasonic ranger and SHT31 Temp/RH sensor
 * TPL set to 15 minutes - send messages every 4 hours, sample every 15 minutes
 * low power mode: send messages every 24 hours, sample every hour
 * NOTE: for low power mode to work the library needs to be updated to include making multiple messages to send
 *     see v0.5_libraried/prototyping/SplitMessages for materials for this update
 * designed for smart ablation stakes
 * uses Iridium sleep pin - wired to pin 13 by default in library, to change use setIridSlpPin before calling logger.begin
 * 
 * Author: Rachel Pagdin
 * July 31, 2024
 */

#include <RemoteLogger.h>
#include <Wire.h>                   // need for SHT31
#include <Adafruit_SHT31.h>         // need for SHT31

const int iridSendHours = 4;

const int ultrasonicPowerPin = 11;          // power for MaxBotix
const int triggerPin = 10;                  // trigger to start sending ultrasonic
const int pulseInputPin = 12;               // input pin to read pulse (must be PWM compatible -- all pins but A1, A5 on Feather M0 Adalogger)
const int tempRHAddress = 0x44;             // sensor address for SHT31 temp/RH sensor - factory default 0x44

String header = "datetime,batt_v,memory,snow_depth_mm,air_2m_temp_deg_c,air_2m_temp_rh_prct";
const byte num_params = 3;
float multipliers[num_params] = {1, 10, 10};
String letters = "EFG";

Adafruit_SHT31 temp_rh = Adafruit_SHT31();                          // pass this to sample_sht31
RemoteLogger logger(header, num_params, multipliers, letters);

String take_measurement(){
    String msmt = String(logger.sample_batt_v()) + "," +
        logger.sample_memory() + "," +
        String(logger.sample_ultrasonic(ultrasonicPowerPin, triggerPin, pulseInputPin)) + "," + 
        logger.sample_sht31(temp_rh, tempRHAddress);

    return msmt;
}

void setup(){
    delay(50);

    Serial.begin(115200);
    Serial.println(F("starting the logger..."));

    logger.begin();
    Serial.println(F("logger started!"));
}

void loop(){
    delay(100);

    DateTime presentTime = logger.rtc.now();
    Serial.print(F("current logger time is ")); Serial.println(presentTime.timestamp());

    float batt_voltage = logger.sample_batt_v();
    Serial.print(F("current logger battery: ")); Serial.print(batt_voltage); Serial.println(F(" V"));

    if(batt_voltage >= 4){       // normal operation
        String sample = take_measurement();
        Serial.print(F("sample: ")); Serial.println(sample);

        logger.write_to_csv(header, presentTime.timestamp() + "," + sample, "/DATA.csv");

        logger.increment_samples();

        int samplesSinceHourly = logger.num_samples();
        Serial.print(F("it's been ")); Serial.print(samplesSinceHourly); Serial.println(F(" samples since a write to hourly"));

        if (samplesSinceHourly == 4) {    // it's been an hour --> write to hourly
            Serial.println(F("writing to hourly..."));
            logger.write_to_csv(header, presentTime.timestamp() + "," + sample, "/HOURLY.csv");
            logger.reset_sample_counter();      // reset the tracker - wrote to hourly

            // determine whether or not to send a message
            int hourlySamples = logger.num_hours();
            Serial.print(F("it's been ")); Serial.print(hourlySamples); Serial.println(F(" hours since data transmission or disposal"));
            if (hourlySamples >= iridSendHours && hourlySamples < 10) {   // more than 4 hours -- send message
                Serial.println(F("preparing message to send"));
                String msg = logger.prep_msg();
                Serial.print(F("message: ")); Serial.println(msg);

                Serial.println(F("attempting to send the message..."));
                int iridErr = logger.send_msg(msg);     

                if (iridErr == 0) {     // successful send -- reset counters
                    Serial.println(F("send successful!"));
                    logger.reset_sample_counter();
                    logger.reset_hourly();
                } else {
                    Serial.println(F("send did not work"));
                }            
            } else if (hourlySamples >= 10) {  // more than 10 hours of data -- too big, delete
                logger.reset_sample_counter();
                logger.reset_hourly();
            }

        } else if (samplesSinceHourly > 4) {    // something went wrong -- reset counters
            logger.reset_sample_counter();
            logger.reset_hourly();
        }

    } else {                // low power mode (voltage < 4V)
        Serial.println(F("low power mode"));
        int samplesSinceHourly = logger.num_samples();
        logger.increment_samples();         // no sample actually taken but want to count TPL cycles

        if(samplesSinceHourly == 4){
            Serial.println(F("it's been an hour - taking a sample"))
            String sample = take_measurement();     // only take a sample if it's been an hour
            Serial.print(F("sample: ")); Serial.println(sample);
            Serial.println(F("writing to data file and hourly..."));
            logger.write_to_csv(header, presentTime.timestamp() + "," + sample, "/DATA.csv");
            logger.write_to_csv(header, presentTime.timestamp() + "," + sample, "/HOURLY.csv");
            logger.reset_sample_counter();

            // need hours > 2 condition here to make sure it will send exactly once per day
            if((presentTime.hour() == 12 || presentTime.hour() == 13) && logger.num_hours() > 2){       // attempt to send a message at noonish
                Serial.println(F("it's midday -- attempting to send a message"));
                Serial.println(F("preparing message to send"));
                String msg = logger.low_pwr_prep_msg();     // prep a message only with most recent hourly written sample
                Serial.print(F("message: ")); Serial.println(msg);

                Serial.println(F("attempting to send the message..."));
                int iridErr = logger.send_msg(msg);

                if (iridErr == 0){
                    Serial.println(F("send successful!"));
                } else {
                    Serial.println(F("send did not work"));
                }

                if (logger.num_hours() > 48) {       // sends unsuccessful two days in a row -- remove to avoid hourly file getting too big
                    logger.reset_hourly();
                }
            }
        } else {
            Serial.println(F("no sample taken -- shutting down"));
        }
    }

    // for visual done: 
    logger.blinky(3, 500, 500, 1000);   // blink 3 times to simulate tpl done

    Serial.println(F("cycle done"));

    // trigger done pin on TPL
    logger.tpl_done();
}