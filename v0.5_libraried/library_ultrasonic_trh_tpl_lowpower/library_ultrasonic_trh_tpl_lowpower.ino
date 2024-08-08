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

    logger.begin();
}

void loop(){
    delay(100);

    DateTime presentTime = logger.rtc.now();

    float batt_voltage = logger.sample_batt_v();

    if(batt_voltage >= 4){       // normal operation
        String sample = take_measurement();



    } else {                // low power mode (voltage < 4V)
        int samplesSinceHourly = logger.num_samples();

        if(samplesSinceHourly == 4){
            String sample = take_measurement();     // only take a sample if it's been an hour

        }
    }
}