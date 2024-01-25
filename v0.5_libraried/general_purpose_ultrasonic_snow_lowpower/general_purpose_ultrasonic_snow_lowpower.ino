/*Include the libraries we need*/
#include <time.h>
#include "RTClib.h"           //Needed for communication with Real Time Clock
#include <SPI.h>              //Needed for working with SD card
#include <SD.h>               //Needed for working with SD card
#include <ArduinoLowPower.h>  //Needed for putting Feather M0 to sleep between samples
#include <IridiumSBD.h>       //Needed for communication with IRIDIUM modem
#include <CSV_Parser.h>       //Needed for parsing CSV data
#include <SDI12.h>            //Needed for SDI-12 communication
#include <QuickStats.h>       // Stats
#include <MemoryFree.h>
#include <Adafruit_SleepyDog.h>

#include <WeatherStation.h>

/*Define global constants*/
const byte chipSelect = 4;          // Chip select pin for SD card
const byte led = 8;                 // Built in led pin
const byte vbatPin = 9;             // Batt pin
const byte triggerPin = 10;         //Range start / stop pin for MaxBotix MB7369 ultrasonic ranger
const byte ultrasonicPowerPin = 11; //Pulse width pin for reading pw from MaxBotix MB7369 ultrasonic ranger
const byte pulsePin = 12;           //Pulse width pin for reading pw from MaxBotix MB7369 ultrasonic ranger
const byte IridSlpPin = 13;         // Power base PN2222 transistor pin to Iridium modem

/*Define global vars */
String my_letter = "E";
String my_header = "datetime,batt_v,memory,snow_depth_mm";
int16_t *sample_freq_m;
uint16_t sample_freq_m_16;
int16_t *irid_freq_h;
uint16_t irid_freq_h_16;
char **test_mode;
String test_mode_string;
int16_t *onstart_samples;
uint16_t onstart_samples_16;

uint16_t blink_freq_s = 60;
uint16_t watchdog_timer = 30000;

String myCommand = "";    // SDI-12 command var
String sdiResponse = "";  // SDI-12 responce var

/*Define Iridium seriel communication as Serial1 */
#define IridiumSerial Serial1

/*SDI-12 sensor address, assumed to be 0*/
#define SENSOR_ADDRESS 0

/*Create library instances*/
WeatherStation ws(my_letters, my_header);
RTC_PCF8523 rtc;                  // Setup a PCF8523 Real Time Clock instance (may have to change this to more precise DS3231)
File dataFile;                    // Setup a log file instance
IridiumSBD modem(IridiumSerial, IridSlpPin);  // Declare the IridiumSBD object
QuickStats stats;                 // Instance of QuickStats

String take_measurement() {

  digitalWrite(ultrasonicPowerPin, HIGH); delay(500);

  String msmt = String(sample_batt_v()) + "," + 
    freeMemory() + "," +
    ws.sample_ultrasonic();
  
  digitalWrite(ultrasonicPowerPin, LOW); delay(50);

  return msmt;
}

String prep_msg(){
  
  SD.begin(chipSelect);
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

  Watchdog.enable(watchdog_timer); // Initialize watchdog (decay function that will reset the logger if it runs out)

  pinMode(13, OUTPUT); digitalWrite(13, LOW); delay(50);
  pinMode(led, OUTPUT); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50);
  
  pinMode(ultrasonicPowerPin, OUTPUT); 
  digitalWrite(ultrasonicPowerPin, HIGH); delay(50);
  digitalWrite(ultrasonicPowerPin, LOW); delay(50);
    
  pinMode(IridSlpPin, OUTPUT);
  digitalWrite(IridSlpPin, LOW); delay(50);

  // CHECK RTC
  Serial.println(" - check clock");
  while (!rtc.begin()) { blinky(1, 200, 200, 2000); }

  // CHECK SD CARD
  Serial.println(" - check card");
  while (!SD.begin(chipSelect)) { blinky(2, 200, 200, 2000); }

  // READ PARAMS
  read_params();

  Watchdog.reset();

  if (test_mode_string == "T") {

    Serial.println("Testing...");

    SD.remove("/HOURLY.csv");

    Serial.begin(9600);
    Serial.println("###########################################");
    Serial.println("starting");

    Serial.println("check params");
    Serial.print(" - sample_freq_m_16: "); Serial.println(sample_freq_m_16);
    Serial.print(" - irid_freq_h_16: "); Serial.println(irid_freq_h_16);
    Serial.print(" - test_mode_string: "); Serial.println(test_mode_string);
    Serial.print(" - onstart_samples_16: "); Serial.println(onstart_samples_16);

    // CHECK SENSORS
    Watchdog.reset();
    Serial.println("check sensors");
    String datastring_start = rtc.now().timestamp() + "," + take_measurement();
    Serial.print(" - "); Serial.println(datastring_start);
    write_to_csv(my_header + ",comment", datastring_start + ", startup", "/DATA.csv");
    write_to_csv(my_header, datastring_start, "/HOURLY.csv");
    write_to_csv(my_header, datastring_start, "/HOURLY.csv");
    write_to_csv(my_header, datastring_start, "/HOURLY.csv");
    write_to_csv(my_header, datastring_start, "/HOURLY.csv");
    write_to_csv(my_header, datastring_start, "/HOURLY.csv");
    Serial.print(" - "); Serial.println(prep_msg());

    // ONSTART SAMPLES
    Serial.println("check onstart samples");
    Serial.print(" - "); Serial.println(my_header);
    for (int i = 0; i < onstart_samples_16; i++) {
      Watchdog.reset();
      String datastring_start = rtc.now().timestamp() + "," + take_measurement();
      Serial.print(" - "); Serial.println(datastring_start);
      write_to_csv(my_header + ",comment", datastring_start + ",startup sample " + i, "/DATA.csv");
    }

  
    Serial.println("check irid");
    Watchdog.disable();
    irid_test(datastring_start);
    Watchdog.enable(watchdog_timer); // Initialize watchdog (decay function that will reset the logger if it runs out)
  
    SD.remove("/HOURLY.csv");
    Watchdog.reset();

  };

  Serial.println("Awaiting delayed start ...");

  int countdownMS = Watchdog.enable(watchdog_timer); // Initialize watchdog (decay function that will reset the logger if it runs out)

}

