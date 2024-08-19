/**
 * v0.2 of RemoteLogger library for modular remote data loggers 
 * requires input for parameters from user: multipliers, letters, number of parameters, and header
 * Author: Rachel Pagdin
 * June 17, 2024
*/

#ifndef RemoteLogger_h
#define RemoteLogger_h

#include <Arduino.h>
#include <time.h>
#include <SPI.h>                // for SD communcation protocol
#include <SD.h>                 // for working with SD card
#include <IridiumSBD.h>         // for working with Iridium RockBlock modem
#include <RTClib.h>             // for working with RTC
#include <CSV_Parser.h>         // for reading from CSV
#include <SDI12.h>              // for SDI-12 sensors
#include <QuickStats.h>         // statistics - used for ultrasonic
#include <MemoryFree.h>         // free memory - deprecate eventually?
#include <Adafruit_SHT31.h>     // for temp/RH SHT31 sensor
#include <Wire.h>               // for temp/RH SHT31 sensor - I2C
#include <OneWire.h>            // for DS18B20 - I2C
#include <DallasTemperature.h>  // for DS18B20

#define IridiumSerial Serial1       // define port for Iridium serial communication
// #define TOTAL_KEYS 6                // number of entries in dictionary
//IridiumSBD modem(IridiumSerial);

class RemoteLogger
{
    public:
        /* CONSTRUCTORS AND STARTUP */
        RemoteLogger();
        RemoteLogger(String header);
        RemoteLogger(String header, byte num_params, float *multipliers, String letters);
        void begin();      // call after changing any pins you want to change

        /* BASIC UNIT FUNCTIONS */
        void blinky(int16_t n, int16_t high_ms, int16_t low_ms, int16_t btw_ms);
        void write_to_csv(String header, String datastring_for_csv, String outname);
        float sample_batt_v();
        int sample_memory();
        void tpl_done();
        void wipe_files();      // wipe tracking, hourly, and data files from SD card

        /* TRACKING */
        void increment_samples();
        int num_samples();
        int num_hours();
        void reset_sample_counter();        // wipe tracker file
        void reset_hourly();                // wipe hourly file

        /* TELEMETRY */
        int send_msg(String myMsg);    // send message over Iridium
        void irid_test(String msg);               // test Iridium modem (sends message)
        String prep_msg();

        /* SAMPLING FUNCTIONS */
        String sample_hydros_M(SDI12 bus, int sensor_address);
        String sample_ott_M(SDI12 bus, int sensor_address);
        String sample_ott_V(SDI12 bus, int sensor_address);
        String sample_ott(SDI12 bus, int sensor_address);     // could make two constituent functions private
        String sample_analite_195(int analogDataPin, int wiperSetPin, int wiperUnsetPin);
        long sample_ultrasonic(int powerPin, int triggerPin, int pulseInputPin);
        String sample_sht31(Adafruit_SHT31 sensor, int sensorAddress);
        String sample_DS18B20(DallasTemperature sensors, int sensorIndex);

        /* PIN ASSIGNMENT SETTERS */
        void setLedPin(byte pin);
        void setBattPin(byte pin);
        void setTplPin(byte pin);
        void setIridSlpPin(byte pin);
        void setSDSelectPin(byte pin);
        //void setDataPin(byte pin);

        RTC_PCF8523 rtc;
    
    private:

        void sync_clock();      // sync RTC to Iridium time - helper to send_msg and test_irid
        //int count_params();            // count parameters in comma-separated header - helper to prep_msg
        String produce_csv_setting();          // generate argument for CSV parsing - helper to prep_msg
        void populate_header_index(int **headerIndex, int num_params);             // determine where each header lives in dictionary - helper to prep_msg
        int find_key(String *key);                   // find index of column name in dictionary

        String myHeader;
        float *myMultipliers;
        byte myParams;
        String myLetters;

        String myCommand;
        String sdiResponse;
        File dataFile;
        QuickStats stats;       

        // IridiumSBD modem{IridiumSerial};

        byte ledPin = 8;        // built-in green LED pin on Feather M0 Adalogger - can modify for other boards
        byte vbatPin = 9;          // built-in battery pin on Feather M0 Adalogger - can modify for other boards
        byte tplPin = A0;           // attach TPL to A0 (only analog output pin on Adalogger)
        byte IridSlpPin = 13;           // attach Irid sleep pin (7 - grey) to pin 13 - can modify to other digital pin
        byte chipSelect = 4;          // SD select pin is 4 on Feather M0 Adalogger - can modify for other boards

        IridiumSBD modem{IridiumSerial, IridSlpPin};

        const float BATT_MULT = 100;
        const float MEM_MULT = 0.01;

        /* dictionary of headers to message letters and value multipliers */
        // String HEADERS[TOTAL_KEYS] = {"water_level_mm", "water_temp_c", "water_ec_dcm", "batt_v", "datetime", "memory"};
        // String LETTERS[TOTAL_KEYS] = {"A", "B", "C", "", "", ""};
        // float MULTIPLIERS[TOTAL_KEYS] = {1, 10, 1, 100, 1, 0.01};
};

#endif