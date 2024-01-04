/*
Version 0.1 of WeatherStation library for modular hydrometric weather stations 
Author: Rachel Pagdin
December 20, 2023
includes support for blinky function
*/

#ifndef WeatherStation_h
#define WeatherStation_h

#include "Arduino.h"
#include "CSV_Parser.h" //needed to parse CSV files

class WeatherStation
{
    public:
        WeatherStation();
        void begin();
        void read_params();
        void blinky(int16_t n, int16_t high_ms, int16_t low_ms, int16_t btw_ms);

        //sampling functions
        //String sample_hydros_M(); //sample from hydros

        //parameters (from param file)
        int16_t *sample_freq_m;
        uint16_t sample_freq_m_16;
        int16_t *irid_freq_h;
        uint16_t irid_freq_h_16;
        char **test_mode;
        String test_mode_string;
        int16_t *onstart_samples;
        uint16_t onstart_samples_16;
    private:
        byte _led;
};

#endif