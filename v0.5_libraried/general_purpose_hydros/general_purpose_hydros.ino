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
const byte chipSelect = 4;      // Chip select pin for SD card
const byte SensorSetPin = 5;    //Power relay set pin to HYDROS21
const byte SensorUnsetPin = 6;  //Power relay unset pin to HYDROS21
const byte led = 8;             // Built in led pin
const byte vbatPin = 9;         // Batt pin
const byte dataPin = 12;        // The pin of the SDI-12 data bus
const byte IridPwrPin = 13;     // Power base PN222 2 transistor pin to Iridium modem

/*Define global vars */
String my_letter = "ABC";
String my_header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";
int16_t *sample_freq_m;
uint16_t sample_freq_m_16;
int16_t *irid_freq_h;
uint16_t irid_freq_h_16;
char **test_mode;
String test_mode_string;
int16_t *onstart_samples;
uint16_t onstart_samples_16;

uint16_t blink_freq_s = 10;
uint16_t watchdog_timer = 30000;

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
WeatherStation ws;                // instance of WeatherStation class

String take_measurement() {

  digitalWrite(SensorSetPin, HIGH); delay(50);
  digitalWrite(SensorSetPin, LOW); delay(1000);
  
  String msmt = String(sample_batt_v()) + "," + 
    freeMemory() + "," + 
    sample_hydros_M();

  digitalWrite(SensorUnsetPin, HIGH); delay(50);
  digitalWrite(SensorUnsetPin, LOW); delay(50);

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
  float *out_water_level_mm = (float *)cp["water_level_mm"];
  float *out_water_temp_c = (float *)cp["water_temp_c"];
  float *out_water_ec_dcm = (float *)cp["water_ec_dcm"];
  
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
      String(round(out_water_level_mm[i])) + ',' + 
      String(round(out_water_temp_c[i]*10)) + ',' + 
      String(round(out_water_ec_dcm[i])) + ':';              
    }

  return datastring_msg;
}

void setup(void) {
    
  //set Irid power and LED pins  
  pinMode(13, OUTPUT); digitalWrite(13, LOW); delay(50); //Irid power pin (I think) - redundant, done below
  pinMode(led, OUTPUT); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50);
  
  //set SDI-12 data bus pin
  pinMode(dataPin, INPUT); 

  //sensor set and unset pins
  pinMode(SensorSetPin, OUTPUT); 
  digitalWrite(SensorSetPin, HIGH); delay(50);
  digitalWrite(SensorSetPin, LOW); delay(50);
  
  pinMode(SensorUnsetPin, OUTPUT);
  digitalWrite(SensorUnsetPin, HIGH); delay(50);
  digitalWrite(SensorUnsetPin, LOW); delay(50);
  
  //set irid power (done above - IridPwrPin = 13)
  pinMode(IridPwrPin, OUTPUT);
  digitalWrite(IridPwrPin, LOW); delay(50);

  // START SDI-12 PROTOCOL
  Serial.println(" - check sdi12"); //note Serial is used for communication btwn board and computer/other device - basic Arduino library
  mySDI12.begin();

  // CHECK RTC (time)
  Serial.println(" - check clock");
  while (!rtc.begin()) { ws.blinky(1, 200, 200, 2000); }

  // CHECK SD CARD
  Serial.println(" - check card");
  while (!SD.begin(chipSelect)) { ws.blinky(2, 200, 200, 2000); }

  // READ PARAMS
  ws.read_params();

  if (test_mode_string == "T") {

    SD.remove("/HOURLY.csv");

    delay(3000);
    Serial.begin(9600);
    Serial.println("###########################################");
    Serial.println("starting");

    Serial.println("check params");
    Serial.print(" - sample_freq_m_16: "); Serial.println(sample_freq_m_16);
    Serial.print(" - irid_freq_h_16: "); Serial.println(irid_freq_h_16);
    Serial.print(" - test_mode_string: "); Serial.println(test_mode_string);
    Serial.print(" - onstart_samples_16: "); Serial.println(onstart_samples_16);

    // CHECK SENSORS
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
      String datastring_start = rtc.now().timestamp() + "," + take_measurement();
      Serial.print(" - "); Serial.println(datastring_start);
      write_to_csv(my_header + ",comment", datastring_start + ",startup sample " + i, "/DATA.csv");
    }
  
    Serial.println("check irid");
    irid_test(datastring_start);
  
    SD.remove("/HOURLY.csv");

  };

  Serial.println("Awaiting delayed start ...");

  int countdownMS = Watchdog.enable(watchdog_timer); // Initialize watchdog (decay function that will reset the logger if it runs out)

}

