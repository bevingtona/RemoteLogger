/**
 * v0.2 of RemoteLogger library for modular remote data loggers 
 * Author: Rachel Pagdin
 * June 4, 2024
*/

#ifndef RemoteLogger_h
#define RemoteLogger_h

#include <Arduino.h>
#include <time.h>
#include <SPI.h>            // for SD communcation protocol
#include <SD.h>             // for working with SD card
#include <IridiumSBD.h>         // for working with Iridium RockBlock modem
#include <RTClib.h>             // for working with RTC

#define SENSOR_ADDRESS 0            // SDI-12 sensor address
#define IridiumSerial Serial1       // define port for Iridium serial communication

class RemoteLogger
{
    public:
        
        RemoteLogger();         // basic constructor

        void blinky(int16_t n, int16_t high_ms, int16_t low_ms, int16_t btw_ms);
        void write_to_csv(String header, String datastring_for_csv, String outname);
        float sample_batt_v();
        void tpl_done();
        int send_msg(String my_msg);    // send message over Iridium

        byte ledPin = 8;        // built-in green LED pin on Feather M0 Adalogger - can modify for other boards
        byte vbatPin = 9;          // built-in battery pin on Feather M0 Adalogger - can modify for other boards
        byte tplPin = A0;           // attach TPL to A0 (only analog output pin on Adalogger)
        byte IridSlpPin = 13;           // attach Irid sleep pin (7 - grey) to pin 13 - can modify to other digital pin

    
    private:

        void sync_clock();
}