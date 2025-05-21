// ONSERVATOR ADDRESS MUST BE SET TO 1
// HYDROS ADDRESS MUST BE SET TO 0
// OBSERVATOR GREEN = 12V GROUND
// OBSERVATOR BROWN = 12V POSITIVE
// OBSERVATOR WHITE = PIN 12 (DATA)
// CTD WHITE = 12V POSITIVE
// CTD RED = PIN 11 (DATA)
// CTD BLACK = 12V GROUND

/*Include the libraries we need*/
#include <time.h>
#include <RTClib.h>             //Needed for communication with Real Time Clock
#include <SPI.h>                //Needed for working with SD card
#include <SD.h>                 //Needed for working with SD card
#include <ArduinoLowPower.h>    //Needed for putting Feather M0 to sleep between samples
#include <IridiumSBD.h>         //Needed for communication with IRIDIUM modem
#include <CSV_Parser.h>         //Needed for parsing CSV data
#include <SDI12.h>              //Needed for SDI-12 communication
#include <QuickStats.h>         // Stats
#include <MemoryFree.h>         // https://github.com/mpflaga/Arduino-MemoryFree

// /*Define global constants*/
const byte chipSelect = 4;      // Chip select pin for SD card
const byte led = 8;             // Built in led pin
const byte vbatPin = 9;         // Batt pin
const byte dataPin_OBS = 12;        // The pin of the SDI-12 data bus for Observator
const byte dataPin_CTD = 11;        // The pin of the SDI-12 data bus for Observator
const byte IridSlpPin = 13;     // Power base PN222 2 transistor pin to Iridium modem

float cor_msmt_ctd_stage_m = 97.895;// surveyed stage from sensor at time of offset
float cor_msmt_ctd_wl_m = 0.783;// water level from sensor at time of offset
float cor_msmt_ctd_offset_m = cor_msmt_ctd_stage_m-cor_msmt_ctd_wl_m;// water level from sensor at time of offset

const byte irid_interval = 2; // Communication interval (in hours)
const byte irid_on = 1; // 1 = ON, 0 = OFF 

// /*Define global vars */
String my_letter = "ABCD";
String my_header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm,nep_temp,nep_med,nep_mean,nep_min,nep_max,cor_msmt_ctd_stage_m,cor_msmt_ctd_wl_m";
int err; 

String myCommand = "";    // SDI-12 command var
String sdiResponse = "";  // SDI-12 responce var

/*Define Iridium seriel communication as Serial1 */
#define IridiumSerial Serial1

/*SDI-12 sensor address, assumed to be 0*/
#define SENSOR_ADDRESS_OBS 1 // HAD TO RESET USING COMMAND 
#define SENSOR_ADDRESS_CTD 0

/*Create library instances*/
RTC_PCF8523 rtc;                  // Setup a PCF8523 Real Time Clock instance (may have to change this to more precise DS3231)
File dataFile;                    // Setup a log file instance
IridiumSBD modem(IridiumSerial, IridSlpPin);  // Declare the IridiumSBD object
SDI12 mySDI12_OBS(dataPin_OBS);
SDI12 mySDI12_CTD(dataPin_CTD);
QuickStats stats;                 // Instance of QuickStats

String parseData(String dataString) {

  int tempSignIndex = dataString.indexOf('-', 1); // Start search after the first character

  if (tempSignIndex > 0 && dataString[tempSignIndex - 1] != ',') {

  dataString = dataString.substring(0, tempSignIndex) + "," + dataString.substring(tempSignIndex);
  }
  return dataString;}

String take_measurement() {

  String wipeResult = sample_observator_M_wipe();
  
  String msmt = String(sample_batt_v()) + "," + 
    freeMemory() + "," + 
    parseData(sample_hydros_M()) + "," + 
    // sample_observator_M_single() + "," + 
    sample_observator_M6_statistical() + "," + 
    String(cor_msmt_ctd_stage_m) + "," + 
    String(cor_msmt_ctd_wl_m);  
    
  return msmt;}

String prep_msg(){
  
  SD.begin(chipSelect);
  CSV_Parser cp("sffffffffffff", true, ',');  // Set paramters for parsing the log file
  cp.readSDfile("/HOURLY.csv");
  int num_rows = cp.getRowsCount();  //Get # of rows
    
  char **out_datetimes = (char **)cp["datetime"];
  float *out_mem = (float *)cp["memory"];
  float *out_batt_v = (float *)cp["batt_v"];
  float *out_water_level_mm = (float *)cp["water_level_mm"];
  float *out_water_temp_c = (float *)cp["water_temp_c"];
  float *out_water_ec_dcm = (float *)cp["water_ec_dcm"];
  float *out_ntu_obs = (float *)cp["nep_mean"];
  
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
      String(round(out_water_level_mm[i]+(cor_msmt_ctd_offset_m*1000))) + ',' + 
      String(round(out_water_temp_c[i]*10)) + ',' + 
      String(round(out_water_ec_dcm[i])) + ',' + 
      String(round(out_ntu_obs[i])) + ':';              
    }

  return datastring_msg;}

