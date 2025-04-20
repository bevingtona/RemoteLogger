// #define ISBD_SUCCESS             0
// #define ISBD_ALREADY_AWAKE       1
// #define ISBD_SERIAL_FAILURE      2
// #define ISBD_PROTOCOL_ERROR      3
// #define ISBD_CANCELLED           4
// #define ISBD_NO_MODEM_DETECTED   5
// #define ISBD_SBDIX_FATAL_ERROR   6
// #define ISBD_SENDRECEIVE_TIMEOUT 7
// #define ISBD_RX_OVERFLOW         8
// #define ISBD_REENTRANT           9
// #define ISBD_IS_ASLEEP           10
// #define ISBD_NO_SLEEP_PIN        11

/*Include the libraries we need*/
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
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

/*Define global constants*/
const byte chipSelect = 4;      // Chip select pin for SD card
const byte led = 8;             // Built in led pin
const byte vbatPin = 9;         // Batt pin
const byte triggerPin = 10; //Range start / stop pin for MaxBotix MB7369 ultrasonic ranger
const byte pulsePin = 12; //Pulse width pin for reading pw from MaxBotix MB7369 ultrasonic ranger
const byte IridSlpPin = 13;     // Power base PN222 2 transistor pin to Iridium modem

/*Define global vars */
String my_letter = "EFG";
String my_header = "datetime,batt_v,memory,snow_depth_mm,air_2m_temp_deg_c,air_2m_temp_rh_prct";
int err;

String myCommand = "";    // SDI-12 command var
String sdiResponse = "";  // SDI-12 responce var

/*Define Iridium seriel communication as Serial1 */
#define IridiumSerial Serial1

/*SDI-12 sensor address, assumed to be 0*/
#define SENSOR_ADDRESS 0

/*Create library instances*/
RTC_PCF8523 rtc;                  // Setup a PCF8523 Real Time Clock instance (may have to change this to more precise DS3231)
File dataFile;                    // Setup a log file instance
// IridiumSBD modem(IridiumSerial);  // Declare the IridiumSBD object
IridiumSBD modem(IridiumSerial, IridSlpPin);  // Declare the IridiumSBD object
Adafruit_SHT31 sht31 = Adafruit_SHT31();
QuickStats stats;                 // Instance of QuickStats

String take_measurement() {

  String msmt = String(sample_batt_v()) + "," + 
    freeMemory() + "," + 
    sample_ultrasonic() + "," + 
    sample_sht();

  return msmt;
}

String prep_msg(){
  
  SD.begin(chipSelect);
  CSV_Parser cp("sfffff", true, ',');  // Set paramters for parsing the log file
  cp.readSDfile("/HOURLY.csv");
  int num_rows = cp.getRowsCount();  //Get # of rows
    
  char **out_datetimes = (char **)cp["datetime"];
  float *out_mem = (float *)cp["memory"];
  float *out_batt_v = (float *)cp["batt_v"];
  float *out_snow_depth_mm = (float *)cp["snow_depth_mm"];
  float *out_air_2m_temp_deg_c = (float *)cp["air_2m_temp_deg_c"];
  float *out_air_2m_temp_rh_prct = (float *)cp["air_2m_temp_rh_prct"];
  
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
      String(round(out_snow_depth_mm[i])) + ',' + 
      String(round(out_air_2m_temp_deg_c[i]*10)) + ',' + 
      String(round(out_air_2m_temp_rh_prct[i]*10)) + ':'; // Serial.println(datastring_msg);
    }
  return datastring_msg;
}

void setup(void) {
  
  delay(2000);
  Serial.println("Starting up...");
  pinMode(13, OUTPUT); digitalWrite(13, LOW); delay(50);
  pinMode(led, OUTPUT); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50);
  
  pinMode(A0, OUTPUT);
  digitalWrite(A0, LOW); delay(50); 

  pinMode(triggerPin, OUTPUT);//Set ultrasonic ranging trigger pin as OUTPUT
  digitalWrite(triggerPin, LOW);//Drive ultrasonic randing trigger pin LOW
  pinMode(pulsePin, INPUT);//Set ultrasonic pulse width pin as INPUT

  pinMode(IridSlpPin, OUTPUT);
  digitalWrite(IridSlpPin, LOW); delay(50);
  modem.sleep();

  // CHECK RTC
  Serial.println(" - check clock");
  while (!rtc.begin()) { blinky(1, 200, 200, 2000); }

  // CHECK SD CARD
  Serial.println(" - check card");
  SD.begin(chipSelect);
  while (!SD.begin(chipSelect)) { blinky(2, 200, 200, 2000); }

  delay(100);
  Serial.begin(9600);

}

