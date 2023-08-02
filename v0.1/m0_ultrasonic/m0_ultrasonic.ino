/*Include required libraries*/
#include "RTClib.h" //Needed for communication with Real Time Clock
#include <SPI.h> //Needed for working with SD card
#include <SD.h> //Needed for working with SD card
#include "ArduinoLowPower.h" //Needed for putting Feather M0 to sleep between samples
#include <IridiumSBD.h> //Needed for communication with IRIDIUM modem 
#include <CSV_Parser.h> //Needed for parsing CSV data
#include <Adafruit_AHTX0.h> //Needed for communicating with  AHT20
#include <QuickStats.h>//Needed for commuting median

/*Define global constants*/
const byte led = 13; // Built in led pin
const byte chipSelect = 4; // Chip select pin for SD card
const byte irid_pwr_pin = 6; // Power base PN2222 transistor pin to Iridium modem
const byte PeriSetPin = 12; //Power relay set pin for all 3.3V peripheral
const byte PeriUnsetPin = 13; //Power relay unset pin for all 3.3V peripheral
const byte pulsePin = 11; //Pulse width pin for reading pw from MaxBotix MB7369 ultrasonic ranger
const byte triggerPin = 10; //Range start / stop pin for MaxBotix MB7369 ultrasonic ranger
const byte vbatPin = 9;
 
/*Define global vars */
String header = "a,b,c,d,e,f";
char **filename; // Name of log file(Read from PARAM.txt)
char **start_time;// Time at which first Iridum transmission should occur (Read from PARAM.txt)
String filestr; // Filename as string
int16_t *sample_intvl; // Sample interval in minutes (Read from PARAM.txt)
int16_t *irid_freq; // Iridium transmit freqency in hours (Read from PARAM.txt)
uint32_t irid_freq_hrs; // Iridium transmit freqency in hours
uint32_t sleep_time;// Logger sleep time in milliseconds
DateTime transmit_time;// Datetime varible for keeping IRIDIUM transmit time
DateTime present_time;// Var for keeping the current time
int err; //IRIDIUM status var
int16_t *N; //Number of ultrasonic reange sensor readings to average.
int sample_n; //same as N[0]
int n_samples = 2; //same as N[0]
int16_t *ultrasonic_height_mm;//Height of ultrasonic sensor,used to compute depth, ignored if stage_mm being logged
int16_t height_mm;//Same as ultrasonic_height_mm[0]
char **metrics_letter_code;// Three letter code for Iridium string, e.g., 'A' for stage 'E' for snow depth (see metrics_lookup in database)
String metrics; //String for representing dist_letter_code
int32_t distance; //Variable for holding distance read from MaxBotix MB7369 ultrasonic ranger
int32_t duration; //Variable for holding pulse width duration read from MaxBotix ultrasonic ranger
sensors_event_t rh_prct, temp_deg_c; //Variable for holding AHT20 temp and RH vals


/*Define Iridium seriel communication as Serial1 */
#define IridiumSerial Serial1

/*Create library instances*/
RTC_DS3231 rtc; // Setup a PCF8523 Real Time Clock instance
File dataFile; // Setup a log file instance
IridiumSBD modem(IridiumSerial); // Declare the IridiumSBD object
Adafruit_AHTX0 aht;//instantiate AHT20 object
QuickStats stats; //initialize an instance of QuickStats class

/*Define User Functions*/
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

String take_measurement(String present) {
  int16_t vbat = read_vbat();
  set_relay();
  float airT = read_air_temp();
  float airRH = read_air_rh();
  float distance = read_ultrasonic();
  unset_relay();
  String datastring = present + "," + distance + "," + airT + "," + airRH + "," + vbat;
  return datastring;
}

