// /**
//  * January 25, 2024: fully converted to use RemoteLogger library
//  * 
//  * Ultrasonic ranger measures water level
// */

// /*Include the libraries we need*/
// #include <time.h>
#include "RTClib.h"           //Needed for communication with Real Time Clock
// #include <SPI.h>              //Needed for working with SD card
// #include <SD.h>               //Needed for working with SD card
// #include <ArduinoLowPower.h>  //Needed for putting Feather M0 to sleep between samples
// #include <CSV_Parser.h>       //Needed for parsing CSV data
// #include <SDI12.h>            //Needed for SDI-12 communication
// #include <QuickStats.h>       // Stats
// #include <MemoryFree.h>
// #include <Adafruit_SleepyDog.h>

#include <RemoteLogger.h>

// /*Define global vars */
String my_letter = "A";
String my_header = "datetime,batt_v,memory,water_level_mm";

// String myCommand = "";    // SDI-12 command var
// String sdiResponse = "";  // SDI-12 responce var


// /*Create library instances*/
RemoteLogger rl(my_letter, my_header);    // custom library

// String take_measurement() {

//   String msmt = String(rl.sample_batt_v()) + "," + 
//     freeMemory() + "," +
//     rl.sample_ultrasonic();

//   return msmt;
// }


void setup(void) {
  Serial.begin(9600);
  
  // set_pins();

}

void loop(void) {
  
  DateTime present_time = rl.rtc.now(); // WAKE UP, WHAT TIME IS IT?
  
  Serial.println(present_time.timestamp());

  // // BLINK INTERVAL, THEN SLEEP
  // if (present_time.second() % 10 == 0){
  //   rl.blinky(1, 20, 200, 200);
    
  //   // TAKE A SAMPLE AT INTERVAL 
  //   if (present_time.minute() % rl.sample_freq_m_16 == 0 & present_time.second() == 0){
  //     String sample = take_measurement();
  //     Watchdog.reset();
      
  //     // SAVE TO HOURLY ON HOUR
  //     if(present_time.minute() == 0){
  //       rl.write_to_csv(my_header, present_time.timestamp() + "," + sample, "/HOURLY.csv");

  //       // SEND MESSAGE
  //       if (present_time.minute() == 0 & present_time.hour() % rl.irid_freq_h_16 == 0){ 
  //         String msg = prep_msg();
  //         Watchdog.disable();
  //         int irid_err = rl.send_msg(msg);
  //         Watchdog.enable(rl.watchdog_timer);
  //         SD.remove("/HOURLY.csv");
  //         Watchdog.reset();
  //       }
  //     }
         
  //     rl.write_to_csv(my_header + ",comment", present_time.timestamp() + "," + sample, "/DATA.csv");// SAMPLE - WRITE TO CSV
  //     Watchdog.disable();
  //     Watchdog.enable(100);
  //     delay(200); // TRIGGER WATCHDOG

  //   }
    
  //   DateTime sample_end = rl.rtc.now();
  //   uint32_t sleep_time = ((rl.blink_freq_s - (sample_end.second() % rl.blink_freq_s)) * 1000.0) - 1000;
  //   LowPower.sleep(sleep_time);
  //   }
  
  // Watchdog.reset();
  delay(500); // Half second to make sure we do not skip a second
}


// /* HELPER METHODS */

// void set_pins(){
//   pinMode(13, OUTPUT); digitalWrite(13, LOW); delay(50);
//   pinMode(rl.LED_PIN, OUTPUT); digitalWrite(rl.LED_PIN, HIGH); delay(50); digitalWrite(rl.LED_PIN, LOW); delay(50);
  
//   pinMode(rl.ULTRASONIC_SET_PIN, OUTPUT); 
//   digitalWrite(rl.ULTRASONIC_SET_PIN, HIGH); delay(50);
//   digitalWrite(rl.ULTRASONIC_SET_PIN, LOW); delay(50);
  
//   pinMode(rl.ULTRASONIC_UNSET_PIN, OUTPUT);
//   digitalWrite(rl.ULTRASONIC_UNSET_PIN, HIGH); delay(50);
//   digitalWrite(rl.ULTRASONIC_UNSET_PIN, LOW); delay(50);
  
//   pinMode(rl.IRID_POWER_PIN, OUTPUT);
//   digitalWrite(rl.IRID_POWER_PIN, LOW); delay(50);
// }

