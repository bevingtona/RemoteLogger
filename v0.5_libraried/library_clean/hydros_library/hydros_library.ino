/**
 * March 27, 2024
 * @author: Rachel Pagdin - adapted from general_purpose_hydros
 * no TPL chip functionality
*/


/* Include libraries */
#include <time.h>
#include <SPI.h>
#include <SD.h>
#include <ArduinoLowPower.h>
#include <CSV_Parser.h>
#include <MemoryFree.h>
#include <Adafruit_SleepyDog.h>

#include <RemoteLogger.h> //custom library

/* Unit-specific information  */
const String my_letter = "ABC";  // see measurement_dictionary.md for letters
const String my_header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";  // correspond to letters

// enter these based on chosen pins (in circuit)
const byte sensor_set_pin = 0;                  // relay set pin
const byte sensor_unset_pin = 0;                // relay unset pin
const byte data_bus_pin = 0;                    // SDI-12 data bus pin (connected to Hydros sensor) (usually 12)
const byte irid_pwr_pin = 0;                    // Iridium modem power (usually 13)
const byte led_pin = 8;                         // Built-in LED (pin 8)


RemoteLogger rl(my_letter, my_header);  // constructor for built-in message prep

void setup(){

    set_pins(); // set up the pins
    rl.start_checks(); // data bus, SD card, RTC
    rl.read_params(); // read in param file

    /* TEST MODE */
    // prints to Serial -- attach to laptop to see output
    if(rl.test_mode_string == "T"){
        SD.remove("/HOURLY.csv");

        delay(3000);
        Serial.begin(9600);
        Serial.println("###########################################");
        Serial.println("starting");

        Serial.println("check params");
        Serial.print(" - sample_freq_m_16: "); Serial.println(rl.sample_freq_m_16);
        Serial.print(" - irid_freq_h_16: "); Serial.println(rl.irid_freq_h_16);
        Serial.print(" - test_mode_string: "); Serial.println(rl.test_mode_string);
        Serial.print(" - onstart_samples_16: "); Serial.println(rl.onstart_samples_16);

        // CHECK SENSORS
        Serial.println("check sensors");
        String datastring_start = rl.rtc.now().timestamp() + "," + take_measurement();
        Serial.print(" - "); Serial.println(datastring_start);
        rl.write_to_csv(my_header + ",comment", datastring_start + ", startup", "/DATA.csv");
        rl.write_to_csv(my_header, datastring_start, "/HOURLY.csv");
        rl.write_to_csv(my_header, datastring_start, "/HOURLY.csv");
        rl.write_to_csv(my_header, datastring_start, "/HOURLY.csv");
        rl.write_to_csv(my_header, datastring_start, "/HOURLY.csv");
        rl.write_to_csv(my_header, datastring_start, "/HOURLY.csv");
        Serial.print(" - "); Serial.println(prep_msg());

        // ONSTART SAMPLES
        Serial.println("check onstart samples");
        Serial.print(" - "); Serial.println(my_header);
        for (int i = 0; i < rl.onstart_samples_16; i++) {
            String datastring_start = rl.rtc.now().timestamp() + "," + take_measurement();
            Serial.print(" - "); Serial.println(datastring_start);
            rl.write_to_csv(my_header + ",comment", datastring_start + ",startup sample " + i, "/DATA.csv");
        }
  
        Serial.println("check irid");
        rl.irid_test(datastring_start);
  
        SD.remove("/HOURLY.csv");

    }

    int countdownMS = Watchdog.enable(rl.watchdog_timer); // Initialize watchdog (decay function that will reset the logger if it runs out)

}

void loop(){
    DateTime present_time = rl.rtc.now();  // wake up, check time
  
    // BLINK INTERVAL, THEN SLEEP
    if (present_time.second() % 10 == 0){
        rl.blinky(1, 20, 200, 200);
    
        // TAKE A SAMPLE AT INTERVAL 
        if (present_time.minute() % rl.sample_freq_m_16 == 0 & present_time.second() == 0){
            String sample = take_measurement();
            Watchdog.reset();
      
            // SAVE TO HOURLY ON HOUR
            if(present_time.minute() == 0){
                rl.write_to_csv(my_header, present_time.timestamp() + "," + sample, "/HOURLY.csv");

                // SEND MESSAGE
                if (present_time.minute() == 0 & present_time.hour() % rl.irid_freq_h_16 == 0){ 
                    String msg = prep_msg();
                    int irid_err = rl.send_msg(msg);
                    SD.remove("/HOURLY.csv");

                }
            }
         
            rl.write_to_csv(my_header + ",comment", present_time.timestamp() + "," + sample, "/DATA.csv");// SAMPLE - WRITE TO CSV
            Watchdog.disable();
            Watchdog.enable(100);
            delay(200); // TRIGGER WATCHDOG

        }
    
        DateTime sample_end = rl.rtc.now();
        uint32_t sleep_time = ((rl.blink_freq_s - (sample_end.second() % rl.blink_freq_s)) * 1000.0) - 1000;
        LowPower.sleep(sleep_time);
    }
  
    Watchdog.reset();
    delay(500); // Half second to make sure we do not skip a second
}

String take_measurement(){
    digitalWrite(sensor_set_pin, HIGH); delay(50);
    digitalWrite(sensor_set_pin, LOW); delay(1000);

    String msmt = String(rl.sample_batt_v()) + "," + 
        freeMemory() + "," +
        rl.sample_hydros_M();

    digitalWrite(sensor_unset_pin, HIGH); delay(50);
    digitalWrite(sensor_unset_pin, LOW); delay(1000);

}

void set_pins(){
    pinMode(sensor_set_pin, OUTPUT); digitalWrite(sensor_set_pin, HIGH); delay(50); digitalWrite(sensor_set_pin, LOW); delay(50); // relay set pin, turn on/off
    pinMode(sensor_unset_pin, OUTPUT); digitalWrite(sensor_unset_pin, HIGH); delay(50); digitalWrite(sensor_unset_pin, LOW); delay(50); // relay unset pin, turn on/off

    pinMode(data_bus_pin, INPUT); // SDI 12 data bus 

    pinMode(irid_pwr_pin, OUTPUT); digitalWrite(irid_pwr_pin, LOW); // RockBlock power, set to off default

    pinMode(led_pin, OUTPUT); // LED
}