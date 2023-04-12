// READ PARAM
// sample_freq_m,irid_freq_h,onstart_irid
// 10,12,T
// LOG FILE
// COMPILE IRID MSG

/*Include the libraries we need*/
#include "RTClib.h" //Needed for communication with Real Time Clock
#include <SPI.h>//Needed for working with SD card
#include <SD.h>//Needed for working with SD card
#include <ArduinoLowPower.h>//Needed for putting Feather M0 to sleep between samples
#include <IridiumSBD.h>//Needed for communication with IRIDIUM modem 
#include <CSV_Parser.h>//Needed for parsing CSV data
#include <SDI12.h>//Needed for SDI-12 communication
#include <QuickStats.h>//Needed for computing medians

/*Define global constants*/
const byte led = 8; // Built in GREEN LED pin (13 conflicts with irid)
const byte chipSelect = 4; // For SD card
const byte IridPwrPin = 13; // Pwr pin to Iridium modem // THIS IS ALSO THE PIN FOR THE RED LED
const byte SensorSetPin = 5; //Pwr set pin to HYDROS21
const byte SensorUnsetPin = 6; //Pwr unset pin to HYDROS21
const byte dataPin = 12; // The pin of the SDI-12 data bus
const byte WiperSetPin = 10; // Pin to set pwr relay to Analite 195
const byte WiperUnsetPin = 11; // Pin to unset pwr relay to Analite 195
const byte TurbAlog = A1; // Pin for reading analog outout from voltage divder (R1=1000 Ohm, R2=5000 Ohm) conncted to Analite 195
const byte vbatPin = 9;

/*Define global vars */
String my_header = "datetime,ntu,water_level_m,water_temp_c,ott_status,ott_rh,ott_dew,ott_deg,batt_v";
char **filename; //Name of log file
char **start_time; //Time at which to begin logging
String filestr; //Filename as string
int16_t *sample_intvl; //Sample interval in minutes
int16_t *irid_freq; //User provided transmission interval for Iridium modem in hours
float *turb_slope; //The linear slope parameter for converting 12-bit analog value to NTU, i.e., from calibration
float m;
float b;
float *turb_intercept; //The intercept parameter for converting 12-bit analog to NTU, i.e., form calibration
int wiper_cnt = 0;
uint32_t irid_freq_hrs;
uint32_t sleep_time;//Logger sleep time in milliseconds
DateTime transmit_time;//Datetime varible for keeping IRIDIUM transmit time
DateTime present_time;//Var for keeping the current time
int err; //IRIDIUM status var
String myCommand   = "";//SDI-12 command var
String sdiResponse = "";//SDI-12 responce var

/*Define Iridium seriel communication COM1*/
#define IridiumSerial Serial1

/*SDI-12 sensor address, assumed to be 0*/
#define SENSOR_ADDRESS 0

/*Create library instances*/
RTC_PCF8523 rtc; // Setup a PCF8523 Real Time Clock instance
File dataFile; // Setup a log file instance
IridiumSBD modem(IridiumSerial); // Declare the IridiumSBD object
SDI12 mySDI12(dataPin);// Define the SDI-12 bus
QuickStats stats;//Instance of QuickStats

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

  return sdiResponse;
}

int sample_analite_195(){

  float values[100]; //Array for storing sampled distances

  if (wiper_cnt >= 5)  {  // Probe will wipe after 6 power cycles (1 hr at 10 min interval)
    digitalWrite(WiperSetPin, HIGH); delay(20);    
    digitalWrite(WiperSetPin, LOW); delay(20); delay(100); // >50 ms burst of power activates the wiper's full rotation
    digitalWrite(WiperUnsetPin, HIGH); delay(20);    
    digitalWrite(WiperUnsetPin, LOW); delay(20);    
    wiper_cnt = 0; // Reset wiper count to zero
    delay(10000); // wait for full rotation (about 6 seconds)
  } else {
    wiper_cnt++;
    digitalWrite(WiperUnsetPin, HIGH); delay(20); // Unnecessary, but good to double check it's off 
    digitalWrite(WiperUnsetPin, LOW); delay(20);    
  }
  for(int i = 0; i<100; i++)
  {
    values[i]= (float) analogRead(TurbAlog); // Read analog value from probe
    delay(5);
  }

  float med_turb_alog = stats.median(values, 100); // Compute median 12-bit analog val

  //Convert analog value (0-4096) to NTU from provided linear calibration coefficients
  float ntu_analog = med_turb_alog; //(m * med_turb_alog) + b;

  int ntu_int = round(ntu_analog);

  return ntu_int;
}

