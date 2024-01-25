
/*
Version 0.1 of WeatherStation library for modular hydrometric weather stations 
Author: Rachel Pagdin
December 20, 2023
includes support for blinky function
*/

#ifndef WeatherStation_h
#define WeatherStation_h

#include "Arduino.h"
#include "CSV_Parser.h"         // Needed to parse CSV files
#include "SD.h"                 // Needed for SD card
#include "SPI.h"                // Needed for SD card
#include "SDI12.h"              // Needed for SDI-12 communication
#include "MemoryFree.h"
#include "RTClib.h"             // Needed for communication with Real Time Clock
#include "IridiumSBD.h"         // Needed for communication with IRIDIUM modem
#include "Adafruit_SleepyDog.h" // Watchdog
#include "time.h"
#include "QuickStats.h"         // Stats
#include "ArduinoLowPower.h"    // Needed for putting Feather M0 to sleep between samples

/*SDI-12 sensor address, assumed to be 0*/
#define SENSOR_ADDRESS 0

/*Define Iridium serial communication as Serial1 */
#define IridiumSerial Serial1

class WeatherStation
{
    public:
        // basic weather station functions 
        WeatherStation(String letters, String header); // arguments possibly temporary (adjust how we indicate which sensors)
        void begin(); // to be called in setup()
        void run(); // to be called in loop()
        void start_and_set_pins(); //  

        void read_params();
        void blinky(int16_t n, int16_t high_ms, int16_t low_ms, int16_t btw_ms);
        void write_to_csv(String header, String datastring_for_csv, String outname);
        String prep_msg(); //currently specific to general_purpose_hydros
        float sample_batt_v(); 
        String take_measurement(); //currently specific to general_purpose_hydros
        void irid_test(String msg);
        int send_msg(String my_msg);

        //data sampling functions
        String sample_hydros_M(); //sample from hydros
        String sample_ott_M();
        String sample_ott_V();

        // PARAMETERS (from param file) 
        //make these private for safety?
        uint16_t sample_freq_m_16;
        uint16_t irid_freq_h_16;
        String test_mode_string;
        uint16_t onstart_samples_16;

        // PIN NUMBERS
        // board-specific constants 
        const byte LED_PIN = 8;                 // Built-in LED pin
        const byte BATT_PIN = 9;                // Battery pin
        const byte DATA_PIN = 12;               // The pin of the SDI-12 data bus
        const byte SD_CHIP_SELECT_PIN = 4;          // Chip select pin for SD card
        const byte HYDROS_SET_PIN = 5;          // Power relay set pin to HYDROS21
        const byte HYDROS_UNSET_PIN = 6;        // Power relay unset pin to HYDROS21
        const byte IRID_POWER_PIN = 13;         // Power base PN2222 transistor pin to Iridium modem

    private:

        // PIN NUMBERS
        byte chipSelect;
        byte SensorSetPin;
        byte SensorUnsetPin;
        byte led;
        byte vbatPin;
        byte dataPin;
        byte IridPwrPin;

        // HEADER (indicates measurements) - set in constructor
        //note: currently these are doubly represented - eventually want them to be user-determined (param file?)
        String my_letter;
        String my_header;

        String myCommand;
        String sdiResponse;

        // LIBRARY INSTANCES
        SDI12 mySDI12; 
        RTC_PCF8523 rtc;
        IridiumSBD modem = IridiumSBD(IridiumSerial);
        QuickStats stats;

        // CONSTANTS
        uint16_t blink_freq_s;
        uint16_t watchdog_timer;
        
};

#endif