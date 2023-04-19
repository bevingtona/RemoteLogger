// LIBRARIES
#include <RTClib.h> //Needed for communication with Real Time Clock
#include <SPI.h>//Needed for working with SD card
#include <SD.h>//Needed for working with SD card
#include <ArduinoLowPower.h>//Needed for putting Feather M0 to sleep between samples
#include <IridiumSBD.h>//Needed for communication with IRIDIUM modem 
#include <CSV_Parser.h>//Needed for parsing CSV data
#include <SDI12.h>//Needed for SDI-12 communication
#include <QuickStats.h>//Needed for computing medians
#include <diy_hydro_functions.h>

// VARIABLES
String my_header = "datetime,ntu,water_level_mm,water_temp_c,water_ec_dcm,batt_v";
String my_letter_code = "ABCD"; 

// CONSTANTS
const byte led = 8; // Built in GREEN LED pin (13 conflicts with irid)
const byte chipSelect = 4; // For SD card

// IRIDIUM PINS
const byte IridPwrPin = 13; // Pwr pin to Iridium modem // THIS IS ALSO THE PIN FOR THE RED LED

// SENSOR PINS
const byte SensorSetPin = 5; //Pwr set pin to HYDROS21
const byte SensorUnsetPin = 6; //Pwr unset pin to HYDROS21
const byte dataPin = 12; // The pin of the SDI-12 data bus

// WIPER PINS 
const byte WiperSetPin = 10; // Pin to set pwr relay to Analite 195
const byte WiperUnsetPin = 11; // Pin to unset pwr relay to Analite 195
const byte TurbAlog = A1; // Pin for reading analog outout from voltage divder (R1=1000 Ohm, R2=5000 Ohm) conncted to Analite 195

// BAT PINS
const byte vbatPin = 9;

// VARIABLES 
DateTime present_time;          // Var for keeping the current time
int16_t *sample_freq_m;         // sample frequency in minutes from SD card
uint32_t sample_freq_m_32;      // sample frequency in minutes from SD card
int16_t *irid_freq_h;           // iridium frequency in hours from SD card
uint32_t irid_freq_h_32;        // iridium frequency in hours from SD card
char **onstart_irid;             // send iridium message onstrart? 
String onstart_irid_string;     // 
int wiper_cnt = 0;
int err;                        // IRIDIUM status var
String myCommand   = "";        // SDI-12 command var
String sdiResponse = "";        // SDI-12 response var

/*Define Iridium seriel communication COM1*/
#define IridiumSerial Serial1

/*SDI-12 sensor address, assumed to be 0*/
int SENSOR_ADDRESS = 0;

/*Create library instances*/
RTC_PCF8523 rtc;                  // Setup a PCF8523 Real Time Clock instance
File dataFile;                    // Setup a log file instance
IridiumSBD modem(IridiumSerial);  // Declare the IridiumSBD object
SDI12 mySDI12(dataPin);           // Define the SDI-12 bus
QuickStats stats;                 // Instance of QuickStats

// TAKE MEASUREMENT FUNCTION
String take_measurement() {
  return String(sample_analite_195(WiperSetPin, WiperUnsetPin, TurbAlog, wiper_cnt)) +","+
         String(sample_hydros_M(SENSOR_ADDRESS, myCommand, sdiResponse, dataPin) +","+
         sample_batt_v(vbatPin));
}

// PREP THE IRIDIUM MESSAGE
String prep_msg(){
  
  SD.begin(chipSelect);
  CSV_Parser cp("sfffff", true, ',');  // Set paramters for parsing the log file
  cp.readSDfile("/HOURLY.csv");
  int num_rows = cp.getRowsCount();  //Get # of rows
    
  char **out_datetimes = (char **)cp["datetime"];
  float *out_ntu = (float *)cp["ntu"];
  float *out_water_level_mm = (float *)cp["water_level_mm"];
  float *out_water_temp_c = (float *)cp["water_temp_c"];
  float *out_water_ec_dcm = (float *)cp["water_ec_dcm"];
  float *out_batt_v = (float *)cp["batt_v"];
  
  String datastring_msg = my_letter_code + ":" +
    String(out_datetimes[0]).substring(2, 4) + 
    String(out_datetimes[0]).substring(5, 7) + 
    String(out_datetimes[0]).substring(8, 10) + 
    String(out_datetimes[0]).substring(11, 13) + ":" + 
    String(round(out_batt_v[num_rows-1] * 100)) + ":";
  
  for (int i = 0; i < num_rows; i++) {  //For each observation in the IRID.csv
    datastring_msg = 
      datastring_msg + 
      String(round(out_ntu[i])) + ',' + 
      String(round(out_water_level_mm[i])) + ',' + 
      String(round(out_water_temp_c[i]*10)) + ',' + 
      String(round(out_water_ec_dcm[i])) + ':'; 
    }
  return datastring_msg;
}