float blinky(int16_t n, int16_t high_ms, int16_t low_ms, int16_t btw_ms){
  for(int i = 1; i <= n; i++) {  
    digitalWrite(led, HIGH);
    delay(high_ms);
    digitalWrite(led, LOW);
    delay(low_ms);}
  delay(btw_ms);  
  return 1;
}

String sample_batt_v(){
    pinMode(vbatPin, INPUT);
    String batt_v = String((analogRead(vbatPin)*2*3.3)/1024);
    return batt_v;
}

float write_to_csv(String header, String datastring_for_csv, String outname) {

  // IF FILE DOES NOT EXIST, WRITE HEADER AND DATA, ELSE, WITE DATA
  if (!SD.exists(outname))  //Write header if first time writing to the logfile
  {
    dataFile = SD.open(outname, FILE_WRITE);  //Open file under filestr name from parameter file
    if (dataFile) {
      dataFile.println(header+",comment");
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
  return 1;
}

float send_msg(String msg, float timeout_s, float retry_n){

  pinMode(IridPwrPin, OUTPUT);     //Set iridium power pin as OUTPUT
  digitalWrite(IridPwrPin, HIGH);   //Drive iridium power pin LOW
  delay(2000);
  int err;
  
  IridiumSerial.begin(19200);  // Start the serial port connected to the satellite modem
  modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE); // This is a low power application
  err = modem.begin();
  write_to_csv("datetime,comment", rtc.now().timestamp()+",Begin "+String(err), "/log.csv");

  modem.adjustSendReceiveTimeout(timeout_s);
  err = modem.sendSBDText(msg.c_str());  
  write_to_csv("datetime,comment", rtc.now().timestamp()+",Message "+String(err), "/log.csv");

  int signalQuality = -1;
  err = modem.getSignalQuality(signalQuality);
  write_to_csv("datetime,comment", rtc.now().timestamp()+",Quality "+String(err), "/log.csv");

  if(retry_n > 0){
  for (int16_t i = 0; i < retry_n; i++) {
    if(err != ISBD_SUCCESS)//err != 0 && err != 13)// If first attemped failed try once more with extended timeout
      {
        delay(1000);        
        write_to_csv("datetime,comment", rtc.now().timestamp()+",Retry "+String(i), "/log.csv");
        modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE); // This is a low power application
        err = modem.begin(); 
        write_to_csv("datetime,comment", rtc.now().timestamp()+",Begin "+String(err), "/log.csv");
        err = modem.sendSBDText(msg.c_str());
        write_to_csv("datetime,comment", rtc.now().timestamp()+",Message "+String(err), "/log.csv");
      }
    }
  }  
  digitalWrite(IridPwrPin, LOW);   //Drive iridium power pin LOW
  return 1;
}

