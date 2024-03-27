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
const byte sensor_set_pin = 0;          // relay set pin
const byte sensor_unset_pin = 0;        // relay unset pin
const byte data_bus_pin = 0;            // SDI-12 data bus pin (connected to Hydros sensor) (usually 12)
const byte irid_pwr_pin = 0;            // Iridium modem power (usually 13)
const byte led_pin = 8;                 // Built-in LED (pin 8)


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
    byte verbose = 1; // 0/1, 1 --> print progress messages

    DateTime present_time = rl.rtc.now();  // wake up, check time

    if(present_time.second() % 10 == 0){
        rl.blinky(1, 20, 200, 0); // blink once 

        /* Sample and write to data file*/
        String sample = take_measurement();

        if(verbose == 1){ Serial.print("Sample: ");Serial.println(sample); }

        if(verbose == 1){ Serial.println("Write to /DATA.csv"); }
        rl.write_to_csv(my_header + ",comment", present_time.timestamp() + "," + sample, "/DATA.csv"); // sample and write to CSV

        /* Check if time to save to hourly or send */
        if(verbose == 1){ Serial.print("Add row to /TRACKING.csv = "); }
        rl.write_to_csv("n", "1", "/TRACKING.csv");
        CSV_Parser cp("s", true, ','); // Set paramters for parsing the tracking file ("s" = "String")
        cp.readSDfile("/TRACKING.csv");
        int num_rows_tracking = cp.getRowsCount()-1;  //Get # of rows minus header
        if(verbose == 1){ Serial.println(num_rows_tracking); }

        // if on an hour (approx) --> write to hourly
        // 4 is for 15 min TPL set
        if(num_rows_tracking >= 4){
            if(verbose == 1){ Serial.print("Write to /HOURLY.csv = "); }
            rl.write_to_csv(my_header, present_time.timestamp() + "," + sample, "/HOURLY.csv");
            CSV_Parser cp("sfffff", true, ',');  // Set paramters for parsing the log file
            cp.readSDfile("/HOURLY.csv");
            int num_rows_hourly = cp.getRowsCount();  //Get # of rows minus header
            if(verbose == 1) { Serial.println(num_rows_hourly); }

            // more than 10 rows in hourly --> delete (too big)
            if(num_rows_hourly >= 10){
                if(verbose == 1){ Serial.println(">10 remove hourly"); }
                SD.remove("/HOURLY.csv");
            }

            // if hourly has more than 2 rows, send message
            if(num_rows_hourly >= 2 & num_rows_hourly < 10){
                if(verbose == 1){ Serial.print("Irid msg = "); }
                String msg = rl.prep_msg();
                if(verbose == 1){ Serial.println(msg); }
                int irid_err = rl.send_msg(msg);
                if(verbose == 1){ Serial.println(irid_err); }

                if(irid_err == ISBD_SUCCESS){
                    Serial.println("Message sent! Removing /HOURLY.csv");
                    SD.remove("/HOURLY.csv");        
                } 
            }
        }       
    }


    // not sure what this does but it seems important (why 4 times?)
    pinMode(A0, OUTPUT);
    digitalWrite(A0, LOW); delay(50); digitalWrite(A0, HIGH); delay(50);
    digitalWrite(A0, LOW); delay(50); digitalWrite(A0, HIGH); delay(50);
    digitalWrite(A0, LOW); delay(50); digitalWrite(A0, HIGH); delay(50);
    digitalWrite(A0, LOW); delay(50); digitalWrite(A0, HIGH); delay(50);

    delay(50);

  
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