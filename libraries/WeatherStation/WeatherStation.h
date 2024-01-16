
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
#include "SD.h"
#include "SDI12.h"              // Needed for SDI-12 communication
#include "MemoryFree.h"
#include "RTClib.h"             // Needed for communication with Real Time Clock
#include "IridiumSBD.h"         // Needed for communication with IRIDIUM modem
#include "Adafruit_SleepyDog.h" // Watchdog
#include "time.h"

/*SDI-12 sensor address, assumed to be 0*/
#define SENSOR_ADDRESS 0

/*Define Iridium seriel communication as Serial1 */
#define IridiumSerial Serial1

class WeatherStation
{
    public:
        // basic weather station functions 
        WeatherStation(String letters, String header); // arguments possibly temporary (adjust how we indicate which sensors)
        void begin();
        void read_params();
        void blinky(int16_t n, int16_t high_ms, int16_t low_ms, int16_t btw_ms);
        void write_to_csv(String header, String datastring_for_csv, String outname);
        String prep_msg();
        float sample_batt_v(); 
        String take_measurement();
        void irid_test(String msg);
        int send_msg(String my_msg);

        //data sampling functions
        String sample_hydros_M(); //sample from hydros

        //File dataFile; //file to hold the data - made local to write_to_csv() function

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
        
        // PARAMETERS (from param file)
        int16_t *sample_freq_m;
        uint16_t sample_freq_m_16;
        int16_t *irid_freq_h;
        uint16_t irid_freq_h_16;
        char **test_mode;
        String test_mode_string;
        int16_t *onstart_samples;
        uint16_t onstart_samples_16;

        // LIBRARY INSTANCES
        SDI12 mySDI12; 
        RTC_PCF8523 rtc;
        IridiumSBD modem = IridiumSBD(IridiumSerial);

        // CONSTANTS
        uint16_t blink_freq_s;
        uint16_t watchdog_timer;
};

#endif