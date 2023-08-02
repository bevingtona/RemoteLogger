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

/*Define global constants*/
const byte chipSelect = 4;      // Chip select pin for SD card
const byte SensorSetPin = 5;    //Power relay set pin to HYDROS21
const byte SensorUnsetPin = 6;  //Power relay unset pin to HYDROS21
const byte led = 8;             // Built in led pin
const byte vbatPin = 9;         // Batt pin
const byte dataPin = 12;        // The pin of the SDI-12 data bus
const byte IridPwrPin = 13;     // Power base PN2222 transistor pin to Iridium modem

/*Define global vars */
String my_header = "datetime,water_level_mm,water_temp_c,water_ec_dcm,batt_v,memory";
int16_t *sample_freq_m;
uint16_t sample_freq_m_16;
int16_t *irid_freq_h;
uint16_t irid_freq_h_16;
char **onstart_irid;
String onstart_irid_string;
int16_t *onstart_samples;
uint16_t onstart_samples_16;
  
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

String sample_batt_v() {
  pinMode(vbatPin, INPUT);
  String batt_v = String((analogRead(vbatPin) * 2 * 3.3) / 1024);
  return batt_v;
}

String take_measurement() {
  digitalWrite(SensorSetPin, HIGH);
  delay(50);
  digitalWrite(SensorSetPin, LOW);
  delay(1000);
  String msmt = sample_hydros_M() + "," + sample_batt_v() + "," + freeMemory();
  digitalWrite(SensorUnsetPin, HIGH);
  delay(50);
  digitalWrite(SensorUnsetPin, LOW);
  delay(50);
  return msmt;
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
  
  digitalWrite(IridPwrPin, HIGH);  //Drive iridium power pin LOW
  delay(2000);
  
  IridiumSerial.begin(19200);                            // Start the serial port connected to the satellite modem
  modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);  // This is a low power application
  
  int err = modem.begin();
  write_to_csv("log", rtc.now().timestamp() + " begin " + err, "iridlog.txt");
  // Serial.println(("begin");
  
  if (err == 10){
    err = modem.begin();
    write_to_csv("log", rtc.now().timestamp() + " begin retry " + err, "iridlog.txt");
    // Serial.println(("retry");
    }  
  
  err = modem.sendSBDText(my_msg.c_str());
  // Serial.println(("sent");
  write_to_csv("log", rtc.now().timestamp() + " sent " + err, "iridlog.txt");
  // Serial.println((err);

  if (err != ISBD_SUCCESS && err != 13){
    err = modem.begin();
    write_to_csv("log", rtc.now().timestamp() + " begin2 " + err, "iridlog.txt");
    // Serial.println(("begin 2");
    
    modem.adjustSendReceiveTimeout(500);
    err = modem.sendSBDText(my_msg.c_str());
    write_to_csv("log", rtc.now().timestamp() + " sent2 " + err, "iridlog.txt");
    // Serial.println(("sent 2");
  }

  if (rtc.now().day() % 1 == 0 & 
    rtc.now().hour() == 12){
    struct tm t;
    int err_time = modem.getSystemTime(t);
    if (err_time == ISBD_SUCCESS){
      String pre_time = rtc.now().timestamp();
      rtc.adjust(DateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec));
      String post_time = rtc.now().timestamp();
      write_to_csv("pre_sync,post_sync,err_time",pre_time+","+post_time+","+err_time,"/IRID.csv");
      // Serial.println(("sync clock");
      }
    }

  digitalWrite(IridPwrPin, LOW);  //Drive iridium power pin LOW
  return err;
}

void irid_test(String msg) {

  pinMode(IridPwrPin, OUTPUT);     //Set iridium power pin as OUTPUT
  digitalWrite(IridPwrPin, HIGH);   //Drive iridium power pin LOW
  delay(2000);
  
  int signalQuality = -1;
  
  IridiumSerial.begin(19200);  // Start the serial port connected to the satellite modem
  modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE); // This is a low power application

  // Begin satellite modem operation
  Serial.println(" - starting modem...");
  int err = modem.begin();

  if (err != ISBD_SUCCESS)
  {
    Serial.print(" - begin failed: error ");
    Serial.println(err);
    if (err == ISBD_NO_MODEM_DETECTED)
      Serial.println(" - no modem detected: check wiring.");
    return;
  }

  // Example: Print the firmware revision
  char version[12];
  err = modem.getFirmwareVersion(version, sizeof(version));
  if (err != ISBD_SUCCESS)
  {
     Serial.print(" - firmware version failed: error ");
     Serial.println(err);
     return;
  }

  Serial.print(" - firmware version is "); Serial.print(version); Serial.println(".");

  int n = 0; 
  while(n < 10){
    err = modem.getSignalQuality(signalQuality);
    if (err != ISBD_SUCCESS)
    {
      Serial.print(" - signalQuality failed: error ");
      Serial.println(err);
      return;
    }

    Serial.print(" - signal quality is currently "); Serial.print(signalQuality); Serial.println(".");
    n = n + 1;  
    delay(1000);
  }

  // Send the message
  Serial.print(" - Attempting: ");
  msg = "Hello world! " + msg;
  Serial.println(msg);  

  err = modem.sendSBDText(msg.c_str());
  
  if (err != ISBD_SUCCESS)
  {
    Serial.print(" - sendSBDText failed: error ");
    Serial.println(err);
    if (err == ISBD_SENDRECEIVE_TIMEOUT)
      Serial.println(" - try again with a better view of the sky.");
  }
  else
  {
    Serial.println(" - hey, it worked!");
  }
  digitalWrite(IridPwrPin, LOW);   //Drive iridium power pin LOW
}

