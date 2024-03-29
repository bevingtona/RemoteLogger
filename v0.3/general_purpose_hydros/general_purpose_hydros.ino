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
int16_t *sample_freq_m;
uint16_t sample_freq_m_16;
int16_t *irid_freq_h;
uint16_t irid_freq_h_16;
char **onstart_irid;
String onstart_irid_string;
int16_t *onstart_samples;
uint16_t onstart_samples_16;
uint16_t blink_freq_s = 10;
uint16_t watchdog_timer = 30000;
int delimiter_1;
int delimiter_2;
float water_level_mm;
float water_temp_c;
float water_ec_dcm;

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

String take_measurement() {

  digitalWrite(SensorSetPin, HIGH); delay(50);
  digitalWrite(SensorSetPin, LOW); delay(1000);

  // IF HYDROS
  String hydrosString = sample_hydros_M();
  int delimeter_1 = hydrosString.indexOf(",");
  int delimeter_2 = hydrosString.indexOf(",", delimeter_1 + 1);
  water_level_mm = hydrosString.substring(0, delimeter_1).toFloat();
  water_temp_c = hydrosString.substring(delimeter_1+1, delimeter_2).toFloat();
  water_ec_dcm = round(hydrosString.substring(delimeter_2+1, hydrosString.length()).toFloat());
  
  String msmt = String(sample_batt_v()) + "," + 
    freeMemory() + "," + 
    water_level_mm + "," + 
    water_temp_c + "," + 
    water_ec_dcm;

  digitalWrite(SensorUnsetPin, HIGH); delay(50);
  digitalWrite(SensorUnsetPin, LOW); delay(50);

  return msmt;
}

String prep_msg(DateTime present_time) {

  String out = my_letter + ":" + 
    present_time.timestamp().substring(2, 4) + 
    present_time.timestamp().substring(5, 7) + 
    present_time.timestamp().substring(8, 10) + 
    present_time.timestamp().substring(11, 13) + ":" + 
    String(round(sample_batt_v()*100)) + "," + 
    round(freeMemory()/100) + "," + 
    round(water_level_mm) + "," + 
    round(water_temp_c*10) + "," + 
    round(water_ec_dcm) + ":";
  
  return out;
}

void setup(void) {
  
  delay(3000);
  Serial.begin(9600);
  Serial.println("###########################################");
  Serial.println("starting");

  for(int i=5; i<=14; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  
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

  // CHECK SD CARD
  Serial.println(" - check card");
  while (!SD.begin(chipSelect)) { blinky(2, 200, 200, 2000); }

  // READ PARAMS
  read_params();
  Serial.println("check params");
  Serial.print(" - sample_freq_m_16: "); Serial.println(sample_freq_m_16);
  Serial.print(" - irid_freq_h_16: "); Serial.println(irid_freq_h_16);
  Serial.print(" - onstart_irid_string: "); Serial.println(onstart_irid_string);
  Serial.print(" - onstart_samples_16: "); Serial.println(onstart_samples_16);

  // CHECK SENSORS
  // Serial.println("check sensors");
  // take_measurement();
  // String datastring_start = rtc.now().timestamp() + "," + take_measurement();
  // Serial.print(" - "); Serial.println(datastring_start);
  // Serial.print(" - "); Serial.println(prep_msg(rtc.now()));
  // write_to_csv(my_header + ",comment", datastring_start + ",startup", "/DATA.csv");

  // ONSTART SAMPLES
  Serial.println("check onstart samples");
  Serial.print(" - "); Serial.println(my_header);
  for (int i = 0; i < onstart_samples_16; i++) {
    String datastring_start = rtc.now().timestamp() + "," + take_measurement();
    Serial.print(" - "); Serial.println(datastring_start);
    write_to_csv(my_header + ",comment", datastring_start + ", startup sample " + i, "/DATA.csv");
  }

  // SAMPLE ON STARTUP - SEND MESSAGE
  if (onstart_irid_string == "T") {
    String datastring_start = rtc.now().timestamp() + "," + take_measurement();
    Serial.println("check irid");
    irid_test(datastring_start);
  };

  Serial.println("Awaiting delayed start ...");

  int countdownMS = Watchdog.enable(watchdog_timer); // Initialize watchdog (decay function that will reset the logger if it runs out)

}

void loop(void) {
  
  DateTime present_time = rtc.now(); // WAKE UP, WHAT TIME IS IT?
  
  if (present_time.second() % 10 == 0) // BLINK INTERVAL, THEN DEEP SLEEP
  {

    blinky(1, 20, 200, 200);
    
    if (present_time.minute() % sample_freq_m_16 == 0 & present_time.second() == 0) // SAMPLE IF AT INTERVAL AND ON 0s
    {
      
      String sample = take_measurement();
      Watchdog.reset();
      
      if (present_time.minute() == 0 & present_time.hour() % irid_freq_h_16 == 0) { // SEND MESSAGE AT INTERVAL

        String msg = prep_msg(present_time);
        int irid_err = send_msg(msg);
        }
         
      write_to_csv(my_header, present_time.timestamp() + "," + sample, "/DATA.csv");// SAMPLE - WRITE TO CSV
      Watchdog.reset(); 

      Watchdog.disable();
      Watchdog.enable(1000);
      delay(1500); // TRIGGER WATCHDOG

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
    struct tm t;
    int err_time = modem.getSystemTime(t);
    if (err_time == ISBD_SUCCESS) {
      String pre_time = rtc.now().timestamp();
      // Serial.print(pre_time);
      // Serial.print(" ");
      rtc.adjust(DateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec));
      String post_time = rtc.now().timestamp();
      // Serial.print(post_time);
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
  digitalWrite(IridPwrPin, LOW);  //Drive iridium power pin LOW
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
  onstart_irid = (char **)cp["onstart_irid"];
  onstart_irid_string = String(onstart_irid[0]);
  onstart_samples = (int16_t *)cp["onstart_samples"];
  onstart_samples_16 = onstart_samples[0];

  delete sample_freq_m;
  delete irid_freq_h;
  delete onstart_irid;
  delete onstart_samples;
}