void loop(void) {
  
  DateTime present_time = rtc.now(); // WAKE UP, WHAT TIME IS IT?
  
  // BLINK INTERVAL, THEN SLEEP
  if (present_time.second() % blink_freq_s == 0){
    blinky(1, 20, 200, 200);
    
    // TAKE A SAMPLE AT INTERVAL 
    if (present_time.minute() % sample_freq_m_16 == 0 & present_time.second() == 0){
      String sample = take_measurement();
      Watchdog.reset();
      
      // SAVE TO HOURLY ON HOUR
      if(present_time.minute() == 0){
        write_to_csv(my_header, present_time.timestamp() + "," + sample, "/HOURLY.csv");

        // SEND MESSAGE
        if (present_time.minute() == 0 & present_time.hour() % irid_freq_h_16 == 0){ 
          String msg = prep_msg();
          Watchdog.disable();
          int irid_err = send_msg(msg);
          Watchdog.enable(watchdog_timer);
          SD.remove("/HOURLY.csv");
          Watchdog.reset();
        }
      }
         
      write_to_csv(my_header + ",comment", present_time.timestamp() + "," + sample, "/DATA.csv");// SAMPLE - WRITE TO CSV
      Watchdog.disable();
      Watchdog.enable(100);
      delay(200); // TRIGGER WATCHDOG

    }
    
    DateTime sample_end = rtc.now();
    uint32_t sleep_time = ((blink_freq_s - (sample_end.second() % blink_freq_s)) * 1000.0) - 1000;
    LowPower.deepSleep(sleep_time);
    }
  
  Watchdog.reset();
  delay(500); // Half second to make sure we do not skip a second
}

void blinky(int16_t n, int16_t high_ms, int16_t low_ms, int16_t btw_ms) {
  for (int i = 1; i <= n; i++) {
    digitalWrite(led, HIGH);
    delay(high_ms);
    digitalWrite(led, LOW);
    delay(low_ms);
  }
  delay(btw_ms);
}

long sample_ultrasonic(){

  pinMode(triggerPin, OUTPUT);       //Set ultrasonic ranging trigger pin as OUTPUT
  pinMode(pulsePin, INPUT);          //Set ultrasonic pulse width pin as INPUT
  float values[10];          //Array for storing sampled distances
  digitalWrite(triggerPin, HIGH);  //Write the ranging trigger pin HIGH
  delay(30);    
  for (int16_t i = 0; i < 10; i++)  //Take N samples
  {
    int32_t duration = pulseIn(pulsePin, HIGH);  //Get the pulse duration (i.e.,time of flight)
    values[i] = duration;
    delay(150);  //Dont sample too quickly < 7.5 Htz
  }
  long med_distance = stats.minimum(values, 10);  //Copute median distance
  digitalWrite(triggerPin, LOW);  //Write the ranging trigger pin HIGH
  return med_distance;
}

float sample_batt_v() {
  pinMode(vbatPin, INPUT);
  float batt_v = (analogRead(vbatPin) * 2 * 3.3) / 1024;
  return batt_v;
}

void write_to_csv(String header, String datastring_for_csv, String outname) {
 
  // IF FILE DOES NOT EXIST, WRITE HEADER AND DATA, ELSE, WITE DATA
  if (!SD.exists(outname))  //Write header if first time writing to the logfile
  {
    dataFile = SD.open(outname, FILE_WRITE);  //Open file under filestr name from parameter file
    if (dataFile) {
      dataFile.println(header);
      dataFile.println(datastring_for_csv);
    }
    dataFile.close();  //Close the dataFile
  } else {
    dataFile = SD.open(outname, FILE_WRITE);
    if (dataFile) {
      dataFile.println(datastring_for_csv);
      dataFile.close();
    }
  }
}

