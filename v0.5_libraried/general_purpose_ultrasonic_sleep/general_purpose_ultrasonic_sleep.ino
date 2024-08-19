/**
 * January 25, 2024: fully converted to use RemoteLogger library
 * 
 * Ultrasonic ranger measures water level
*/

/*Include the libraries we need*/
#include <time.h>
#include "RTClib.h"           //Needed for communication with Real Time Clock
#include <SPI.h>              //Needed for working with SD card
#include <SD.h>               //Needed for working with SD card
#include <ArduinoLowPower.h>  //Needed for putting Feather M0 to sleep between samples
#include <CSV_Parser.h>       //Needed for parsing CSV data
#include <SDI12.h>            //Needed for SDI-12 communication
#include <QuickStats.h>       // Stats
#include <MemoryFree.h>
#include <Adafruit_SleepyDog.h>

#include <RemoteLogger.h>

/*Define global vars */
String my_letter = "A";
String my_header = "datetime,batt_v,memory,water_level_mm";

String myCommand = "";    // SDI-12 command var
String sdiResponse = "";  // SDI-12 responce var

/*Create library instances*/
RemoteLogger rl(my_letter, my_header);    // custom library

String take_measurement() {

  digitalWrite(rl.ULTRASONIC_SET_PIN, HIGH); delay(50);
  digitalWrite(rl.ULTRASONIC_SET_PIN, LOW); delay(1000);

  String msmt = String(rl.sample_batt_v()) + "," + 
    freeMemory() + "," +
    rl.sample_ultrasonic();
  
  digitalWrite(rl.ULTRASONIC_UNSET_PIN, HIGH); delay(50);
  digitalWrite(rl.ULTRASONIC_UNSET_PIN, LOW); delay(50);

  return msmt;
}

String prep_msg(){
  
  SD.begin(rl.SD_CHIP_SELECT_PIN);
  CSV_Parser cp("sfff", true, ',');  // Set paramters for parsing the log file
  cp.readSDfile("/HOURLY.csv");
  int num_rows = cp.getRowsCount();  //Get # of rows
    
  char **out_datetimes = (char **)cp["datetime"];
  float *out_mem = (float *)cp["memory"];
  float *out_batt_v = (float *)cp["batt_v"];
  float *out_water_level_mm = (float *)cp["water_level_mm"];
  
  String datastring_msg = 
    my_letter + ":" +
    String(out_datetimes[0]).substring(2, 4) + 
    String(out_datetimes[0]).substring(5, 7) + 
    String(out_datetimes[0]).substring(8, 10) + 
    String(out_datetimes[0]).substring(11, 13) + ":" +
    String(round(out_batt_v[num_rows-1] * 100)) + ":" +
    String(round(out_mem[num_rows-1] / 100)) + ":";
  
  for (int i = 0; i < num_rows; i++) {  //For each observation in the IRID.csv
    datastring_msg = 
      datastring_msg + 
      String(round(out_water_level_mm[i])) + ':';              
    }

  return datastring_msg;
}

void setup(void) {
    
  set_pins();

  // Ultrasonic does not use databus --> no need to start
  /** TODO: is this a problem for space usage? or usability? -- did it for flexibility */
  rl.check_clock();
  rl.check_card();

  // READ PARAMS
  rl.read_params();


  if (rl.test_mode_string == "T") {

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

  };

  Serial.println("Awaiting delayed start ...");

  int countdownMS = Watchdog.enable(rl.watchdog_timer); // Initialize watchdog (decay function that will reset the logger if it runs out)

}

void loop(void) {
  
  DateTime present_time = rl.rtc.now(); // WAKE UP, WHAT TIME IS IT?
  
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
          Watchdog.disable();
          int irid_err = rl.send_msg(msg);
          Watchdog.enable(rl.watchdog_timer);
          SD.remove("/HOURLY.csv");
          Watchdog.reset();
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


/* HELPER METHODS */

void set_pins(){
  pinMode(13, OUTPUT); digitalWrite(13, LOW); delay(50);
  pinMode(rl.ledPin, OUTPUT); digitalWrite(rl.ledPin, HIGH); delay(50); digitalWrite(rl.ledPin, LOW); delay(50);
  
  pinMode(rl.ULTRASONIC_SET_PIN, OUTPUT); 
  digitalWrite(rl.ULTRASONIC_SET_PIN, HIGH); delay(50);
  digitalWrite(rl.ULTRASONIC_SET_PIN, LOW); delay(50);
  
  pinMode(rl.ULTRASONIC_UNSET_PIN, OUTPUT);
  digitalWrite(rl.ULTRASONIC_UNSET_PIN, HIGH); delay(50);
  digitalWrite(rl.ULTRASONIC_UNSET_PIN, LOW); delay(50);
  
  pinMode(rl.IRID_POWER_PIN, OUTPUT);
  digitalWrite(rl.IRID_POWER_PIN, LOW); delay(50);
}
