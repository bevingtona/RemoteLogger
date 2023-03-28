/*Include required libraries*/
#include <RTClib.h>           //Needed for communication with Real Time Clock
#include <SPI.h>              //Needed for working with SD card
#include <SD.h>               //Needed for working with SD card
#include <IridiumSBD.h>       //Needed for communication with IRIDIUM modem
#include <CSV_Parser.h>       //Needed for parsing CSV data
#include <Adafruit_AHTX0.h>   //Needed for communicating with  AHT20
#include <QuickStats.h>       //Needed for computing median
#include <ArduinoLowPower.h>  //Needed for putting Feather M0 to sleep

/*Define global constants*/
const byte led = 13;           // Built in led pin
const byte chipSelect = 4;     // Chip select pin for SD card
const byte irid_pwr_pin = 6;   // Power base PN2222 transistor pin to Iridium modem
const byte PeriSetPin = 12;    // Power relay set pin for all 3.3V peripheral
const byte PeriUnsetPin = 13;  // Power relay unset pin for all 3.3V peripheral
const byte pulsePin = 11;      // Pulse width pin for reading pw from MaxBotix MB7369 ultrasonic ranger
const byte triggerPin = 10;    // Range start / stop pin for MaxBotix MB7369 ultrasonic ranger
const byte vbatPin = 9;        // WARNING: If something is plugged into pin 9, then th voltage will be wrong
const int  alogRes = 12;       // Define anlog read resolution as 12 bit
float vbat;

/*Define global variables*/
int32_t distance; //Variable for holding distance read from MaxBotix MB7369 ultrasonic ranger
int32_t duration; //Variable for holding pulse width duration read from MaxBotix ultrasonic ranger

/*Define Iridium serial communication as Serial1 */
#define IridiumSerial Serial1

/*Create library instances*/
RTC_DS3231 rtc;                   // Setup a PCF8523 Real Time Clock instance
File dataFile;                    // Setup a log file instance
IridiumSBD modem(IridiumSerial);  // Declare the IridiumSBD object
Adafruit_AHTX0 aht;               //instantiate AHT20 object
QuickStats stats;                 //initialize an instance of QuickStats class

/*Variable names from Params CSV*/
String header = "datetime,distance_mm,air_temp_deg_c,rh_prct,bat_v";
String msmt;
int16_t *sample_int_s;
int16_t *irid_freq_h; // Iridium transmit freqency in hours (Read from PARAM.txt)
int16_t n_samples = 2; //Number of ultrasonic reange sensor readings to average.
char **metrics_code; // Three letter code for Iridium string, e.g., 'A' for stage 'E' for snow depth (see metrics_lookup in database)
char **onstart_sync_clock;
char **onstart_print_vals;
char **onstart_irid_check;
int16_t counter = 1;

/* Define User Functions */

