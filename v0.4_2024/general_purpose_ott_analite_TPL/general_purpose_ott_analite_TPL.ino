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

/*Define global constants*/
const byte chipSelect = 4;      // Chip select pin for SD card
const byte SensorSetPin = 5;    //Power relay set pin to HYDROS21
const byte SensorUnsetPin = 6;  //Power relay unset pin to HYDROS21
const byte led = 8;             // Built in led pin
const byte vbatPin = 9;         // Batt pin
const byte WiperSetPin = 10;    //Power relay set pin to HYDROS21
const byte WiperUnsetPin = 11;  //Power relay unset pin to HYDROS21
const byte dataPin = 12;        // The pin of the SDI-12 data bus
const byte IridPwrPin = 13;     // Power base PN2222 transistor pin to Iridium modem
const byte TurbAlog = A1;       // Pin for reading analog outout from voltage divder (R1=1000 Ohm, R2=5000 Ohm) conncted to Analite

/*Define global vars */
String my_letter = "ABD";
String my_header = "datetime,batt_v,memory,water_level_mm,water_temp_c,ott_status,ott_rh,ott_dew,ott_deg,ntu";

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
IridiumSBD modem(IridiumSerial);  // Declare the IridiumSBD object
SDI12 mySDI12(dataPin);           // Define the SDI-12 bus
QuickStats stats;                 // Instance of QuickStats

String parseData(String dataString) {

  int tempSignIndex = dataString.indexOf('-', 1); // Start search after the first character

  if (tempSignIndex > 0 && dataString[tempSignIndex - 1] != ',') {

  dataString = dataString.substring(0, tempSignIndex) + "," + dataString.substring(tempSignIndex);
  }
  return dataString;
}

String take_measurement() {

  digitalWrite(SensorSetPin, HIGH); delay(50);
  digitalWrite(SensorSetPin, LOW); delay(1000);

  String msmt = String(sample_batt_v()) + "," + 
    freeMemory() + "," + 
    parseData(sample_ott_M()) + "," + 
    sample_ott_V() + "," + 
    sample_analite_195();

  digitalWrite(SensorUnsetPin, HIGH); delay(50);
  digitalWrite(SensorUnsetPin, LOW); delay(50);

  return msmt;
}

String prep_msg(){
  
  SD.begin(chipSelect);
  CSV_Parser cp("sfffffffff", true, ',');  // Set paramters for parsing the log file
  cp.readSDfile("/HOURLY.csv");
  int num_rows = cp.getRowsCount();  //Get # of rows
    
  char **out_datetimes = (char **)cp["datetime"];
  float *out_mem = (float *)cp["memory"];
  float *out_batt_v = (float *)cp["batt_v"];
  float *out_water_level_mm = (float *)cp["water_level_mm"];
  float *out_water_temp_c = (float *)cp["water_temp_c"];
  float *out_ntu = (float *)cp["ntu"];

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
      String(round(out_water_level_mm[i]*1000)) + ',' + 
      String(round(out_water_temp_c[i]*10)) + ',' + 
      String(round(out_ntu[i])) + ':';              
    }

  return datastring_msg;
}

void setup(void) {
 
  pinMode(13, OUTPUT); digitalWrite(13, LOW); delay(50);
  pinMode(led, OUTPUT); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50);
  
  pinMode(dataPin, INPUT); 

  pinMode(WiperSetPin, OUTPUT);  digitalWrite(WiperSetPin, HIGH); delay(50); digitalWrite(WiperSetPin, LOW); delay(50);
  pinMode(WiperUnsetPin, OUTPUT); digitalWrite(WiperUnsetPin, HIGH); delay(50); digitalWrite(WiperUnsetPin, LOW); delay(50); 
  pinMode(SensorSetPin, OUTPUT);  digitalWrite(SensorSetPin, HIGH); delay(50); digitalWrite(SensorSetPin, LOW); delay(50);
  pinMode(SensorUnsetPin, OUTPUT); digitalWrite(SensorUnsetPin, HIGH); delay(50); digitalWrite(SensorUnsetPin, LOW); delay(50);

  pinMode(A0, OUTPUT);
  digitalWrite(A0, LOW); delay(50);
  
  pinMode(IridPwrPin, OUTPUT);
  digitalWrite(IridPwrPin, LOW); delay(50);

  // START SDI-12 PROTOCOL
  Serial.println(" - check sdi12");
  mySDI12.begin();

  // CHECK RTC
  Serial.println(" - check clock");
  while (!rtc.begin()) { blinky(1, 200, 200, 2000); }

  // CHECK SD CARD
  Serial.println(" - check card");
  while (!SD.begin(chipSelect)) { blinky(2, 200, 200, 2000); }
  
  delay(100);
  Serial.begin(9600);

  }

