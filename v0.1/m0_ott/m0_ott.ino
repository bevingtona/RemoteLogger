/*Include required libraries*/
//#include <RTClib.h>           //Needed for communication with Real Time Clock
//#include <SPI.h>              //Needed for working with SD card
//#include <SD.h>               //Needed for working with SD card
//#include <IridiumSBD.h>       //Needed for communication with IRIDIUM modem
//#include <CSV_Parser.h>       //Needed for parsing CSV data
//#include <Adafruit_AHTX0.h>   //Needed for communicating with  AHT20
//#include <QuickStats.h>       //Needed for computing median
//#include <ArduinoLowPower.h>  //Needed for putting Feather M0 to sleep
#include <SDI12.h> //Needed for SDI-12 communication

/*Define global constants*/
const byte led = 13; // Built in led pin
const byte chipSelect = 4; // Chip select pin for SD card
const byte irid_pwr_pin = 6; // Power base PN2222 transistor pin to Iridium modem
const byte PeriSetPin = 12;    // Power relay set pin for all 3.3V peripheral
const byte PeriUnsetPin = 13;  // Power relay unset pin for all 3.3V peripheral
const byte dataPin = 11; // The pin of the SDI-12 data bus
//const byte vbatPin = 9;        // WARNING: If something is plugged into pin 9, then th voltage will be wrong
//const int  alogRes = 12;       // Define anlog read resolution as 12 bit
//float vbat;

/*Define global vars */
//char **filename; // Name of log file(Read from PARAM.txt)
//char **start_time;// Time at which first Iridum transmission should occur (Read from PARAM.txt)
//String filestr; // Filename as string
//int16_t *sample_intvl; // Sample interval in minutes (Read from PARAM.txt)
//int16_t *irid_freq; // Iridium transmit freqency in hours (Read from PARAM.txt)
//uint32_t irid_freq_hrs; // Iridium transmit freqency in hours
//uint32_t sleep_time;// Logger sleep time in milliseconds
//DateTime transmit_time;// Datetime varible for keeping IRIDIUM transmit time
//DateTime present_time;// Var for keeping the current time
//int err; //IRIDIUM status var
String myCommand   = "";// SDI-12 command var
String sdiResponse = "";// SDI-12 responce var

/*Define Iridium seriel communication as Serial1 */
#define IridiumSerial Serial1

/*SDI-12 sensor address, assumed to be 0*/
#define SENSOR_ADDRESS 0

/*Create library instances*/
//RTC_PCF8523 rtc; // Setup a PCF8523 Real Time Clock instance
//File dataFile; // Setup a log file instance
//IridiumSBD modem(IridiumSerial); // Declare the IridiumSBD object
SDI12 mySDI12(dataPin);// Define the SDI-12 bus

String sample_hydros21()
{
  
  pinMode(PeriSetPin, OUTPUT);       //Set peripheral set pin as OUTPUT
  pinMode(PeriUnsetPin, OUTPUT);     //Set peripheral unset pin as OUTPUT

  //Switch power to HYDR21 via latching relay
  digitalWrite(PeriSetPin , HIGH);
  delay(30);
  digitalWrite(PeriSetPin, LOW);

  mySDI12.begin();
  //Give HYDROS21 sensor time to power up
  delay(1000);

// M ////////////////////////////////////////////////
  myCommand = String(SENSOR_ADDRESS) + "M!";
  mySDI12.sendCommand(myCommand);
  delay(30);  // wait a while for a response
  while (mySDI12.available()) {  // build response string
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);}}
  if (sdiResponse.length() > 1)
    mySDI12.clearBuffer();
  delay(1000);       // delay between taking reading and requesting data
  sdiResponse = "";  // clear the response string

// D0 ////////////////////////////////////////////////
  myCommand = String(SENSOR_ADDRESS) + "D0!";
  mySDI12.sendCommand(myCommand);
  delay(30);  // wait a while for a response
  while (mySDI12.available()) {  // build string from response
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);}}
  sdiResponse = sdiResponse.substring(3);
  for (int i = 0; i < sdiResponse.length(); i++){
    char c = sdiResponse.charAt(i);
    if (c == '+'){
      sdiResponse.setCharAt(i, ',');}}
  if (sdiResponse.length() > 1)
    mySDI12.clearBuffer();
  String hydrostring = "";
  hydrostring = hydrostring + sdiResponse;

// M ////////////////////////////////////////////////
  myCommand = String(SENSOR_ADDRESS) + "V!";
  mySDI12.sendCommand(myCommand);
  delay(30);  // wait a while for a response
  while (mySDI12.available()) {  // build response string
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      // Serial.println(sdiResponse);
      delay(10);}}
  if (sdiResponse.length() > 1)
    mySDI12.clearBuffer();
  delay(1000);       // delay between taking reading and requesting data
  sdiResponse = "";  // clear the response string

// D0 ////////////////////////////////////////////////
  myCommand = String(SENSOR_ADDRESS) + "D0!";
  mySDI12.sendCommand(myCommand);
  delay(30);  // wait a while for a response
  while (mySDI12.available()) {  // build string from response
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);}}
  sdiResponse = sdiResponse.substring(3);
  for (int i = 0; i < sdiResponse.length(); i++){
    char c = sdiResponse.charAt(i);
    if (c == '+' |c == '-'){
      sdiResponse.setCharAt(i, ',');}}
  hydrostring = hydrostring + "," + sdiResponse;
  if (sdiResponse.length() > 1)
    mySDI12.clearBuffer();
  
  //Switch power to HYDR21 via latching relay
  digitalWrite(PeriUnsetPin, HIGH);
  delay(30);
  digitalWrite(PeriUnsetPin, LOW);
  Serial.println(hydrostring);
  return hydrostring;
}


void setup(void){
  Serial.begin(9600);
  delay(1000);
  Serial.println("setup");
}

void loop(void){
  Serial.begin(9600);
  delay(1000);
  Serial.println("present_time.timestamp()");
  String datastring = sample_hydros21();    
  Serial.println(datastring);
  digitalWrite(led, HIGH);
  delay(250);
  digitalWrite(led, LOW);
  delay(5000);
}