void read_params(){

  CSV_Parser cp("ddsd", true, ','); 
  Serial.println(" - check param.txt"); while (!cp.readSDfile("/PARAM.txt")) { blinky(3, 200, 200, 1000); }
  cp.parseLeftover();

  sample_freq_m = (int16_t *)cp["sample_freq_m"];
  sample_freq_m_16 = sample_freq_m[0];
  irid_freq_h = (int16_t *)cp["irid_freq_h"];
  irid_freq_h_16 = irid_freq_h[0];
  onstart_irid = (char **)cp["onstart_irid"];
  onstart_irid_string = String(onstart_irid[0]);
  onstart_samples = (int16_t *)cp["onstart_samples"];
  onstart_samples_16 = onstart_samples[0];

  delete sample_freq_m;
  delete irid_freq_h;
  delete onstart_irid;
  delete onstart_samples;
}

void setup(void) {

  delay(3000);
  Serial.begin(9600);
  Serial.println("###########################################");
  Serial.println("starting");

  // SET PINS
  pinMode(led, OUTPUT); digitalWrite(led, LOW); delay(50);
  pinMode(SensorSetPin, OUTPUT); digitalWrite(SensorSetPin, HIGH); delay(50); digitalWrite(SensorSetPin, LOW); delay(50);
  pinMode(SensorUnsetPin, OUTPUT); digitalWrite(SensorUnsetPin, HIGH); delay(50); digitalWrite(SensorUnsetPin, LOW); delay(50);
  pinMode(IridPwrPin, OUTPUT);  digitalWrite(IridPwrPin, LOW); delay(50);

  // START SDI-12 PROTOCOL
  Serial.println(" - check sdi12"); mySDI12.begin();

  // CHECK RTC
  Serial.println(" - check clock"); while (!rtc.begin()) { blinky(1, 200, 200, 2000); }

  // CHECK SD CARD
  Serial.println(" - check card"); while (!SD.begin(chipSelect)) { blinky(2, 200, 200, 2000); }

  // READ PARAMS
  read_params();
  Serial.println("check params");
  Serial.print(" - sample_freq_m_16: "); Serial.println(sample_freq_m_16);
  Serial.print(" - irid_freq_h_16: "); Serial.println(irid_freq_h_16);
  Serial.print(" - onstart_irid_string: "); Serial.println(onstart_irid_string);
  Serial.print(" - onstart_samples_16: "); Serial.println(onstart_samples_16);
  
  // CHECK SENSORS
  Serial.println("check sensors");
  String datastring_start = rtc.now().timestamp() + "," + take_measurement(); 
  Serial.print(" - "); Serial.println(datastring_start);
  write_to_csv(my_header + ",comment", datastring_start + ",startup", "/DATA.csv");
  
  // SAMPLE ON STARTUP - SEND MESSAGE
  if(onstart_irid_string == "T"){
    Serial.println("check irid");
    irid_test(datastring_start);
    };  

  // ONSTART SAMPLES
  Serial.println("check onstart samples");  
  Serial.print(" - "); Serial.println(my_header);
  for (int i = 0; i < onstart_samples_16; i++){  
    datastring_start = rtc.now().timestamp() + "," + take_measurement(); 
    Serial.print(" - "); Serial.println(datastring_start);
    write_to_csv(my_header + ",comment", datastring_start + ", startup sample " + i, "/DATA.csv");
  }
  Serial.println("Awaiting delayed start ...");
}

void loop(void) {

  // WAKE UP, WHAT TIME?
  DateTime present_time = rtc.now();
  
  // BLINK IF INTERVAL OF 10s
  if (present_time.second() % 10 == 0) {
    blinky(1, 20, 200, 200);
    
    // SAMPLE IF AT INTERVAL AND ON 0s
    if (present_time.minute() % sample_freq_m_16 == 0 & present_time.second() == 0) {
      
      // SAMPLE - SAMPLE
      String sample = take_measurement();
      String datastring = present_time.timestamp() + "," + sample;
      String comment = "logging";
      
      // SAVE TO HOURLY IF ON THE HOUR
      if (present_time.minute() == 0 & present_time.hour() % 2 == 0) {

        digitalWrite(SensorSetPin, HIGH);
        delay(50);
        digitalWrite(SensorSetPin, LOW);
        delay(1000);
        
        String msg = "ABC:" + 
                     present_time.timestamp().substring(2, 4) + 
                     present_time.timestamp().substring(5, 7) + 
                     present_time.timestamp().substring(8, 10) + 
                     present_time.timestamp().substring(11, 13) + ":" +
                     sample_batt_v() + ":" +
                     freeMemory() + ":" +
                     sample_hydros_M() + ":";

        digitalWrite(SensorUnsetPin, HIGH);
        delay(50);
        digitalWrite(SensorUnsetPin, LOW);
        delay(50);
        int irid_err = send_msg(msg);
        comment = "iridium " + irid_err; 
      }

      // SAMPLE - WRITE TO CSV
      write_to_csv(my_header + ",comment", datastring + "," + comment, "/DATA.csv");
    }
    
    DateTime sample_end = rtc.now();
    uint32_t sleep_time = ((10 - (sample_end.second() % 10)) * 1000.0) - 1000;
    // delay(sleep_time);
    LowPower.deepSleep(sleep_time);
  }
  delay(500);
}

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