void setup(void) {
 
  pinMode(13, OUTPUT); digitalWrite(13, LOW); delay(50);
  pinMode(led, OUTPUT); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50);
  
  pinMode(dataPin_OBS, INPUT); 
  pinMode(dataPin_CTD, INPUT); 
  
  pinMode(A0, OUTPUT);
  digitalWrite(A0, LOW); delay(50);
    
  pinMode(IridSlpPin, OUTPUT);
  digitalWrite(IridSlpPin, LOW); delay(50);
  modem.sleep();

  // START SDI-12 PROTOCOL
  Serial.println(" - check sdi12");
  mySDI12_OBS.begin();
  mySDI12_CTD.begin();
  
  // CHECK RTC
  Serial.println(" - check clock");
  while (!rtc.begin()) { blinky(1, 200, 200, 2000); }

  // CHECK SD CARD
  Serial.println(" - check card");
  while (!SD.begin(chipSelect)) { blinky(2, 200, 200, 2000); }
  
  delay(100);
  Serial.begin(9600);

  delay(10000);

  Serial.print("cor_msmt_ctd_stage_m: ");
  Serial.println(cor_msmt_ctd_stage_m);
  Serial.print("cor_msmt_ctd_wl_m: ");
  Serial.println(cor_msmt_ctd_wl_m);
  Serial.print("cor_msmt_ctd_offset_m: ");
  Serial.println(cor_msmt_ctd_offset_m);

  }

void loop(void) {

  delay(100);
  Serial.println("###########################################");
    
  // READ TIME
  DateTime present_time = rtc.now();

  // TAKE MEASUREMENT
  String sample = take_measurement();
  Serial.println(present_time.timestamp());
  Serial.println(my_header);
  Serial.print("Sample: ");
  Serial.println(sample);
  
  // WRITE TO DATA.csv
  Serial.println("Write to /DATA.csv");
  write_to_csv(my_header, present_time.timestamp() + "," + sample, "/DATA.csv");// SAMPLE - WRITE TO CSV

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
    CSV_Parser cp("sffffffffffff", true, ',');  // Set paramters for parsing the log file
    cp.readSDfile("/HOURLY.csv");
    int num_rows_hourly = cp.getRowsCount();  //Get # of rows minus header
    Serial.println(num_rows_hourly);
    
    // If HOURLY >= 2 rows, then send
    if(num_rows_hourly >= irid_interval & num_rows_hourly < 10){
      
      // PARSE MSG FROM HOURLY.csv
      Serial.print("Irid msg = ");
      String msg = prep_msg();
      Serial.println(msg);

      // SEND!
      if(irid_on == 1){
        int irid_err = send_msg(msg); 
        
        // IF SUCCESS, delete hourly (if not, will try again at next hourly interval)
        if(irid_err == 0){
          Serial.println("Message sent! Removing /HOURLY.csv");
          SD.remove("/HOURLY.csv");        
          Serial.println("Remove tracking");
          SD.remove("/TRACKING.csv");    
          }
      }      
      }

    // If HOURLY > 10 rows, then delete it! Probably too big to send
    if(num_rows_hourly >= 10){
      Serial.println(">10 remove hourly");
      SD.remove("/HOURLY.csv");        
      Serial.println("Remove tracking");
      SD.remove("/TRACKING.csv");}

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
  };

void blinky(int16_t n, int16_t high_ms, int16_t low_ms, int16_t btw_ms) {
  for (int i = 1; i <= n; i++) {
    digitalWrite(led, HIGH);
    delay(high_ms);
    digitalWrite(led, LOW);
    delay(low_ms);
  }
  delay(btw_ms);}

float sample_batt_v() {
  pinMode(vbatPin, INPUT);
  float batt_v = (analogRead(vbatPin) * 2 * 3.3) / 1024;
  return batt_v;}

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
  }}

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

  return err;}


String sample_observator_M_wipe() {
  
  Serial.println("wipe start");
  // Ensure both SDI12 objects are properly initialized
  mySDI12_OBS.begin();
  
  // Clear any previous data  // mySDI12_OBS.clearBuffer();
  
  // Create the command
  String myCommand = String(SENSOR_ADDRESS_OBS) + "M1!";
  
  // Send the wipe command
  mySDI12_OBS.sendCommand(myCommand);
  
  // Wait for wipe operation to complete
  delay(16000);
  
  // Clear buffer again // mySDI12_OBS.clearBuffer();
  
  mySDI12_OBS.end();
  
  Serial.println("wipe complete");

  return "wiped";
  };