void setup(void){
  
  // OPTIONAL IF PLUGGED IN
  delay(1000);
  Serial.begin(9600);
  Serial.println("starting");

  // SET LED  
  pinMode(led, OUTPUT); delay(30); digitalWrite(led, HIGH); delay(2000); digitalWrite(led, LOW);
  
  // SET RELAYS
  pinMode(WiperSetPin, OUTPUT); digitalWrite(WiperSetPin, HIGH); delay(30); digitalWrite(WiperSetPin, LOW);
  pinMode(WiperUnsetPin, OUTPUT); digitalWrite(WiperUnsetPin, HIGH); delay(30); digitalWrite(WiperUnsetPin, LOW);
  pinMode(SensorSetPin, OUTPUT); digitalWrite(SensorSetPin, HIGH); delay(30); digitalWrite(SensorSetPin, LOW);
  pinMode(SensorUnsetPin, OUTPUT); digitalWrite(SensorUnsetPin, HIGH); delay(30); digitalWrite(SensorUnsetPin, LOW);

  // SET IRIDIUM 
  pinMode(IridPwrPin, OUTPUT);// delay(30); digitalWrite(IridPwrPin, LOW);

  // START SDI-12 PROTOCOL
  mySDI12.begin();

  // CHECK RTC
  while (!rtc.begin()){blinky(2,200,200,2000);}
  
  // CHECK SD CARD
  while (!SD.begin(chipSelect)) {blinky(3,200,200,2000);}

  //Set paramters for parsing the log file
  CSV_Parser cp("sddffs", true, ',');

  // CHECK PARAM.txt
  while (!cp.readSDfile("/PARAM.txt")){blinky(4,200,200,2000);}

  // SAMPLE ON STARTUP - SET RELAY
  digitalWrite(SensorSetPin, HIGH); delay(30);
  digitalWrite(SensorSetPin, LOW);
  delay(1000); 

  // SAMPLE ON STARTUP - ACTIVATE WIPER
  digitalWrite(WiperSetPin, HIGH); delay(20);    
  digitalWrite(WiperSetPin, LOW); delay(20); delay(100); // >50 ms burst of power activates the wiper's full rotation
  digitalWrite(WiperUnsetPin, HIGH); delay(20);    
  digitalWrite(WiperUnsetPin, LOW); delay(20);    
  delay(10000); // wait for full rotation (about 6 seconds)

  // SAMPLE ON STARTUP - SAMPLE
  String datastring = rtc.now().timestamp()+","+String(sample_analite_195())+","+sample_ott_M()+","+sample_ott_V()+","+sample_batt_v();
  Serial.println(datastring);  
  
  // SAMPLE ON STARTUP - WRITE
  write_to_csv(my_header, datastring, "/DATA.csv");

  // SAMPLE ON STARTUP - UNSET RELAY
  digitalWrite(SensorUnsetPin, HIGH); delay(30);
  digitalWrite(SensorUnsetPin, LOW);

  // SAMPLE ON STARTUP - SEND MESSAGE
  // send_msg("startup: "+ datastring, 120, 1);
  
}

void loop(void){

  // WAKE UP, WHAT TIME? 
  present_time = rtc.now();
  Serial.println(rtc.now().timestamp());
    
  if(present_time.second() % 10 == 0){
    
    blinky(1,20,200,200);

  // WAIT FOR ZERO SECONDS
  if(present_time.second() == 0){
    
    // WAIT FOR SAMPLE INTERVAL 
    if(present_time.minute() % 2 == 0){    
      
      // TURN ON SENSORS  
      digitalWrite(SensorSetPin, HIGH); delay(30);
      digitalWrite(SensorSetPin, LOW);
      delay(1000); 

      String datastring = present_time.timestamp()+","+String(sample_analite_195())+","+sample_ott_M()+","+sample_ott_V()+","+sample_batt_v();
      Serial.println(datastring);

      write_to_csv(my_header, datastring, "/DATA.csv");
    
      digitalWrite(SensorUnsetPin, HIGH); delay(30);
      digitalWrite(SensorUnsetPin, LOW);

      if(present_time.minute() == 0){
        write_to_csv(my_header, datastring, "/HOURLY.csv");
        
        if(present_time.hour() % 1 == 0){
          send_msg(datastring, 300, 2); // 300 secs = 5 min, retry = 2

        }
      }
    }
  }  

  // Sleep until 10 secs interval 0,10,20,...
  DateTime sample_end = rtc.now();
  float sleep = 9.5-(sample_end.second()%10);
  Serial.println(sleep);
  delay(sleep*1000);
  }
  delay(300);
}