void loop(void) {

  delay(100);
  Serial.println("###########################################");
    
  // READ TIME
  DateTime present_time = rtc.now();

  // TAKE MEASUREMENT
  String sample = take_measurement();
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
    
    Serial.println("wiper on, 20s");
    wiper_analite_195();

    // WRITE TO HOURLY
    Serial.print("Write to /HOURLY.csv = ");
    write_to_csv(my_header, present_time.timestamp() + "," + sample, "/HOURLY.csv");
    SD.begin(chipSelect);
    CSV_Parser cp("sfffffffff", true, ',');  // Set paramters for parsing the log file
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

String sample_hydros_M() {

  myCommand = String(SENSOR_ADDRESS) + "M!";  // first command to take a measurement

  mySDI12.sendCommand(myCommand);
  delay(30);  // wait a while for a response

  while (mySDI12.available()) {  // build response string
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);  // 1 character ~ 7.5ms
    }
  }

  /*Clear buffer*/
  if (sdiResponse.length() > 1)
    mySDI12.clearBuffer();

  delay(2000);       // delay between taking reading and requesting data
  sdiResponse = "";  // clear the response string

  // next command to request data from last measurement
  myCommand = String(SENSOR_ADDRESS) + "D0!";

  mySDI12.sendCommand(myCommand);
  delay(30);  // wait a while for a response

  while (mySDI12.available()) {  // build string from response
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);  // 1 character ~ 7.5ms
    }
  }

  sdiResponse = sdiResponse.substring(3);

  for (int i = 0; i < sdiResponse.length(); i++) {
    char c = sdiResponse.charAt(i);
    if (c == '+') {
      sdiResponse.setCharAt(i, ',');
    }
  }

  //clear buffer
  if (sdiResponse.length() > 1)
    mySDI12.clearBuffer();

  if (sdiResponse == "")
    sdiResponse = "-9,-9,-9";

  return sdiResponse;
}

void wiper_analite_195() {

  digitalWrite(SensorSetPin, HIGH); delay(50);
  digitalWrite(SensorSetPin, LOW); delay(1000);

  digitalWrite(WiperSetPin, HIGH); delay(50); delay(100);
  digitalWrite(WiperSetPin, LOW); delay(50); 
  digitalWrite(WiperUnsetPin, HIGH); delay(50); 
  digitalWrite(WiperUnsetPin, LOW); delay(50); 
  delay(20000);  // wait for full rotation (about 6 seconds)

  digitalWrite(SensorUnsetPin, HIGH); delay(50);
  digitalWrite(SensorUnsetPin, LOW); delay(50);

}

String sample_analite_195() {

  analogReadResolution(12);

  float values[10];  //Array for storing sampled distances

  for (int i = 0; i < 10; i++) {
    values[i] = (float)
    analogRead(TurbAlog);  // Read analog value from probe
    delay(5);
  }

  float med_turb_alog = stats.median(values, 10);  // Compute median 12-bit analog val

  //Convert analog value (0-4096) to NTU from provided linear calibration coefficients
  float ntu_analog = med_turb_alog;  //(m * med_turb_alog) + b;

  int ntu_int = round(ntu_analog);

  analogReadResolution(10);

  return String(ntu_int);
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

  digitalWrite(IridPwrPin, HIGH);  //Drive iridium power pin LOW
  delay(2000);
  Serial.println(" - Send_msg");

  IridiumSerial.begin(19200); // Start the serial port connected to the satellite modem

  int err = modem.begin();
  Serial.print(" - modem begin: "); Serial.println(err);

  modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);  // This is a low power application

  if (err == 10) {
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
  
  digitalWrite(IridPwrPin, LOW);  //Drive iridium power pin LOW
  return err;
}

String sample_ott_M(){

  myCommand = String(SENSOR_ADDRESS) + "M!";// first command to take a measurement

  mySDI12.sendCommand(myCommand);
  delay(30);  // wait a while for a response

  while (mySDI12.available()) {  // build response string
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);  // 1 character ~ 7.5ms
    }
  }

  /*Clear buffer*/
  if (sdiResponse.length() > 1)
    mySDI12.clearBuffer();

  delay(2000);       // delay between taking reading and requesting data
  sdiResponse = "";  // clear the response string


  // next command to request data from last measurement
  myCommand = String(SENSOR_ADDRESS) + "D0!";

  mySDI12.sendCommand(myCommand);
  delay(30);  // wait a while for a response

  while (mySDI12.available()) {  // build string from response
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);  // 1 character ~ 7.5ms
    }
  }

  //subset responce
  sdiResponse = sdiResponse.substring(3);

  for (int i = 0; i < sdiResponse.length(); i++)
  {

    char c = sdiResponse.charAt(i);

    if (c == '+')
    {
      sdiResponse.setCharAt(i, ',');
    }

  }

  //clear buffer
  if (sdiResponse.length() > 1)
    mySDI12.clearBuffer();

  if(sdiResponse == "")
    sdiResponse = "-9,-9,-9";
    
  return sdiResponse;
}

String sample_ott_V(){

  myCommand = String(SENSOR_ADDRESS) + "V!";// first command to take a measurement

  mySDI12.sendCommand(myCommand);
  delay(30);  // wait a while for a response

  while (mySDI12.available()) {  // build response string
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);  // 1 character ~ 7.5ms
    }
  }

  /*Clear buffer*/
  if (sdiResponse.length() > 1)
    mySDI12.clearBuffer();

  delay(2000);       // delay between taking reading and requesting data
  sdiResponse = "";  // clear the response string


  // next command to request data from last measurement
  myCommand = String(SENSOR_ADDRESS) + "D0!";

  mySDI12.sendCommand(myCommand);
  delay(30);  // wait a while for a response

  while (mySDI12.available()) {  // build string from response
    char c = mySDI12.read();
    if(c == '-'){c = (char) ',';}    
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);  // 1 character ~ 7.5ms
    }
  }

  //subset responce
  sdiResponse = sdiResponse.substring(3);

  for (int i = 0; i < sdiResponse.length(); i++)
  {

    char c = sdiResponse.charAt(i);

    if (c == '+')
    {
      sdiResponse.setCharAt(i, ',');
    }

  }

  //clear buffer
  if (sdiResponse.length() > 1)
    mySDI12.clearBuffer();
    
  if(sdiResponse == "")
    sdiResponse = "-9,-9,-9";

  return sdiResponse;
}