void setup(void){
  
  // OPTIONAL
  delay(1000);
  Serial.begin(9600);
  Serial.println("starting");

  // SET LED  
  pinMode(led, OUTPUT); delay(30); digitalWrite(led, HIGH); delay(30); digitalWrite(led, LOW);

  // SET RELAYS
  pinMode(SensorSetPin, OUTPUT); digitalWrite(SensorSetPin, HIGH); delay(30); digitalWrite(SensorSetPin, LOW);
  pinMode(SensorUnsetPin, OUTPUT); digitalWrite(SensorUnsetPin, HIGH); delay(30); digitalWrite(SensorUnsetPin, LOW);
  
  // SET IRIDIUM 
  pinMode(IridPwrPin, OUTPUT); delay(30); digitalWrite(IridPwrPin, HIGH); delay(30); digitalWrite(IridPwrPin, LOW);

  // CHECK RTC
  while (!rtc.begin()){blinky(led, 2,200,200,2000);}
  
  // CHECK SD CARD
  while (!SD.begin(chipSelect)) {blinky(led, 3,200,200,2000);}

  // START SDI-12 PROTOCOL
  mySDI12.begin();

  // READ PARAMS
  CSV_Parser cp("dds", true, ',');
	while (!cp.readSDfile("/PARAM.txt"))  {blinky(led, 1,1000,1000,1000);}
	  
	sample_freq_m = (int16_t*)cp["sample_freq_m"];
	sample_freq_m_32 = sample_freq_m[0];
	Serial.println(sample_freq_m_32);
	
	irid_freq_h = (int16_t*)cp["irid_freq_h"];
	irid_freq_h_32 = irid_freq_h[0];
	Serial.println(irid_freq_h_32);
	
	onstart_irid = (char**)cp["onstart_irid"];
	onstart_irid_string = String(onstart_irid[0]);
	Serial.println(onstart_irid_string);
    
  // SAMPLE ON STARTUP - SET RELAY
  digitalWrite(SensorSetPin, HIGH); delay(30); digitalWrite(SensorSetPin, LOW); delay(1000); 

  // SAMPLE ON STARTUP - ACTIVATE WIPER
  pinMode(WiperSetPin, OUTPUT); digitalWrite(WiperSetPin, HIGH); delay(200); digitalWrite(WiperSetPin, LOW);
  pinMode(WiperUnsetPin, OUTPUT); digitalWrite(WiperUnsetPin, HIGH); delay(30); digitalWrite(WiperUnsetPin, LOW);
  delay(10000);

  // SAMPLE ON STARTUP - SAMPLE
  String datastring = rtc.now().timestamp()+","+ take_measurement();
  Serial.println(datastring);  
  
  // SAMPLE ON STARTUP - WRITE
  write_to_csv(my_header, datastring, "/DATA.csv");

  // SAMPLE ON STARTUP - UNSET RELAY
  digitalWrite(SensorUnsetPin, HIGH); delay(30); digitalWrite(SensorUnsetPin, LOW);

  // TEST IRID MSG FORMAT
  SD.remove("/HOURLY.csv");  
  write_to_csv(my_header, datastring, "/HOURLY.csv");
  write_to_csv(my_header, datastring, "/HOURLY.csv");
  write_to_csv(my_header, datastring, "/HOURLY.csv");
  write_to_csv(my_header, datastring, "/HOURLY.csv");
  write_to_csv(my_header, datastring, "/HOURLY.csv");
  datastring = prep_msg();
  Serial.println(datastring);

  // SAMPLE ON STARTUP - SEND MESSAGE
  if(onstart_irid_string == "T"){send_msg(IridPwrPin, "startup: "+ datastring, 120, 1);}


}

void loop(void){

  // WAKE UP, WHAT TIME? 
  present_time = rtc.now();
  Serial.println(rtc.now().timestamp());

  // BLINK IF INTERVAL OF 10s
  if(present_time.second() % 10 == 0){
    blinky(led, 1,20,200,200);

    // SAMPLE IF AT INTERVAL AND ON 0s
    if(present_time.minute() % sample_freq_m_32 == 0 & present_time.second() == 0){    
        
      // SAMPLE - SET RELAY 
      digitalWrite(SensorSetPin, HIGH); delay(30); digitalWrite(SensorSetPin, LOW); delay(1000); 

      // SAMPLE - SAMPLE
      String datastring = present_time.timestamp()+","+ take_measurement();
      Serial.println(datastring);

      // SAMPLE - WRITE TO CSV 
      write_to_csv(my_header, datastring, "/DATA.csv");
      
      // SAMPLE - UNSET RELAY
      digitalWrite(SensorUnsetPin, HIGH); delay(30); digitalWrite(SensorUnsetPin, LOW);

      // SAVE TO HOURLY IF ON THE HOUR
      if(present_time.minute() == 0){
        write_to_csv(my_header, datastring, "/HOURLY.csv");
          
        // SEND MESSAGE IF ON IRIDIUM INTERVAL
        if(present_time.hour() % irid_freq_h_32 == 0){
          datastring = prep_msg();
          send_msg(IridPwrPin, datastring, 300, 2); 
          SD.remove("/HOURLY.csv");
           
        }
      }
    }
    
    // Sleep until 10 secs interval 0,10,20,...
    DateTime sample_end = rtc.now();
    uint32_t sleep_time = ((10-(sample_end.second()%10))*1000.0)-500;
    Serial.println(sleep_time);
    LowPower.sleep(sleep_time); 
  }  
  delay(300);
}