void loop(void) {

  delay(100);

  Serial.println("###########################################");
    
  // READ TIME
  DateTime present_time = rtc.now();
  // Serial.println(present_time.timestamp());
  
  // TAKE MEASUREMENT
  String sample = take_measurement();
  Serial.println(my_header);
  Serial.println(present_time.timestamp());
  Serial.print("Sample: ");
  Serial.println(sample);
  
  // WRITE TO DATA.csv
  Serial.println("Write to /DATA.csv");
  write_to_csv(my_header + ",comment", present_time.timestamp() + "," + sample, "/DATA.csv");// SAMPLE - WRITE TO CSV

  // INCREMENT TRACKING.csv to know how many samples have been taken
  Serial.print("Add row to /TRACKING.csv = ");
  write_to_csv("n", "1", "/TRACKING.csv");
  CSV_Parser cp("s", true, ','); // Set paramters for parsing the tracking file ("s" = "String")
  cp.readSDfile("/TRACKING.csv");
  int num_rows_tracking = cp.getRowsCount()-1;  //Get # of rows minus header
  Serial.println(num_rows_tracking);
  blinky(num_rows_tracking,100,200,2000);
  
  // HOW MANY SAMPLES UNTIL YOU WRITE TO HOURLY (for TPL at 15m, then this should be >=4)
  if(num_rows_tracking == 4){ 
    
    // WRITE TO HOURLY
    Serial.print("Write to /HOURLY.csv = ");
    write_to_csv(my_header, present_time.timestamp() + "," + sample, "/HOURLY.csv");
    SD.begin(chipSelect);
    CSV_Parser cp("sfffff", true, ',');  // Set paramters for parsing the log file
    cp.readSDfile("/HOURLY.csv");
    int num_rows_hourly = cp.getRowsCount();  //Get # of rows minus header
    Serial.println(num_rows_hourly);
    
    // If HOURLY >= 2 rows, then send
    if(num_rows_hourly >= 2 & num_rows_hourly < 10){
      
      // PARSE MSG FROM HOURLY.csv
      Serial.print("Irid msg = ");
      String msg = prep_msg();
      Serial.println(msg);

      // SEND!
      int irid_err = send_msg(msg); 
      
      // IF SUCCESS, delete hourly (if not, will try again at next hourly interval)
      if(irid_err == 0){
        Serial.println("Message sent! Removing /HOURLY.csv");
        SD.remove("/HOURLY.csv");        
        Serial.println("Remove tracking");
        SD.remove("/TRACKING.csv");    
        }      
      }

    // If HOURLY > 10 rows, then delete it! Probably too big to send
    if(num_rows_hourly >= 10){
      Serial.println(">10 remove hourly");
      SD.remove("/HOURLY.csv");        
      Serial.println("Remove tracking");
      SD.remove("/TRACKING.csv");
    }

    Serial.println("Remove tracking");
    SD.remove("/TRACKING.csv");
    }
    
    if(num_rows_tracking > 4){ 
      SD.remove("/HOURLY.csv");
      SD.remove("/TRACKING.csv");
    }

  // TRIGGER DONE PIN ON TPL
  Serial.println("TPLDONE");
  pinMode(A0, OUTPUT);
  digitalWrite(A0, LOW); delay(50); digitalWrite(A0, HIGH); delay(50);
  digitalWrite(A0, LOW); delay(50); digitalWrite(A0, HIGH); delay(50);
  digitalWrite(A0, LOW); delay(50); digitalWrite(A0, HIGH); delay(50);
  digitalWrite(A0, LOW); delay(50); digitalWrite(A0, HIGH); delay(50);

  delay(50);

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

  // digitalWrite(IridSlpPin, HIGH);  //Drive iridium power pin LOW
  delay(2000);
  Serial.println(" - Send_msg");

  IridiumSerial.begin(19200); // Start the serial port connected to the satellite modem

  int err = modem.begin();
  Serial.print(" - modem begin: "); Serial.println(err);

  modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);  // This is a low power application

  if (err == ISBD_IS_ASLEEP) {
    Serial.println(" - Modem asleep, wake up");
    err = modem.begin();
    Serial.print(" - modem begin: "); Serial.println(err);
  }

  Serial.println(" - Sending...");
  err = modem.sendSBDText(my_msg.c_str());
  Serial.print(" - Send response... "); Serial.println(err);
  
  if (err != ISBD_SUCCESS){ //} && err != 13) {

    Serial.println(" - Retry...");
    err = modem.begin();
    
    if (err == ISBD_IS_ASLEEP) {
      Serial.println(" - Modem asleep, wake up");
      err = modem.begin();
      Serial.print(" - modem begin: "); Serial.println(err);
      }

    // modem.adjustSendReceiveTimeout(300);
    Serial.println ("  - Sending...");
    err = modem.sendSBDText(my_msg.c_str());
    Serial.print("  - Send response... "); Serial.println(err);
    }
 
    struct tm t;
    int err_time = modem.getSystemTime(t);
    Serial.print("  - Sync clocks... "); Serial.println(err_time);
    if (err_time == ISBD_SUCCESS) {
      String pre_time = rtc.now().timestamp();
      Serial.print(" - Arduino Time: ");Serial.println(pre_time);
      rtc.adjust(DateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec));
      Serial.print(" - Arduino New Time: ");String post_time = rtc.now().timestamp();
      Serial.println(post_time);
  }
  
  // digitalWrite(IridPwrPin, LOW);  //Drive iridium power pin LOW
  modem.sleep();

  return err;
}

long sample_ultrasonic(){

  Serial.println("starting ultrasonic");
  pinMode(triggerPin, OUTPUT);       //Set ultrasonic ranging trigger pin as OUTPUT
  pinMode(pulsePin, INPUT);          //Set ultrasonic pulse width pin as INPUT
  float values[10];          //Array for storing sampled distances
  digitalWrite(triggerPin, HIGH);  //Write the ranging trigger pin HIGH
  delay(30);    
  for (int16_t i = 0; i < 10; i++)  //Take N samples
  {
    int32_t duration = pulseIn(pulsePin, HIGH);  //Get the pulse duration (i.e.,time of flight)
    // Serial.println(duration);
    values[i] = duration;
    delay(150);  //Dont sample too quickly < 7.5 Htz
  }
  long med_distance = stats.minimum(values, 10);  //Copute median distance
  digitalWrite(triggerPin, LOW);  //Write the ranging trigger pin HIGH
  return med_distance;
}

String sample_sht(){
  if (!sht31.begin(0x44)) { 
    blinky(5, 100, 100, 2000);  
    Serial.println("Couldn't find Temp/RH");
  }
  sht31.heater(0);  
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();
  return String(t) + "," + String(h);
  }