float write_to_csv(String datastring) {

  // FILENAME
  String outname = "DATA2.csv";

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

int send_hourly_data(){//Function reads HOURLY.CSV and sends hourly averages over IRIDIUM, formatted as to be ingested by the Omineca CGI script / database

  int err;  // For capturing Iridium errors

  digitalWrite(irid_pwr_pin, HIGH);  // Provide power to Iridium Modem
  delay(200); // Allow warm up

  IridiumSerial.begin(19200);// Start the serial port connected to the satellite modem

  CSV_Parser cp("sdff", true, ',');// Set paramters for parsing the log file

  char **datetimes;//pointer to datetimes
  int16_t *distances;//pointer to distances
  float *air_temps;//pointer to air temps
  float *rhs;//pointer to RHs

  cp.readSDfile("/HOURLY.CSV");// Read HOURLY.CSV file from SD

  int num_rows = cp.getRowsCount();//Get # of rows

  datetimes = (char**)cp["datetime"];//populate datetimes
  if (metrics.charAt(0) == 'E')//If measuring snow depth
  {
    distances = (int16_t*)cp["snow_depth_mm"];//populate snow depths
  } else {//If measuring stage
    distances = (int16_t*)cp["stage_mm"];//populate stage
  }
  air_temps = (float*)cp["air_temp_deg_c"];//populate air temps
  rhs = (float*)cp["rh_prct"];//populate RHs

  String datastring = metrics + ":" + String(datetimes[0]).substring(0, 10) + ":" + String(datetimes[0]).substring(11, 13) + ":";//Formatted for CGI script >> sensor_letter_code:date_of_first_obs:hour_of_first_obs:data

  int start_year = String(datetimes[0]).substring(0, 4).toInt();//year of first obs
  int start_month = String(datetimes[0]).substring(5, 7).toInt();//month of first obs
  int start_day = String(datetimes[0]).substring(8, 10).toInt();//day of first obs
  int start_hour = String(datetimes[0]).substring(11, 13).toInt();//hour of first obs
  int end_year = String(datetimes[num_rows - 1]).substring(0, 4).toInt();//year of last obs
  int end_month = String(datetimes[num_rows - 1]).substring(5, 7).toInt();//month of last obs
  int end_day = String(datetimes[num_rows - 1]).substring(8, 10).toInt();//day of last obs
  int end_hour = String(datetimes[num_rows - 1]).substring(11, 13).toInt();//hour of last obs

  DateTime start_dt = DateTime(start_year, start_month, start_day, start_hour, 0, 0);//Set start date time to first observation time rounded down @ the hour
  DateTime end_dt = DateTime(end_year, end_month, end_day, end_hour + 1, 0, 0);//Set the end datetime to last observation + 1 hour
  DateTime intvl_dt;//For keeping track of the datetime at the end of each hourly interval

  while (start_dt < end_dt)//while the start datetime is < end datetime
  {
    intvl_dt = start_dt + TimeSpan(0, 1, 0, 0);//intvl_dt is equal to start_dt + 1 hour

    float mean_depth = -9999.0; //mean depth / distance
    float mean_temp;//mean air temp
    float mean_rh;//mean air RH
    boolean is_first_obs = true;//Boolean indicating first hourly observation
    boolean is_first_obs_snow = true;//Boolean for first houry snow observation
    int N = 0;//Sample N counter
    int N_snow = 0;

    for (int i = 0; i < num_rows; i++) { //For each observation in the HOURLY.CSV

      String datetime = String(datetimes[i]);//Datetime and row i
      int dt_year = datetime.substring(0, 4).toInt();
      int dt_month = datetime.substring(5, 7).toInt();
      int dt_day = datetime.substring(8, 10).toInt();
      int dt_hour = datetime.substring(11, 13).toInt();
      int dt_min = datetime.substring(14, 16).toInt();
      int dt_sec = datetime.substring(17, 19).toInt();

      DateTime obs_dt = DateTime(dt_year, dt_month, dt_day, dt_hour, dt_min, dt_sec);//Construct DateTime for obs at row i

      if (obs_dt >= start_dt && obs_dt <= intvl_dt)//If obs datetime is withing the current hour interval
      {

        float snow_depth = (float) distances[i];//depth / distance at row i
        float air_temp = air_temps[i];//air temp at row i
        float rh = rhs[i];//RH at row i

        //Need to treat snow diffrent as a good reading is not guarteed
        if (is_first_obs_snow == true)
        {
          //Check that depth is greater than zero (ie not negative) with 100mm of tolerance 
          if (snow_depth > -100.0)
          {
            mean_depth = snow_depth;//mean depth / distance equal to depth at i
            N_snow++;
          }
        } else {
          if (snow_depth > -100.0)
          {
            mean_depth = mean_depth + snow_depth;//Add depth / distance at i to mean_depth
            N_snow++;
          }
        }

        if (is_first_obs == true)//Check if this is the first observation for the hour
        {

          mean_temp = air_temp;//mean_air temp equal to air temp at i
          mean_rh = rh;//mean RH equal to RH at i
          is_first_obs = false;//No longer first obs
          N++;//Increment sample N
        } else {
          mean_temp = mean_temp + air_temp;//Add air temp at i to mean temp
          mean_rh = mean_rh + rh;//Add RH at i to mean RH
          N++;//Increment sample N
        }

      }
    }

    if (N_snow > 0)
    {
      mean_depth = mean_depth / N;//mean depth / distance
    }

    if (N > 0)//Check if there were any observations for the hour
    {
      mean_temp = (mean_temp / N) * 10.0;//mean air temp, convert to int
      mean_rh = (mean_rh / N) * 10.0; //mean RH, convert to int

      datastring = datastring + String(round(mean_depth)) + ',' + String(round(mean_temp)) + ',' + String(round(mean_rh)) + ':';//Assemble the data string

    }

    start_dt = intvl_dt;//Set intvl_dt to start_dt,i.e., next hour
  }

  uint8_t dt_buffer[340];//Binary bufffer for iridium transmission (max allowed buffer size 340 bytes)

  int message_bytes = datastring.length();// Total bytes in Iridium message

  int buffer_idx = 0;//Set buffer index to zero

  for (int i = 0; i < message_bytes; i++)//For each byte in the message (i.e. each char)
  {
    dt_buffer[buffer_idx] = datastring.charAt(i);//Update the buffer at buffer index with corresponding char
    buffer_idx++;//increment buffer index
  }

  modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);// Prevent from trying to charge to quickly, low current setup

  err = modem.begin();// Begin satellite modem operation, blink led (1-sec) if there was an issue

  if (err == ISBD_IS_ASLEEP)//Check if modem is asleep for whatever reason
  {
    modem.begin();
  }

  digitalWrite(led, HIGH); //Indicate the modem is trying to send with led

  err = modem.sendSBDBinary(dt_buffer, buffer_idx); //transmit binary buffer data via iridium

  if (err != 0 && err != 13)// If first attemped failed try once more with extended timeout
  {
    err = modem.begin();
    modem.adjustSendReceiveTimeout(500);
    err = modem.sendSBDBinary(dt_buffer, buffer_idx);
  }

  digitalWrite(led, LOW);//Indicate transmission ended

  digitalWrite(irid_pwr_pin, LOW);//Kill power to modem to save pwr

  SD.remove("/HOURLY.CSV");  //Remove previous daily values CSV

  return err;  //Return err code, not used but can be used for trouble shooting

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
  
  String datastring_msg = String(metrics_letter_code[0]) + ":" + String(datetimes[0]).substring(2, 4) + String(datetimes[0]).substring(5, 7) + String(datetimes[0]).substring(8, 10) + String(datetimes[0]).substring(11, 13) + ":" + String(round(batv[num_rows-1] * 10)) + ":";
 
  for (int i = 0; i < num_rows; i++) {  //For each observation in the IRID.csv
    datastring_msg = datastring_msg + String(round(distances[i])) + ',' + String(round(air_temps[i]*10)) + ',' + String(round(rhs[i]*10)) + ':';              
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

float irid_temp_msg_test(){
  remove_irid_temp();
  String msmt = take_measurement(rtc.now().timestamp());
  write_to_irid_temp(msmt);
  write_to_irid_temp(msmt);
  write_to_irid_temp(msmt);
  write_to_irid_temp(msmt);
  write_to_irid_temp(msmt);
  String irid_msg = irid_temp_to_irid();
  remove_irid_temp();
  return 1;
}

float remove_irid_temp(){
  if(SD.exists("IRID.csv")) {
    SD.remove("IRID.csv");
  }
  return 1;
}

void setup(void){ //Setup section, runs once upon powering up the Feather M0

  while (!SD.begin(chipSelect)) {//Make sure a SD is available (2-sec flash led means SD card did not initialize)
    digitalWrite(led, HIGH);
    delay(2000);
    digitalWrite(led, LOW);
    delay(2000);
  }

  CSV_Parser cp("sddsdds", true, ',');//Set paramters for parsing the parameter file PARAM.txt

  while (!cp.readSDfile("/PARAM.txt"))//Read the parameter file 'PARAM.txt', blink (1-sec) if fail to read
  {
    digitalWrite(led, HIGH);
    delay(1000);
    digitalWrite(led, LOW);
    delay(1000);
  }

  filename = (char**)cp["filename"];//Get file name from parameter file
  sample_intvl = (int16_t*)cp["sample_intvl"];//Get sample interval in seconds from parameter file
  irid_freq = (int16_t*)cp["irid_freq"];//Get iridium freqency in hours from parameter file
  start_time = (char**)cp["start_time"];//Get idridium start time as timestamp from parameter file
  N = (int16_t*)cp["N"];//Get sample N from parameter file
  sample_n = N[0];//Update value of sample_n from parameter file
  ultrasonic_height_mm = (int16_t*)cp["ultrasonic_height_mm"];//Get height of ultrasonic sensor in mm from parameter file
  metrics_letter_code = (char**)cp["metrics_letter_code"];//Get metrics letter code string from parameter file

  metrics = String(metrics_letter_code[0]);//Update metrics with value from parameter file

  sleep_time = sample_intvl[0] * 1000;//Sleep time between samples in miliseconds

  filestr = String(filename[0]); //Log file name

  irid_freq_hrs = irid_freq[0];  //Iridium transmission frequency in hours

  height_mm = ultrasonic_height_mm[0];
  
  if (! rtc.begin()) {
    digitalWrite(led, HIGH); delay(50);
    digitalWrite(led, LOW); delay(50);}

  if (rtc.lostPower()) {
    digitalWrite(led, HIGH); delay(50);
    digitalWrite(led, LOW); delay(50);}
  
  DateTime present_time = rtc.now();
  take_measurement(present_time.timestamp());
  take_measurement(present_time.timestamp());
  take_measurement(present_time.timestamp());
  String msmt = take_measurement(present_time.timestamp());
  write_to_csv(msmt);

}

void loop()//Code executes repeatedly until loss of power
{

  DateTime present_time = rtc.now();//Get the present datetime
    
  if(present_time.second() == 0){
    if(present_time.minute() % 2 == 0){
        String msmt = take_measurement(present_time.timestamp());
        write_to_csv(msmt);
        if(present_time.minute() == 0){
          write_to_irid_temp(msmt);      
          if(present_time.hour() % 1 == 0){
            String irid_msg = irid_temp_to_irid();
            send_msg(irid_msg, 300, 1, "F");  
            remove_irid_temp();          
        }
      }
    }
  }

  digitalWrite(led, HIGH);
  delay(50);
  digitalWrite(led, LOW);
  delay(50);

  DateTime sample_end_time = rtc.now();
  DateTime sample_end_time_simple = DateTime(sample_end_time.year(),
      sample_end_time.month(),
      sample_end_time.day(),
      sample_end_time.hour(),
      sample_end_time.minute(), 
      0);
  DateTime sample_end_time_simple_plus = sample_end_time_simple + TimeSpan(0,0,1,0);
  int32_t delay_seconds = sample_end_time_simple_plus.unixtime() - sample_end_time.unixtime();
  uint32_t sleep_time = 1000 * (delay_seconds - 1); // delay - 1s
  LowPower.sleep(sleep_time);
}