int send_msg(String my_msg) {

  write_to_csv("log", rtc.now().timestamp() + " startup", "iridlog.txt");

  // digitalWrite(IridPwrPin, HIGH);  //Drive iridium power pin LOW
  // delay(2000);

  IridiumSerial.begin(19200);                            // Start the serial port connected to the satellite modem
  modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);  // This is a low power application

  int err = modem.begin();

  if (err == ISBD_IS_ASLEEP) {
    write_to_csv("log", rtc.now().timestamp() + " wake up " + err, "iridlog.txt");
    modem.begin();
  }

  write_to_csv("log", rtc.now().timestamp() + " begin " + err, "iridlog.txt");
  // Serial.println(("begin");

  if (err == 10) {
    err = modem.begin();
    write_to_csv("log", rtc.now().timestamp() + " begin retry " + err, "iridlog.txt");
    // Serial.println(("retry");
  }

  err = modem.sendSBDText(my_msg.c_str());
  // Serial.println(("sent");
  write_to_csv("log", rtc.now().timestamp() + " sent " + err, "iridlog.txt");
  // Serial.println((err);

  if (err != ISBD_SUCCESS && err != 13) {
    err = modem.begin();
    write_to_csv("log", rtc.now().timestamp() + " begin2 " + err, "iridlog.txt");
    // Serial.println(("begin 2");

    modem.adjustSendReceiveTimeout(500);
    err = modem.sendSBDText(my_msg.c_str());
    write_to_csv("log", rtc.now().timestamp() + " sent2 " + err, "iridlog.txt");
    // Serial.println(("sent 2");
  }

  if (rtc.now().day() % 2 == 0 & rtc.now().hour() == 12) {
    struct tm t;
    int err_time = modem.getSystemTime(t);
    if (err_time == ISBD_SUCCESS) {
      String pre_time = rtc.now().timestamp();
      rtc.adjust(DateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec));
      String post_time = rtc.now().timestamp();
      write_to_csv("pre_sync,post_sync,err_time", pre_time + "," + post_time + "," + err_time, "/IRID.csv");
      // Serial.println(("sync clock");
    }
  }

  modem.sleep();

  return err;
}

void irid_test(String msg) {

  // pinMode(IridPwrPin, OUTPUT);     //Set iridium power pin as OUTPUT
  // digitalWrite(IridPwrPin, HIGH);  //Drive iridium power pin LOW
  // delay(2000);

  int signalQuality = -1;

  IridiumSerial.begin(19200);                            // Start the serial port connected to the satellite modem
  modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);  // This is a low power application

  // Begin satellite modem operation
  Serial.println(" - starting modem...");
  int err = modem.begin();

  if (err != ISBD_SUCCESS) {
    Serial.print(" - begin failed: error ");
    Serial.println(err);
    if (err == ISBD_NO_MODEM_DETECTED)
      Serial.println(" - no modem detected: check wiring.");
    return;
  }

  // Example: Print the firmware revision
  char version[12];
  err = modem.getFirmwareVersion(version, sizeof(version));
  if (err != ISBD_SUCCESS) {
    Serial.print(" - firmware version failed: error ");
    Serial.println(err);
    return;
  }

  Serial.print(" - firmware version is ");
  Serial.print(version);
  Serial.println(".");

  int n = 0;
  while (n < 10) {
    err = modem.getSignalQuality(signalQuality);
    if (err != ISBD_SUCCESS) {
      Serial.print(" - signalQuality failed: error ");
      Serial.println(err);
      return;
    }

    Serial.print(" - signal quality is currently ");
    Serial.print(signalQuality);
    Serial.println(".");
    n = n + 1;
    delay(1000);
  }

  // Send the message
  Serial.print(" - Attempting: ");
  msg = "Hello world! " + msg;
  Serial.println(msg);

  err = modem.sendSBDText(msg.c_str());

  if (err != ISBD_SUCCESS) {
    Serial.print(" - sendSBDText failed: error ");
    Serial.println(err);
    if (err == ISBD_SENDRECEIVE_TIMEOUT)
      Serial.println(" - try again with a better view of the sky.");
  } else {
    Serial.println(" - hey, it worked!");
  }

  modem.sleep();

}

void read_params() {

  CSV_Parser cp("ddsd", true, ',');
  Serial.println(" - check param.txt");
  while (!cp.readSDfile("/PARAM.txt")) { blinky(3, 200, 200, 1000); }
  cp.parseLeftover();

  sample_freq_m = (int16_t *)cp["sample_freq_m"];
  sample_freq_m_16 = sample_freq_m[0];
  irid_freq_h = (int16_t *)cp["irid_freq_h"];
  irid_freq_h_16 = irid_freq_h[0];
  test_mode = (char **)cp["test_mode"];
  test_mode_string = String(test_mode[0]);
  onstart_samples = (int16_t *)cp["onstart_samples"];
  onstart_samples_16 = onstart_samples[0];

  delete sample_freq_m;
  delete irid_freq_h;
  delete test_mode;
  delete onstart_samples;
}