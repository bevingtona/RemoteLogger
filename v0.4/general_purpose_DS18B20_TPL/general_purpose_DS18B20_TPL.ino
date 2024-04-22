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
#include <OneWire.h> //Needed for oneWire communication 
#include <DallasTemperature.h> //Needed for communication with DS18B20

/*Define global constants*/
const byte chipSelect = 4;      // Chip select pin for SD card
const byte led = 8;             // Built in led pin
const byte vbatPin = 9;         // Batt pin
const byte dataPin = 11;        // The pin of the SDI-12 data bus
const byte IridPwrPin = 13;     // Power base PN222 2 transistor pin to Iridium modem

/*Define global vars */
String my_letter = "B";
String my_header = "datetime,batt_v,memory,water_temp_c";
int err;

/*Define Iridium seriel communication as Serial1 */
#define IridiumSerial Serial1

OneWire oneWire(dataPin);// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.

/*Create library instances*/
RTC_PCF8523 rtc;                  // Setup a PCF8523 Real Time Clock instance (may have to change this to more precise DS3231)
File dataFile;                    // Setup a log file instance
IridiumSBD modem(IridiumSerial);  // Declare the IridiumSBD object
QuickStats stats;                 // Instance of QuickStats

float sample_ds18b20(){

    sensors.requestTemperatures(); 

  //print the temperature in Celsius
  Serial.print("Temperature: ");
  Serial.print(sensors.getTempCByIndex(0));
  Serial.print("C  |  ");

  // sensors.setResolution(12);
  // sensors.requestTemperatures(); // Send the command to get temperatures
  // float tempC = sensors.getTempCByIndex(0);
  // while(tempC != DEVICE_DISCONNECTED_C)
  // {
  //   blinky(1000,1000,1000,1000);
  // }
  // return tempC;
}

String take_measurement() {

  String msmt = String(sample_batt_v()) + "," + 
    freeMemory() + "," + 
    sample_ds18b20();

  return msmt;
}

String prep_msg(){
  
  SD.begin(chipSelect);
  CSV_Parser cp("sffffffff", true, ',');  // Set paramters for parsing the log file
  cp.readSDfile("/HOURLY.csv");
  int num_rows = cp.getRowsCount();  //Get # of rows
    
  char **out_datetimes = (char **)cp["datetime"];
  float *out_mem = (float *)cp["memory"];
  float *out_batt_v = (float *)cp["batt_v"];
  float *out_water_level_mm = (float *)cp["water_level_mm"];
  float *out_water_temp_c = (float *)cp["water_temp_c"];
  
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
      String(round(out_water_temp_c[i]*10)) + ':'; // Serial.println(datastring_msg);
    }
  return datastring_msg;
}


void setup(void) {

  sensors.begin();
  Serial.begin(9600);

  // pinMode(13, OUTPUT); digitalWrite(13, LOW); delay(50);
  // pinMode(led, OUTPUT); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50);
  
  // Serial.println("a0");
  // pinMode(A0, OUTPUT);
  // digitalWrite(A0, LOW); delay(50); 

  // pinMode(dataPin, INPUT); 
  
  // Serial.println("ir");
  // pinMode(IridPwrPin, OUTPUT);
  // digitalWrite(IridPwrPin, LOW); delay(50);

  // // CHECK RTC
  // Serial.println(" - check clock");
  // while (!rtc.begin()) { blinky(1, 200, 200, 2000); }

  // // CHECK SD CARD
  // Serial.println(" - check card");
  // SD.begin(chipSelect);
  // while (!SD.begin(chipSelect)) { blinky(2, 200, 200, 2000); }

  // Serial.println("sen");
  
  // delay(100);

}

void loop(void) {

  sample_ds18b20();

//   delay(100);

//   Serial.println("###########################################");
    
//   // READ TIME
//   DateTime present_time = rtc.now();

//   // TAKE MEASUREMENT
//   String sample = take_measurement();
//   Serial.println(present_time.timestamp());
//   Serial.print("Sample: ");
//   Serial.println(sample);
  
//   // WRITE TO DATA.csv
//   Serial.println("Write to /DATA.csv");
//   write_to_csv(my_header + ",comment", present_time.timestamp() + "," + sample, "/DATA.csv");// SAMPLE - WRITE TO CSV

//   // INCREMENT TRACKING.csv to know how many samples have been taken
//   Serial.print("Add row to /TRACKING.csv = ");
//   write_to_csv("n", "1", "/TRACKING.csv");
//   CSV_Parser cp("s", true, ','); // Set paramters for parsing the tracking file ("s" = "String")
//   cp.readSDfile("/TRACKING.csv");
//   int num_rows_tracking = cp.getRowsCount()-1;  //Get # of rows minus header
//   Serial.println(num_rows_tracking);
//   blinky(num_rows_tracking,100,200,2000);
  
//   // HOW MANY SAMPLES UNTIL YOU WRITE TO HOURLY (for TPL at 15m, then this should be >=4)
//   if(num_rows_tracking == 4){ 
    
//     // WRITE TO HOURLY
//     Serial.print("Write to /HOURLY.csv = ");
//     write_to_csv(my_header, present_time.timestamp() + "," + sample, "/HOURLY.csv");
//     SD.begin(chipSelect);
//     CSV_Parser cp("sfffff", true, ',');  // Set paramters for parsing the log file
//     cp.readSDfile("/HOURLY.csv");
//     int num_rows_hourly = cp.getRowsCount();  //Get # of rows minus header
//     Serial.println(num_rows_hourly);
    
//     // If HOURLY >= 2 rows, then send
//     if(num_rows_hourly >= 4 & num_rows_hourly < 10){
      
//       // PARSE MSG FROM HOURLY.csv
//       Serial.print("Irid msg = ");
//       String msg = prep_msg();
//       Serial.println(msg);

//       // SEND!
//       int irid_err = send_msg(msg); 
      
//       // IF SUCCESS, delete hourly (if not, will try again at next hourly interval)
//       if(irid_err == 0){
//         Serial.println("Message sent! Removing /HOURLY.csv");
//         SD.remove("/HOURLY.csv");        
//         Serial.println("Remove tracking");
//         SD.remove("/TRACKING.csv");    
//         }      
//       }

//     // If HOURLY > 10 rows, then delete it! Probably too big to send
//     if(num_rows_hourly >= 10){
//       Serial.println(">10 remove hourly");
//       SD.remove("/HOURLY.csv");        
//       Serial.println("Remove tracking");
//       SD.remove("/TRACKING.csv");
// }

//     Serial.println("Remove tracking");
//     SD.remove("/TRACKING.csv");
//     }
    
//     if(num_rows_tracking > 4){ 
//       SD.remove("/HOURLY.csv");
//       SD.remove("/TRACKING.csv");
//     }

//   // TRIGGER DONE PIN ON TPL
//   Serial.println("TPLDONE");
//   pinMode(A0, OUTPUT);
//   digitalWrite(A0, LOW); delay(50); digitalWrite(A0, HIGH); delay(50);
//   digitalWrite(A0, LOW); delay(50); digitalWrite(A0, HIGH); delay(50);
//   digitalWrite(A0, LOW); delay(50); digitalWrite(A0, HIGH); delay(50);
//   digitalWrite(A0, LOW); delay(50); digitalWrite(A0, HIGH); delay(50);

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
    
    if (err == 10) {
      Serial.println(" - Modem asleep, wake up");
      err = modem.begin();
      Serial.print(" - modem begin: "); Serial.println(err);
      }

    modem.adjustSendReceiveTimeout(300);
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