void loop(void) {
  
  DateTime present_time = rtc.now(); // WAKE UP, WHAT TIME IS IT?
  
  // BLINK INTERVAL, THEN SLEEP
  if (present_time.second() % 10 == 0){
    ws.blinky(1, 20, 200, 200);
    
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
          int irid_err = send_msg(msg);
          SD.remove("/HOURLY.csv");

        }
      }
         
      write_to_csv(my_header + ",comment", present_time.timestamp() + "," + sample, "/DATA.csv");// SAMPLE - WRITE TO CSV
      Watchdog.disable();
      Watchdog.enable(100);
      delay(200); // TRIGGER WATCHDOG

    }
    
    DateTime sample_end = rtc.now();
    uint32_t sleep_time = ((blink_freq_s - (sample_end.second() % blink_freq_s)) * 1000.0) - 1000;
    LowPower.sleep(sleep_time);
    }
  
  Watchdog.reset();
  delay(500); // Half second to make sure we do not skip a second
}

/* Added to WeatherStations library */
/*
void blinky(int16_t n, int16_t high_ms, int16_t low_ms, int16_t btw_ms) {
  for (int i = 1; i <= n; i++) {
    digitalWrite(led, HIGH);
    delay(high_ms);
    digitalWrite(led, LOW);
    delay(low_ms);
  }
  delay(btw_ms);
}
*/

//same as for general_purpose_hydros_noSD and general_purpose_hydros_analite
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

/*
note to self: bunch of watchdog stuff in here that Alex said we don't need in new versions
need to understand how that works before putting into library
*/
int send_msg(String my_msg) {

  digitalWrite(IridPwrPin, HIGH);  //Drive iridium power pin LOW
  delay(2000);

  IridiumSerial.begin(19200);                            // Start the serial port connected to the satellite modem
  modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);  // This is a low power application
  //modem is instance of IridiumSBD

  Watchdog.disable();
  // Serial.print(" begin");
  int err = modem.begin(); 
  Watchdog.enable(watchdog_timer);

  if (err == ISBD_IS_ASLEEP) {
    Watchdog.disable();
    // Serial.print(" wake");
    err = modem.begin();
    Watchdog.enable(watchdog_timer);
  }

  Watchdog.disable();
  // modem.adjustSendReceiveTimeout(300);
  // Serial.print(" send");
  err = modem.sendSBDText(my_msg.c_str());
  Watchdog.enable(watchdog_timer);
  
  //if it doesn't send try again
  if (err != ISBD_SUCCESS){ //} && err != 13) {
    Watchdog.disable();
    // Serial.print(" retry");
    err = modem.begin();
    Watchdog.enable(watchdog_timer);
    // modem.adjustSendReceiveTimeout(300);
    // Serial.print(" send");
    Watchdog.disable();
    err = modem.sendSBDText(my_msg.c_str());
    Watchdog.enable(watchdog_timer);
    }

  //update local time (RTC) to time from Iridium
  // Serial.print(" time");
  if(rtc.now().hour() == 12 & rtc.now().day() % 5 == 0){
    struct tm t;
    int err_time = modem.getSystemTime(t);
    if (err_time == ISBD_SUCCESS) {
      String pre_time = rtc.now().timestamp();
      rtc.adjust(DateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec));
      String post_time = rtc.now().timestamp();
    }
  }
  
  digitalWrite(IridPwrPin, LOW);  //Drive iridium power pin LOW
  return err;
}

void irid_test(String msg) {

  pinMode(IridPwrPin, OUTPUT);     //Set iridium power pin as OUTPUT
  digitalWrite(IridPwrPin, HIGH);  //Drive iridium power pin LOW
  delay(2000);

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

  Serial.println("Sync clock to Iridium");
  struct tm t;
  int err_time = modem.getSystemTime(t);
  if (err_time == ISBD_SUCCESS) {
    String pre_time = rtc.now().timestamp();
    rtc.adjust(DateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec));
    String post_time = rtc.now().timestamp();
  }

  digitalWrite(IridPwrPin, LOW);  //Drive iridium power pin LOW
}

/* Added to WeatherStation library */
/*
void read_params() {

  CSV_Parser cp("ddsd", true, ',');
  Serial.println(" - check param.txt");
  while (!cp.readSDfile("/PARAM.txt")) { ws.blinky(3, 200, 200, 1000); }
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
*/