float read_vbat() { // Reads battery voltage on a Adalogger M0 (assumes 12-bit alog res)
  pinMode(vbatPin, INPUT); //Set VBAT pin as INPUT
  return (analogRead(vbatPin)*2*3.3)/1024;
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

float read_air_temp() {
  if (! aht.begin()) {
    Serial.println("Could not find AHT. Check wiring");
    while (1) {
      digitalWrite(led, HIGH);
      delay(100);
      digitalWrite(led, LOW);
      delay(100);}
  }
  sensors_event_t temp, humidity;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  return temp.temperature;
  }

float read_air_rh() {
  if (! aht.begin()) {
    Serial.println("Could not find AHT. Check wiring");
    while (1) {
      digitalWrite(led, HIGH);
      delay(100);
      digitalWrite(led, LOW);
      delay(100);}
  }
  sensors_event_t temp, humidity;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  return humidity.relative_humidity;
}

float read_ultrasonic() {
  pinMode(triggerPin, OUTPUT);       //Set ultrasonic ranging trigger pin as OUTPUT
  pinMode(pulsePin, INPUT);          //Set ultrasonic pulse width pin as INPUT

  float values[n_samples];          //Array for storing sampled distances
  digitalWrite(triggerPin, HIGH);  //Write the ranging trigger pin HIGH
  delay(2000);                     //Let sensor settle
  for (int16_t i = 0; i < n_samples; i++)  //Take N samples
  {
    duration = pulseIn(pulsePin, HIGH);  //Get the pulse duration (i.e.,time of flight)
    values[i] = duration;
    delay(150);  //Dont sample too quickly < 7.5 Htz
  }
  int16_t med_distance = round(stats.median(values, n_samples));  //Copute median distance
  digitalWrite(triggerPin, LOW);  //Stop continious ranging
  return med_distance;
}

String take_measurement(String present_time) {
  vbat = read_vbat();
  set_relay();
  float airT = read_air_temp();
  float airRH = read_air_rh();
  float distance = read_ultrasonic();
  unset_relay();
  String datastring = present_time + "," + distance + "," + airT + "," + airRH + "," + vbat;
  return datastring;
}

float write_to_csv(String datastring) {

  // FILENAME
  String outname = "DATA.csv";

  // IF FILE DOES NOT EXIST, WRITE HEADER AND DATA, ELSE, WITE DATA
  if (!SD.exists(outname))  //Write header if first time writing to the logfile
  {
    dataFile = SD.open(outname, FILE_WRITE);  //Open file under filestr name from parameter file
    if (dataFile) {
      dataFile.println(header+",comment");
      dataFile.println(datastring);
    }
    dataFile.close();  //Close the dataFile
  } else {
    dataFile = SD.open(outname, FILE_WRITE);
    if (dataFile) {
      dataFile.println(datastring);
      dataFile.close();
    }
  }
  return 1;
}

float write_to_irid_temp(String datastring) {

  // FILENAME
  String outname = "IRID.csv";

  // IF FILE DOES NOT EXIST, WRITE HEADER AND DATA, ELSE, WITE DATA
  if (!SD.exists(outname))  //Write header if first time writing to the logfile
  {
    dataFile = SD.open(outname, FILE_WRITE);  //Open file under filestr name from parameter file
    if (dataFile) {
      dataFile.println(header);
      dataFile.println(datastring);
    }
    dataFile.close();  //Close the dataFile
  } else {
    dataFile = SD.open(outname, FILE_WRITE);
    if (dataFile) {
      dataFile.println(datastring);
      dataFile.close();
    }
  }
  return 1;
}

String irid_temp_to_irid(){

  SD.begin(chipSelect);
  CSV_Parser cp("sffff", true, ',');  // Set paramters for parsing the log file
  cp.readSDfile("IRID.csv");     // Read IRID.csv file from SD
  char **datetimes;   //pointer to datetimes
  float *distances;   //pointer to distances
  float *air_temps;   //pointer to air temps
  float *rhs;         //pointer to RHs
  float *batv;        //pointer to V
  int num_rows = cp.getRowsCount();  //Get # of rows

  datetimes = (char **)cp["datetime"];        // populate datetimes
  distances = (float *)cp["distance_mm"];     // populate snow depths
  air_temps = (float *)cp["air_temp_deg_c"];  // populate air temps
  rhs = (float *)cp["rh_prct"];               // populate RHs
  batv = (float *)cp["bat_v"];                // populate V
  
  String datastring_msg = " " + String(metrics_code[0]) + ":" + String(datetimes[0]).substring(2, 4) + String(datetimes[0]).substring(5, 7) + String(datetimes[0]).substring(8, 10) + String(datetimes[0]).substring(11, 13) + ":" + String(round(batv[num_rows-1] * 10)) + ":";
 
  for (int i = 0; i < num_rows; i++) {  //For each observation in the IRID.csv
    datastring_msg = datastring_msg + String(round(distances[i])) + ',' + String(round(air_temps[i]*10)) + ',' + String(round(rhs[i]*10)) + ':';              
    }
  return datastring_msg;
  }
  
float send_msg(String msg){
  
  pinMode(irid_pwr_pin, OUTPUT);     //Set iridium power pin as OUTPUT
  digitalWrite(irid_pwr_pin, HIGH);   //Drive iridium power pin LOW
  delay(2000);
  int err;
  Serial.begin(115200);// Start the console serial port
  while (!Serial);
  IridiumSerial.begin(19200);  // Start the serial port connected to the satellite modem
  Serial.println("Starting modem...");  // Begin satellite modem operation
  //IridiumSerial.setPowerProfile(IridiumSBD::USB_POWER_PROFILE); // This is a low power application
  err = modem.begin();
  err = modem.sendSBDText(msg.c_str());
  if (err != ISBD_SUCCESS){
    Serial.println("Failed..");
  }else{
    Serial.println("Success..");
  } 
  digitalWrite(irid_pwr_pin, LOW);   //Drive iridium power pin LOW
  String datastring = rtc.now().timestamp() + "," + msg;
  Serial.println(datastring);  
  return 1;
  }

float irid_temp_msg_test(){
  remove_irid_temp();
  String msmt = take_measurement(rtc.now().timestamp());
  write_to_irid_temp(msmt);
  write_to_irid_temp(msmt);
  write_to_irid_temp(msmt);
  write_to_irid_temp(msmt);
  write_to_irid_temp(msmt);
  String irid_msg = irid_temp_to_irid();
  Serial.println("irid: " + irid_msg);
  remove_irid_temp();
  return 1;
}

float remove_irid_temp(){
  if(SD.exists("IRID.csv")) {
    Serial.println("IRID.csv exists, remove");
    SD.remove("IRID.csv");
  }else{
    Serial.println("IRID.csv doesn't exist");
  }
  return 1;
}
    
void setup() {

  // START SERIAL, WAIT 2 SECS
  Serial.begin(9600);
  delay(2000); 

  // CHECK IF SD ELSE 1 BEEP
  while (!SD.begin(chipSelect)) {
    digitalWrite(led, HIGH);
    delay(200);
    digitalWrite(led, LOW);
    delay(2000);
    }

  // REMOVE IRID_TEMP ON STARTUP
  remove_irid_temp();
  
  // READ PARAMS
  CSV_Parser cp("ddssss", true, ',');  //Set paramters for parsing the parameter file PARAM.txt

  // READ SD 2 BEEPS
  while (!cp.readSDfile("/PARAM.txt")) {
    digitalWrite(led, HIGH);
    delay(200);
    digitalWrite(led, LOW);
    delay(200);
    digitalWrite(led, HIGH);
    delay(200);
    digitalWrite(led, LOW);
    delay(2000);
    }

  // READ SAMPLE INTERVAL
  sample_int_s = (int16_t *)cp["sample_int_s"];
  String sample_int_s_string = String(sample_int_s[0]);
  Serial.print("Sample interval: " + sample_int_s_string + " seconds\r\n");

  // READ IRIDIUM FREQUENCY
  irid_freq_h = (int16_t *)cp["irid_freq_h"];   //Get iridium freqency in hours from parameter file
  String irid_freq_h_string = String(irid_freq_h[0]);
  Serial.print("Iridium frequency: " + irid_freq_h_string + " hours\r\n");

  // READ LETTER CODES
  metrics_code = (char **)cp["metrics_code"];             //Get metrics letter code string from parameter file
  String metrics_code_string = String(metrics_code[0]);
  Serial.print("Letter codes: " + metrics_code_string + "\r\n");

  // READ ONSTART SYNC CLOCK
  onstart_sync_clock = (char **)cp["onstart_sync_clock"];
  String onstart_sync_clock_string = String(onstart_sync_clock[0]);
  Serial.print("Sync clock: " + onstart_sync_clock_string + "\r\n");

  // READ ONSTART PRINT VALS
  onstart_print_vals = (char **)cp["onstart_print_vals"];
  String onstart_print_vals_string = String(onstart_print_vals[0]);
  Serial.print("Print vals: " + onstart_print_vals_string + "\r\n");

  // READ ONSTART IRIDIUM CHECK
  onstart_irid_check = (char **)cp["onstart_irid_check"];
  String onstart_irid_check_string = String(onstart_irid_check[0]);
  Serial.print("Iridium check: " + onstart_irid_check_string + "\r\n" + "\r\n");
  
  // SYNC CLOCK IF TRUE ////////////////////////////////////////////////////////////
  
  if (onstart_sync_clock_string == String("T")) {
    set_relay();
    Serial.println("SYNC RTC CLOCK TO PC CLOCK TZ"); // DO NOT SHARE RTC POWER OR GROUND WITH RELAY
    if (! rtc.begin()) {
      Serial.println("Couldn't find RTC");
      Serial.flush();
      while (1) delay(10);
      }
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println(rtc.now().timestamp());
    Serial.println();
    unset_relay();
    }

  // PRINT VALS AND SAVE TO CARD ///////////////////////////////////////////////////
  
  if (onstart_print_vals_string == String("T")) {
    msmt = take_measurement(rtc.now().timestamp());
    Serial.println(msmt);
    Serial.println();
    write_to_csv(msmt + ",startup");
    }

  // TEST IRIDIUM IF TRUE //////////////////////////////////////////////////////////
  
  if (onstart_irid_check_string == String("TT")) {
    send_msg(msmt);
  }
  
  // irid_temp_msg_test(); // TEST IRIDIUM FUNCTIONS
}

void loop() {

  Serial.begin(9600);
  DateTime sample_start_time = rtc.now();
  Serial.println(sample_start_time.timestamp());
  
  int16_t sample_int_m = 1;
  int16_t irid_sample_freq_m = 5;
  int16_t irid_msg_freq_m = 10;

  // ONLY SAMPLE ON 0 SECONDS
  if(sample_start_time.second() == 0) {
    
    // SAMPLE ON INTERVAL
    if(sample_start_time.minute() % sample_int_m == 0){
      
      // TAKE MEASUREMENT
      String msmt = take_measurement(sample_start_time.timestamp());//
      Serial.println("msmt: " + msmt);
      write_to_csv(msmt+",");
      
      // WRITE TO IRID_TEMP ON HOUR
      if(sample_start_time.minute() % irid_sample_freq_m == 0) {
                
        // WRITE TO IRID_TEMP
        Serial.println("irid_temp: " + msmt);
        write_to_irid_temp(msmt);

        // SEND IRIDIUM IF IRID MODULUS == 0
        if(sample_start_time.minute() % irid_msg_freq_m == 0) {
        
          SD.begin(chipSelect);
          CSV_Parser cp("ddssss", true, ',');  
          cp.readSDfile("PARAM.txt");
          metrics_code = (char **)cp["metrics_code"]; 
          
          String irid_msg = irid_temp_to_irid();
          Serial.println("irid: " + irid_msg);
          
          send_msg(irid_msg);  
          remove_irid_temp();          
        }
      }

    // SLEEP
    DateTime sample_end_time = rtc.now();
    int32_t delay_seconds = sample_start_time.unixtime() + (sample_int_m*60) - sample_end_time.unixtime();
    Serial.println(delay_seconds - 1);
    uint32_t sleep_time = 1000 * (delay_seconds - 1); // delay - 1s
    LowPower.sleep(sleep_time);// 
    // delay(sleep_time); 
    }
  }
  delay(1000);
}
