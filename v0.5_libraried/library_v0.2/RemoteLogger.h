/**
 * v0.2 of RemoteLogger library for modular remote data loggers 
 * Author: Rachel Pagdin
 * June 4, 2024
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
#include <SDI-12.h>             // for SDI-12 sensors
#include <QuickStats.h>         // statistics - used for ultrasonic
#include <MemoryFree.h>         // free memory - deprecate eventually?

#define IridiumSerial Serial1       // define port for Iridium serial communication

class RemoteLogger
{
    public:
        /* CONSTRUCTORS AND STARTUP */
        RemoteLogger();
        RemoteLogger(String header);
        void begin();      // call after changing any pins you want to change

        /* BASIC UNIT FUNCTIONS */
        void blinky(int16_t n, int16_t high_ms, int16_t low_ms, int16_t btw_ms);
        void write_to_csv(String header, String datastring_for_csv, String outname);
        float sample_batt_v();
        int sample_memory();
        void tpl_done();

        /* TRACKING */
        void increment_samples();
        int num_samples();
        int num_hours();
        void reset_sample_counter();
        void reset_hourly();

        /* TELEMETRY */
        int send_msg(String my_msg);    // send message over Iridium
        void irid_test(String msg);               // test Iridium modem (sends message)
        String prep_msg();

        /* SAMPLING FUNCTIONS */
        String sample_hydros_M(SDI12 bus, int sensor_address);

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
        int count_params(String header);            // count parameters in comma-separated header - helper to prep_msg
        String produce_csv_setting(int n);          // generate argument for CSV parsing - helper to prep_msg
        void populate_header_index(int **headerIndex);             // determine where each header lives in dictionary - helper to prep_msg
        int find_key(String key);                   // find index of column name in dictionary

        String myHeader;

        IridiumSDB modem;

        byte ledPin = 8;        // built-in green LED pin on Feather M0 Adalogger - can modify for other boards
        byte vbatPin = 9;          // built-in battery pin on Feather M0 Adalogger - can modify for other boards
        byte tplPin = A0;           // attach TPL to A0 (only analog output pin on Adalogger)
        byte IridSlpPin = 13;           // attach Irid sleep pin (7 - grey) to pin 13 - can modify to other digital pin
        byte chipSelect = 4;          // SD select pin is 4 on Feather M0 Adalogger - can modify for other boards
        //byte dataPin = 12;              // attach SDI-12 data line to pin 12 

        /* dictionary of headers to message letters and value multipliers */
        int TOTAL_KEYS = 6;           // number of supported column headers (length of all dictionary arrays)
        String HEADERS[] = {"water_level_mm", "water_temp_c", "water_ec_dcm", "batt_v", "datetime", "memory"};
        String LETTERS[] = {"A", "B", "C", "", "", ""};
        float MULTIPLIERS[] = {1, 10, 1, 100, 1, 0.01};
}