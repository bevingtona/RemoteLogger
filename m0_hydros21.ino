/*Include the libraries we need*/
#include "RTClib.h" //Needed for communication with Real Time Clock
#include <SPI.h> //Needed for working with SD card
#include <SD.h> //Needed for working with SD card
#include <ArduinoLowPower.h> //Needed for putting Feather M0 to sleep between samples
#include <IridiumSBD.h> //Needed for communication with IRIDIUM modem 
#include <CSV_Parser.h> //Needed for parsing CSV data
#include <SDI12.h> //Needed for SDI-12 communication
#include <QuickStats.h>//Needed for commuting median

/*Define global constants*/
const byte led = 13; // Built in led pin
const byte chipSelect = 4; // Chip select pin for SD card
const byte irid_pwr_pin = 6; // Power base PN2222 transistor pin to Iridium modem
const byte PeriSetPin = 5; //Power relay set pin to HYDROS21
const byte PeriUnsetPin = 10; //Power relay unset pin to HYDROS21
const byte dataPin = 12; // The pin of the SDI-12 data bus
const byte vbatPin = 9;


/*Define global vars */
String header = "datetime, water_level_mm, water_temp_c, water_ec_dcms, batt_v";
int16_t *sample_intvl;// Sample interval in minutes (Read from PARAM.txt)
int16_t *irid_freq;// Iridium transmit freqency in hours (Read from PARAM.txt)
uint32_t irid_freq_hrs;// Iridium transmit freqency in hours
uint32_t sleep_time;// Logger sleep time in milliseconds
DateTime present_time;// Var for keeping the current time
int err;//IRIDIUM status var
String myCommand   = "";// SDI-12 command var
String sdiResponse = "";// SDI-12 responce var

/*Define Iridium seriel communication as Serial1 */
#define IridiumSerial Serial1

/*SDI-12 sensor address, assumed to be 0*/
#define SENSOR_ADDRESS 0

/*Create library instances*/
RTC_PCF8523 rtc; // Setup a PCF8523 Real Time Clock instance (may have to change this to more precise DS3231)
File dataFile; // Setup a log file instance
IridiumSBD modem(IridiumSerial); // Declare the IridiumSBD object
SDI12 mySDI12(dataPin);// Define the SDI-12 bus

float read_params(){
    //Set paramters for parsing the parameter file PARAM.txt
  CSV_Parser cp("dd", true, ',');

  /*Read the parameter file 'PARAM.txt', blink (1-sec) if fail to read*/
  while (!cp.readSDfile("/PARAM.txt"))
  {
    digitalWrite(led, HIGH);
    delay(1000);
    digitalWrite(led, LOW);
    delay(1000);
  }

  sample_intvl = (int16_t*)cp["sample_intvl"];
  irid_freq = (int16_t*)cp["irid_freq"];

  sleep_time = sample_intvl[0] * 1000;//Sleep time between samples in miliseconds from PARAM.txt
  irid_freq_hrs = irid_freq[0];  //Iridium transmission frequency in hours form PARAM.txt
  return 1;
}
float read_vbat() { 
  pinMode(vbatPin, INPUT); //Set VBAT pin as INPUT
  return (analogRead(vbatPin)*2*3.3)/1024;
}

String sample_hydros21(){
  
  set_relay();

  delay(1000); //Give HYDROS21 sensor time to settle 

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

  delay(1000);       // delay between taking reading and requesting data
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

  //Assemble datastring
  ;

  /*Switch power off to HYDROS21 via latching relay*/
  unset_relay();

  return sdiResponse;
}

float set_relay() {
  pinMode(PeriSetPin, OUTPUT);       //Set peripheral set pin as OUTPUT
  pinMode(PeriUnsetPin, OUTPUT);     //Set peripheral unset pin as OUTPUT
  digitalWrite(PeriUnsetPin, HIGH); delay(20); //Drive peripheral unset pin HIGH to assure unset relay
  digitalWrite(PeriUnsetPin, LOW); delay(20);  //Drive peripheral unset pin LOW as it is a latching relay
  digitalWrite(PeriSetPin, HIGH); delay(20); //Drive peripheral unset pin HIGH to assure unset relay
  digitalWrite(PeriSetPin, LOW); delay(20);  //Drive peripheral unset pin LOW as it is a latching relay
  return 1;
}

float unset_relay() {
  pinMode(PeriSetPin, OUTPUT);       //Set peripheral set pin as OUTPUT
  pinMode(PeriUnsetPin, OUTPUT);     //Set peripheral unset pin as OUTPUT
  digitalWrite(PeriUnsetPin, HIGH); delay(20);  //Drive peripheral unset pin HIGH to assure unset relay
  digitalWrite(PeriUnsetPin, LOW); delay(20);  //Drive peripheral unset pin LOW as it is a latching relay
  return 1;
}

String take_measurement(String present) {
  float vbat = read_vbat();
  String hydros = sample_hydros21();
  String datastring = present + "," + hydros + "," + vbat;
  return datastring;
}

