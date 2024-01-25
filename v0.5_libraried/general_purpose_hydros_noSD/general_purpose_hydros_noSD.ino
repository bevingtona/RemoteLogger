/**
 * January 25, 2024: partially converted to use WeatherStation library, just needs names changed to be converted (library contains all necessary functionality)
*/

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
const byte IridPwrPin = 13;     // Power base PN2222 transistor pin to Iridium modem

/*Define global vars */
String my_letter = "ABC";
String my_header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";

uint16_t blink_freq_s = 10;
uint16_t watchdog_timer = 30000;

String myCommand = "";    // SDI-12 command var
String sdiResponse = "";  // SDI-12 responce var

/*Define Iridium seriel communication as Serial1 */
#define IridiumSerial Serial1

/*SDI-12 sensor address, assumed to be 0*/
#define SENSOR_ADDRESS 0

/*Create library instances*/
WeatherStation ws(my_letter, my_header);
RTC_PCF8523 rtc;                  // Setup a PCF8523 Real Time Clock instance (may have to change this to more precise DS3231)
File dataFile;                    // Setup a log file instance
IridiumSBD modem(IridiumSerial);  // Declare the IridiumSBD object
SDI12 mySDI12(dataPin);           // Define the SDI-12 bus
QuickStats stats;                 // Instance of QuickStats

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

void setup(void) {
    
  pinMode(13, OUTPUT); digitalWrite(13, LOW); delay(50);
  pinMode(led, OUTPUT); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50);
  
  pinMode(dataPin, INPUT); 

  pinMode(SensorSetPin, OUTPUT); 
  digitalWrite(SensorSetPin, HIGH); delay(50);
  digitalWrite(SensorSetPin, LOW); delay(50);
  
  pinMode(SensorUnsetPin, OUTPUT);
  digitalWrite(SensorUnsetPin, HIGH); delay(50);
  digitalWrite(SensorUnsetPin, LOW); delay(50);
  
  pinMode(IridPwrPin, OUTPUT);
  digitalWrite(IridPwrPin, LOW); delay(50);

  // START SDI-12 PROTOCOL
  Serial.println(" - check sdi12");
  mySDI12.begin();

  // CHECK RTC
  Serial.println(" - check clock");
  while (!rtc.begin()) { blinky(1, 200, 200, 2000); }

  // if (test_mode_string == "T") {

    // delay(3000);
    // Serial.begin(9600);
    // delay(5000);
    // Serial.println("###########################################");
    // Serial.println("starting");

    // // CHECK SENSORS
    // Serial.println("check sensors");
    // String datastring_start = rtc.now().timestamp() + "," + take_measurement();
    // Serial.print(" - "); Serial.println(datastring_start);

    // // ONSTART SAMPLES
    // Serial.println("check onstart samples");
    // Serial.print(" - "); Serial.println(my_header);
    // for (int i = 0; i < 5; i++) {
    //   String datastring_start = rtc.now().timestamp() + "," + take_measurement();
    //   Serial.print(" - "); Serial.println(datastring_start);
    // }
  
    // Serial.println("check irid");
    // irid_test(datastring_start);
  
  // };

  // Serial.println("Awaiting delayed start ...");

  int countdownMS = Watchdog.enable(watchdog_timer); // Initialize watchdog (decay function that will reset the logger if it runs out)

}

void loop(void) {
  
  DateTime present_time = rtc.now(); // WAKE UP, WHAT TIME IS IT?
  
  // BLINK INTERVAL, THEN SLEEP
  if (present_time.second() % 10 == 0){
    blinky(1, 20, 200, 200);
    
    // TAKE A SAMPLE AT INTERVAL 
    if (present_time.minute() == 0){ 
      
      String sample = take_measurement();
      Watchdog.reset();
      
      // SEND MESSAGE
      String msg = "Hello world! " + present_time.timestamp() + "," + sample;
      int irid_err = send_msg(msg);
         
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

void blinky(int16_t n, int16_t high_ms, int16_t low_ms, int16_t btw_ms) {
  for (int i = 1; i <= n; i++) {
    digitalWrite(led, HIGH);
    delay(high_ms);
    digitalWrite(led, LOW);
    delay(low_ms);
  }
  delay(btw_ms);
}

//same as for general_purpose_hydros and general_purpose_hydros_analite
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

int send_msg(String my_msg) {

  digitalWrite(IridPwrPin, HIGH);  //Drive iridium power pin LOW
  delay(2000);

  IridiumSerial.begin(19200);                            // Start the serial port connected to the satellite modem
  modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);  // This is a low power application

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