String sample_observator_M_single() {

  // Ensure both SDI12 objects are properly initialized
  mySDI12_OBS.begin();
  
  // Clear any previous data
  mySDI12_OBS.clearBuffer();
  
  String sdiResponse = "";  // Local response string
  
  // Create and send the measurement command
  String myCommand = String(SENSOR_ADDRESS_OBS) + "M!";
  mySDI12_OBS.sendCommand(myCommand);
  delay(30);  // wait for initial response
  
  // Collect response to M6 command
  while (mySDI12_OBS.available()) {
    char c = mySDI12_OBS.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);
    }
  }
  
  Serial.println("OBS M command response: " + sdiResponse);
  
  // Clear buffer after reading response
  mySDI12_OBS.clearBuffer();
  
  // Wait for measurement to complete (this is a long measurement)
  Serial.println("waiting for response...");
  delay(10000);
  
  // Clear response string for data
  sdiResponse = "";
  
  // Request data
  myCommand = String(SENSOR_ADDRESS_OBS) + "D1!";
  // Serial.println(myCommand);
  mySDI12_OBS.sendCommand(myCommand);
  delay(30);
  
  // Collect data response
  while (mySDI12_OBS.available()) {
    char c = mySDI12_OBS.read();
    // Serial.println(c);
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);
    }
  }
  // Serial.println(sdiResponse.length());
  // Process response if valid
  if (sdiResponse.length() > 1) {
    // Remove address and first separator
    sdiResponse = sdiResponse.substring(2);
    // Serial.println(sdiResponse);
    // Replace + with commas for CSV format
    for (int i = 0; i < sdiResponse.length(); i++) {
      if (sdiResponse.charAt(i) == '+') {
        sdiResponse.setCharAt(i, ',');
      }
    }
  } else {
    sdiResponse = "Error: No valid data received";
  }
  
  // Final buffer clear
  mySDI12_OBS.clearBuffer();
  
  mySDI12_OBS.end();
  
  return sdiResponse;}

String sample_observator_M6_statistical() {

  // Ensure both SDI12 objects are properly initialized
  mySDI12_OBS.begin();
  
  // Clear any previous data
  mySDI12_OBS.clearBuffer();
  
  String sdiResponse = "";  // Local response string
  
  // Create and send the measurement command
  String myCommand = String(SENSOR_ADDRESS_OBS) + "M6!";
  mySDI12_OBS.sendCommand(myCommand);
  delay(30);  // wait for initial response
  
  // Collect response to M6 command
  while (mySDI12_OBS.available()) {
    char c = mySDI12_OBS.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);
    }
  }
  
  Serial.println("M6 command response: " + sdiResponse);
  
  // Clear buffer after reading response
  mySDI12_OBS.clearBuffer();
  
  // Wait for measurement to complete (this is a long measurement)
  Serial.println("Waiting for statistical measurement to complete...");
  delay(70000);
  
  // Clear response string for data
  sdiResponse = "";
  
  // Request data
  myCommand = String(SENSOR_ADDRESS_OBS) + "D1!";
  mySDI12_OBS.sendCommand(myCommand);
  delay(30);
  
  // Collect data response
  while (mySDI12_OBS.available()) {
    char c = mySDI12_OBS.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);
    }
  }
  
  // Process response if valid
  if (sdiResponse.length() > 2) {
    // Remove address and first separator
    sdiResponse = sdiResponse.substring(2);
    
    // Replace + with commas for CSV format
    for (int i = 0; i < sdiResponse.length(); i++) {
      if (sdiResponse.charAt(i) == '+') {
        sdiResponse.setCharAt(i, ',');
      }
    }
  } else {
    sdiResponse = "Error: No valid data received";
  }
  
  // Final buffer clear
  mySDI12_OBS.clearBuffer();
  
  mySDI12_OBS.end();
  
  return sdiResponse;}

String sample_hydros_M() {
  // Ensure both SDI12 objects are properly initialized
  mySDI12_CTD.begin();
  
  // Clear any previous data
  mySDI12_CTD.clearBuffer();
  
  String sdiResponse = "";  // Local response string
  
  // Create and send the measurement command
  String myCommand = String(SENSOR_ADDRESS_CTD) + "M!";
  mySDI12_CTD.sendCommand(myCommand);
  delay(30);
  
  // Collect response to M command
  while (mySDI12_CTD.available()) {
    char c = mySDI12_CTD.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);
    }
  }
  
  Serial.println("CTD M command response: " + sdiResponse);
  
  // Clear buffer after reading response
  mySDI12_CTD.clearBuffer();
  
  // Wait for measurement to complete
  delay(2000);
  
  // Clear response string for data
  sdiResponse = "";
  
  // Request data
  myCommand = String(SENSOR_ADDRESS_CTD) + "D0!";
  mySDI12_CTD.sendCommand(myCommand);
  delay(30);
  
  // Collect data response
  while (mySDI12_CTD.available()) {
    char c = mySDI12_CTD.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);
    }
  }
  
  // Process response if valid
  if (sdiResponse.length() > 3) {
    // Remove address and first separators
    sdiResponse = sdiResponse.substring(3);
    
    // Replace + with commas for CSV format
    for (int i = 0; i < sdiResponse.length(); i++) {
      if (sdiResponse.charAt(i) == '+') {
        sdiResponse.setCharAt(i, ',');
      }
    }
  } else {
    sdiResponse = "Error: No valid data received";
  }
  
  // Final buffer clear
  mySDI12_CTD.clearBuffer();
 
 mySDI12_CTD.end();
   
  return sdiResponse;}