float write_to_csv(String datastring_for_csv, String outname) {

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

String irid_temp_to_irid(){

  SD.begin(chipSelect);
  CSV_Parser cp("sffffs", true, ',');  // Set paramters for parsing the log file
  cp.readSDfile("/TEMP.csv");     // Read IRID.csv file from SD
  
  int num_rows = cp.getRowsCount();  //Get # of rows
    
  char **out_datetimes = (char **)cp["datetime"];        // populate datetimes
  float *out_water_level_mm = (float *)cp["water_level_mm"];     // populate snow depths
  float *out_water_temp_c = (float *)cp["water_temp_c"];  // populate air temps
  float *out_water_ec_dcms = (float *)cp["water_ec_dcms"];               // populate RHs
  float *out_bat_v = (float *)cp["batt_v"];                // populate V
  
  String datastring_msg = "ABC:" + String(out_datetimes[0]).substring(2, 4) + String(out_datetimes[0]).substring(5, 7) + String(out_datetimes[0]).substring(8, 10) + String(out_datetimes[0]).substring(11, 13) + ":" + String(round(out_bat_v[num_rows-1] * 10)) + ":";
  
  for (int i = 0; i < num_rows; i++) {  //For each observation in the IRID.csv
    datastring_msg = datastring_msg + String(round(out_water_level_mm[i])) + ',' + String(round(out_water_temp_c[i]*10)) + ',' + String(round(out_water_ec_dcms[i])) + ':';              
    }
  
  return datastring_msg;
  }
  

float send_msg(String msg, float timeout_s, float retry_n, String quality){

  pinMode(irid_pwr_pin, OUTPUT);     //Set iridium power pin as OUTPUT
  digitalWrite(irid_pwr_pin, HIGH);   //Drive iridium power pin LOW
  delay(2000);
  int err;
  
  IridiumSerial.begin(19200);  // Start the serial port connected to the satellite modem
  modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE); // This is a low power application
  err = modem.begin();
  
  if(quality == "T"){
    int signalQuality = -1;
    for (int16_t i = 0; i < 5; i++) {
      err = modem.getSignalQuality(signalQuality);
      }
  }

  modem.adjustSendReceiveTimeout(timeout_s);
  err = modem.sendSBDText(msg.c_str());  
  if(retry_n > 0){
  for (int16_t i = 0; i < retry_n; i++) {
    if(err != ISBD_SUCCESS)//err != 0 && err != 13)// If first attemped failed try once more with extended timeout
      {
        modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE); // This is a low power application
        err = modem.begin(); 
        modem.adjustSendReceiveTimeout(timeout_s);
          err = modem.sendSBDText(msg.c_str());
      }
    }
  }  
  digitalWrite(irid_pwr_pin, LOW);   //Drive iridium power pin LOW
  return 1;
  }



/*
   The setup function. We only initialize the sensors, RTC, SD and varible states here
*/
void setup(void)
{
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  pinMode(irid_pwr_pin, OUTPUT);
  digitalWrite(irid_pwr_pin, LOW);


  /*Make sure a SD is available (2-sec flash led means SD card did not initialize)*/
  while (!SD.begin(chipSelect)) {
    digitalWrite(led, HIGH);
    delay(2000);
    digitalWrite(led, LOW);
    delay(2000);
  }

  read_params();
  
  /*Make sure RTC is available*/
  while (!rtc.begin())
  {
    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(led, LOW);
    delay(500);
  }

  mySDI12.begin();  //Begin HYDROS21
  // Serial.begin(9600);
  
  String datastring = take_measurement(rtc.now().timestamp());
  write_to_csv(datastring + ",startup", "/DATA.csv");
  // send_msg("startup:"+datastring, 300, 2, "F");
  // Serial.println(datastring);

  SD.remove("/TEMP.CSV");  //Remove previous daily values CSV

}

/*
   Code here is repeated, sample HYDROS21 at sample interval, log to SD, and transmit hourly averages over IRIDIUM at specified intervals
*/
void loop(void)
{
  digitalWrite(led, HIGH);
  delay(50);
  digitalWrite(led, LOW);
    
  present_time = rtc.now();  //Get the present datetime from RTC
  // Serial.println(present_time.timestamp());  
  
  if(present_time.second() == 0 ){
    read_params();

    if(present_time.minute() % sample_intvl[0] == 0){
      
      String datastring = take_measurement(present_time.timestamp());
      write_to_csv(datastring,"/DATA.csv");
      // Serial.println(datastring);

        if(present_time.minute() == 0){
          write_to_csv(datastring,"/TEMP.csv"); 

          if(present_time.hour() % irid_freq[0] == 0){
            String irid_datastring = irid_temp_to_irid();
            send_msg(irid_datastring, 300, 2, "F");
            write_to_csv(datastring+",irid","/HIST.csv"); 
            // Serial.println(irid_datastring);
            SD.remove("/TEMP.CSV");  //Remove previous daily values CSV
          
          }
        }
      }
      
    }

  if(rtc.now().second() < 56){
    // delay(4000);
    LowPower.sleep(4000);
    }else{
      // delay(1000);
      LowPower.sleep(1000);
      }

}
