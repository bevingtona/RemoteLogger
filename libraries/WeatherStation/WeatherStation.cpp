/*
Version 0.1 of WeatherStation library for modular hydrometric weather stations 
Author: Rachel Pagdin
December 20, 2023
includes support for blinky function
*/

#include "Arduino.h"
#include "WeatherStation.h"
#include "CSV_Parser.h"

//constructor
WeatherStation::WeatherStation(){
    _led = 8; //pin 8 is LED on Feather M0
}

void WeatherStation::begin(){
    pinMode(_led, OUTPUT);
}

void WeatherStation::read_params(){
    CSV_Parser cp("ddsd", true, ',');
    Serial.println(" - check param.txt");
    while(!cp.readSDfile("/PARAM.txt")) { blinky(3, 200, 200, 1000); } //blink while reading the file from SD
    cp.parseLeftover();

    /* Assign to class member variables (public) */
    sample_freq_m = (int16_t *) cp["sample_freq_m"];
    sample_freq_m_16 = sample_freq_m[0];
    irid_freq_h = (int16_t *)cp["irid_freq_h"];
    irid_freq_h_16 = irid_freq_h[0];
    test_mode = (char **)cp["test_mode"];
    test_mode_string = String(test_mode[0]);
    onstart_samples = (int16_t *)cp["onstart_samples"];
    onstart_samples_16 = onstart_samples[0];

    /* Get rid of all the stuff we don't need (save space) */
    delete sample_freq_m;
    delete irid_freq_h;
    delete test_mode;
    delete onstart_samples;
}

void WeatherStation::blinky(int16_t n, int16_t high_ms, int16_t low_ms, int16_t btw_ms){
    for(int i = 1; i <= n; i++){
        digitalWrite(_led, HIGH);
        delay(high_ms);
        digitalWrite(_led, LOW);
        delay(low_ms);
    }
    delay(btw_ms);
}