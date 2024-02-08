
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

class RemoteLogger
{
    public:
        // basic weather station functions 
        RemoteLogger(); //no arg - for testing
        RemoteLogger(String letters, String header); // arguments possibly temporary (adjust how we indicate which sensors)
        void start_checks(); // start up data bus protocol, check SD and RTC  
        void start_data_bus();
        void check_card();
        void check_clock();

        void read_params();
        void blinky(int16_t n, int16_t high_ms, int16_t low_ms, int16_t btw_ms);
        void write_to_csv(String header, String datastring_for_csv, String outname);
        String prep_msg(); //currently specific to general_purpose_hydros
        float sample_batt_v(); 
        String take_measurement(); //currently specific to general_purpose_hydros
        void irid_test(String msg);
        int send_msg(String my_msg);

        //data sampling functions
        String sample_hydros_M();           // sample from Hydros
        String sample_ott_M();              // sample from OTT      /** TODO: ask Alex what these do for documentation */
        String sample_ott_V();              // sample from OTT 
        String sample_analite_195();        // sample from Analite
        long sample_ultrasonic();           // sample from ultrasonic

        // PARAMETERS (from param file) 
        //make these private for safety?
        uint16_t sample_freq_m_16;
        uint16_t irid_freq_h_16;
        String test_mode_string;
        uint16_t onstart_samples_16;
        // CONSTANTS
        uint16_t blink_freq_s;
        uint16_t watchdog_timer;

        // GLOBAL VARIABLES
        int analite_wiper_cnt;

        // PIN NUMBERS
        /* board-specific constants (Feather M0) */
        const byte LED_PIN = 8;                     // Built-in LED pin
        const byte BATT_PIN = 9;                    // Battery pin
        /** TODO: want this to be flexible --> maybe put data pin as variable into sampling function */
        const byte DATA_PIN = 12;                   // The pin of the SDI-12 data bus (usually carries data from sensor) 
        const byte SD_CHIP_SELECT_PIN = 4;          // Chip select pin for SD card
        const byte IRID_POWER_PIN = 13;             // Power base PN2222 transistor pin to Iridium modem
        /* sensor-specific pins */
        const byte HYDROS_SET_PIN = 5;              // Power relay set pin to HYDROS21
        const byte HYDROS_UNSET_PIN = 6;            // Power relay unset pin to HYDROS21
        const byte OTT_SET_PIN = 5;                 // Power relay set pin to OTT 
        const byte OTT_UNSET_PIN = 6;               // Power relay unset pin to OTT 
        const byte ANALITE_WIPER_SET_PIN = 10;      // Pin to set wiper for analite probe 
        const byte ANALITE_WIPER_UNSET_PIN = 11;    // Pin to unset wiper for analite probe
        const byte ANALITE_TURB_ANALOG = A1;        // Pin for reading analog outout from voltage divder (R1=1000 Ohm, R2=5000 Ohm) conncted to Analite
        const byte ULTRASONIC_TRIGGER_PIN = 10;     // Range start / stop pin for MaxBotix MB7369 ultrasonic ranger
        const byte ULTRASONIC_PULSE_PIN = 12;       // Pulse width pin for reading pw from MaxBotix MB7369 ultrasonic ranger -- this is just the SDI data bus pin
        const byte ULTRASONIC_PWR_PIN = 11;         // Power for ultrasonic ranger -- for low power ultrasonic (turns on to measure)
        const byte ULTRASONIC_SET_PIN = 5;          // Power relay set pin to ultrasonic ranger
        const byte ULTRASONIC_UNSET_PIN = 6;        // Power relay unset pin to ultrasonic ranger

        // LIBRARY INSTANCES
        SDI12 mySDI12 = SDI12(DATA_PIN); 
        RTC_PCF8523 rtc;
        IridiumSBD modem = IridiumSBD(IridiumSerial);
        QuickStats stats;
    
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

};

#endif