/**
 * v0.2 of RemoteLogger library for modular remote data loggers
 * requires input for parameters from user: multipliers, letters, number of parameters, and header
 * Author: Rachel Pagdin
 * June 17, 2024
*/

#include <Arduino.h>
#include <RemoteLogger.h>

/* CONSTRUCTORS AND STARTUP */

RemoteLogger::RemoteLogger(){
    /** TODO: remove, this is default header for Hydros21 */
    myHeader = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";
}

RemoteLogger::RemoteLogger(String header){
    myHeader = header;
}

RemoteLogger::RemoteLogger(String header, byte num_params, float *multipliers, String letters){
    myHeader = header;
    myMultipliers = multipliers;
    myParams = num_params;
    myLetters = letters;
}

/**
 * sets up pins and starts up external hardware (RTC, SD)
 * user is responsible for setting up any sensors (SDI-12, etc) to pass to sample functions
*/
void RemoteLogger::begin(){
    // set up main logger pins
    pinMode(ledPin, OUTPUT);
    pinMode(vbatPin, INPUT);
    pinMode(tplPin, OUTPUT);
    pinMode(IridSlpPin, OUTPUT);

    // start RTC
    rtc.begin();

    // start SD card
    SD.begin(chipSelect);

    /* would read any parameters here */
}




/* BASIC UNIT FUNCTIONS */

/**
 * blink preset LED with number of blinks, timing, and pause between sequences set
 * n: number of blinks
 * high_ms: time on for each blink
 * low_ms: time off for each blink
 * btw_ms: time between each sequence of n blinks
*/
void RemoteLogger::blinky(int16_t n, int16_t high_ms, int16_t low_ms, int16_t btw_ms){
    for(int i = 1; i <=n; i++){
        digitalWrite(ledPin, HIGH);
        delay(high_ms);
        digitalWrite(ledPin, LOW);
        delay(low_ms);
    }
    delay(btw_ms);
}

/**
 * write specified header and data to CSV file
 * does not manage matching the lengths for you -- you are responsible for making sure your datastring is the right length
 * do NOT add newline characters to the end of datastrings, this will add empty lines in CSV file
 * only writes header if the file is newly created (i.e. has no header yet)
 * 
 * header: column headers for CSV file
 * datastring_for_csv: the line of data to write to the CSV file
 * outname: name of the CSV file (e.g. /HOURLY.csv)
*/
void RemoteLogger::write_to_csv(String header, String datastring_for_csv, String outname){
    //File dataFile;      // File instance -- only used within this function

    /* If file doesn't exist, write header and data, otherwise only write data */
    if (!SD.exists(outname)){
        dataFile = SD.open(outname, FILE_WRITE);
        if (dataFile){
            dataFile.println(header);       // write header to file
            dataFile.println(datastring_for_csv);       // write data to file
        }
        dataFile.close();       // make sure the file is closed
    } else {
        dataFile = SD.open(outname, FILE_WRITE);
        if (dataFile) {
            dataFile.println(datastring_for_csv);       // write data to file
        }
        dataFile.close();       // make sure the file is closed
    }
}

/**
 * reads battery voltage, returns in volts
 * if using a board other than Feather M0 Adalogger, check documentation for battery read pin
 * TODO: documentation for changing preset pins for a different board
*/
float RemoteLogger::sample_batt_v(){
   pinMode(vbatPin, INPUT);
   float batt_v = (analogRead(vbatPin) * 2 * 3.3) / 1024;      // preset conversion to volts
   return batt_v; 
}

/**
 * sample amount of RAM (memory) available on board

*/
int RemoteLogger::sample_memory(){
    return freeMemory();
}

/**
 * alert TPL done
 * use A0 as output on Feather M0 Adalogger (only analog output)
 * TODO: set A0 to low in setup code first thing to avoid alerting prematurely?
*/
void RemoteLogger::tpl_done(){
    pinMode(tplPin, OUTPUT);       // just in case
    digitalWrite(tplPin, LOW); delay(50); digitalWrite(tplPin, HIGH); delay(50);
    digitalWrite(tplPin, LOW); delay(50); digitalWrite(tplPin, HIGH); delay(50);
    digitalWrite(tplPin, LOW); delay(50); digitalWrite(tplPin, HIGH); delay(50);
    digitalWrite(tplPin, LOW); delay(50); digitalWrite(tplPin, HIGH); delay(50);
}

/**
 * remove datalogging and tracking files from the SD card
 */
void RemoteLogger::wipe_files(){
    SD.remove("/TRACKING.csv");
    SD.remove("/DATA.csv");
    SD.remove("/HOURLY.csv");
}




/* TRACKING */

/**
 * increment the counter tracking how many samples have been taken since the last write to hourly
 * this counter should not exceed 4 for 15min TPL interval
 * use num_samples to access counter value
*/
void RemoteLogger::increment_samples(){
    write_to_csv("n", "1", "/TRACKING.csv");
}

/**
 * access counter of samples since last write to hourly
 * should not exceed 4 for 15min TPL interval
 * use increment_samples to increment counter
*/
int RemoteLogger::num_samples(){
    CSV_Parser cp("s", true, ',');     //for reading from TRACKING.csv
    cp.readSDfile("/TRACKING.csv");
    return cp.getRowsCount() - 1; //don't count the header
}

/**
 * access counter of number of hourly samples waiting to be transmitted
*/
int RemoteLogger::num_hours(){
    String csv_setting = produce_csv_setting();
    char buf[csv_setting.length()+1];
    csv_setting.toCharArray(buf, csv_setting.length()+1);

    CSV_Parser cp(buf, true, ',');
    cp.readSDfile("/HOURLY.csv");

    return cp.getRowsCount();
}

/**
 * set sample counter to zero
 * counts samples taken since a write to hourly
*/
void RemoteLogger::reset_sample_counter(){
    SD.remove("/TRACKING.csv");
}

/**
 * set hourly counter to zero
 * counts hourly samples waiting to be transmitted
 * warning: resetting this counter deletes all data stored in hourly file
*/
void RemoteLogger::reset_hourly(){
    SD.remove("/HOURLY.csv");
}




/* TELEMETRY */

/**
 * send provided message over the Iridium network
 * connect Iridium sleep pin (7 - grey) to pin 13 or change value of IridSlpPin
 * TODO: investigate -- did removing the Watchdog mess things up? try it with the TPL
*/
int RemoteLogger::send_msg(String myMsg){
    digitalWrite(IridSlpPin, HIGH);     // wake up the modem
    delay(2000);        // wait for RockBlock to power on

    IridiumSerial.begin(19200);     // Iridium serial at 19200 baud
    modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);

    int err = modem.begin();        // start up the modem
    if (err == ISBD_IS_ASLEEP){
        err = modem.begin();    // try to start up again
    }

    err = modem.sendSBDText(myMsg.c_str());    // try to send the message

    if (err != ISBD_SUCCESS) { // if unsuccessful try again
        err = modem.begin();
        err = modem.sendSBDText(myMsg.c_str());
    }

    // calibrate the RTC time roughly every 5 days
    /** TODO: will this miss days by accident? what if we miss noon? (i.e. only sending every two hours)*/
    /** TODO: do we need the pre/post time strings? */
    if (rtc.now().hour() == 12 & rtc.now().day() % 5 == 0) {
        sync_clock();
    }

    digitalWrite(IridSlpPin, LOW);      // put the modem back to sleep
    return err; 
}

/**
 * test the Iridium modem + connection by sending a message
 * sends "Hello world" + provided message
 * will print firmware version and test signal quality
 * if any functions fail it will leave the function immediately 
 * warning that this will attempt to send a message and use credits
 * prints status messages to Serial - does not return any status information from function
*/
void RemoteLogger::irid_test(String msg){
    digitalWrite(IridSlpPin, HIGH);         // turn on modem
    delay(2000);        // wait for modem to start up

    int signalQuality = -1;     // need this to pass in for signal quality query

    IridiumSerial.begin(19200);
    modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);

    /* begin satellite modem operation */
    Serial.println(" - starting modem...");
    int err = modem.begin();
    if (err != ISBD_SUCCESS) {
        Serial.print(" - begin failed: error ");
        Serial.println(err);
        if (err == ISBD_NO_MODEM_DETECTED) {
            Serial.println(" - no modem detected: check wiring.");
        }
        return;     // leave the function - no point in trying to send
    }

    /* print the firmware version */
    char version[12];
    err = modem.getFirmwareVersion(version, sizeof(version));
    if (err != ISBD_SUCCESS) {      // didn't get version info
        Serial.print(" - firmware version failed: error ");
        Serial.println(err);
        return;     // leave the test function
    }
    Serial.print(" - firmware version is ");
    Serial.print(version);
    Serial.println(".");

    /* get signal quality */
    int n = 0;
    while (n < 10) {    // test signal quality 10 times
        err = modem.getSignalQuality(signalQuality);    // query signal quality
        if (err != ISBD_SUCCESS) {
            Serial.print(" - signalQuality failed: error ");
            Serial.println(err);
            return;        // leave the test function
        }
        Serial.print(" - signal quality is currently ");
        Serial.print(signalQuality);
        Serial.println(".");
        n++;
        delay(1000);
    }

    /* send the message */
    Serial.print(" - Attempting: ");
    msg = "Hello world! " + msg;
    Serial.println(msg);
    err = modem.sendSBDText(msg.c_str());
    if (err != ISBD_SUCCESS) {
        Serial.print(" - sendSBDText failed: error ");
        Serial.println(err);
        if (err =- ISBD_SENDRECEIVE_TIMEOUT) {
            Serial.println(" - try again with a better view of the sky.");
        }
    } else {
        Serial.println(" - hey, it worked!");
    }

    /* sync clock to Iridium */
    Serial.println("Sync clock to Iridium");
    sync_clock();
    
}

/**
 * prepare message from hourly data file to send via Iridium to database
 * actual data values are multiplied by varying powers of 10 to remove decimals
 * see documentation for letter-to-header mappings and multipliers
 * 
 * e.g. 
 * Data file: 
 * datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm
 * 2001-01-10T01:11:05,4.31,24627,10,18.7,3
 * 
 * Message:
 * ABC:01011001:431,246,10,187,3:
*/
String RemoteLogger::prep_msg(){
    // process header to determine number of columns (parameters)
    // Serial.println(F("preparing csv setting..."));         /** TODO: remove */
    String csv_setting = produce_csv_setting();
    // Serial.println(csv_setting);    /** TODO: remove */

    SD.begin(chipSelect);       //start the SD card connection
    
    // Serial.println(freeMemory());       /** TODO: remove */
    // Serial.println(F("preparing CSV parser..."));      /** TODO: remove */
    char buf[csv_setting.length()+1];   Serial.println(1);  /** TODO: remove */
    csv_setting.toCharArray(buf, csv_setting.length()+1);  
    // Serial.println(freeMemory());       /** TODO: remove */
    CSV_Parser cp(buf, true, ',');      
    cp.readSDfile("/HOURLY.csv");       
    int num_rows = cp.getRowsCount();  

    // Serial.println(6);      /** TODO: remove */

    // Serial.println(F("preparing header index..."));        /** TODO: remove */
    // figure out where each parameter's info is in the dictionary
    // int **headerIndex;
    // headerIndex[num_params];
    // populate_header_index(headerIndex, num_params);

    // Serial.println(F("building message..."));      /** TODO: remove */
    // generate the letters
    String datastring_msg = "";
    datastring_msg.reserve(200);
    // for(int i = 0; i < num_params; i++){
    //     datastring_msg += LETTERS[*headerIndex[i]];
    // }

    datastring_msg += myLetters;
    datastring_msg += ":";

    int column = 0;        // position of the header we're on

    // Serial.println(F("date and time..."));     /** TODO: remove */
    //datetime (of first measurement in message)
    char **out_datetimes = (char **)cp[column];       // get list of datetimes 
    datastring_msg += String(out_datetimes[0]).substring(2,4); 
    datastring_msg += String(out_datetimes[0]).substring(5,7);
    datastring_msg += String(out_datetimes[0]).substring(8,10);
    datastring_msg += String(out_datetimes[0]).substring(11,13);
    datastring_msg += ":";

    delete out_datetimes;       // free up memory

    column++;

    float *floatOut;        //temporary variable for float columns to live in

    // Serial.println(F("battery voltage and memory..."));   /** TODO: remove */
    //battery voltage (most recent)
    floatOut = (float *)cp[column];
    datastring_msg += String(round(floatOut[num_rows-1] * BATT_MULT));
    datastring_msg += ":";

    column++;

    //free memory (most recent)
    floatOut = (float *)cp[column];
    datastring_msg += String(round(floatOut[num_rows-1] * MEM_MULT)); 
    datastring_msg += ":";

    // Serial.print(F("adding data... "));           /** TODO: remove */
    //sampled data
    for (int row = 0; row < num_rows; row++) {        // for each row in the data file
        column = 3;    //start each time at the start of the sampled data (fourth column)
        // Serial.print(F("row ")); Serial.print(row); Serial.print(F("   "));   /** TODO: remove */

        while (column < myParams+3) {      // for each column (sampled data point) in the row 
            // floatOut = (float *)cp[column];
            // datastring_msg += String(round(floatOut[row] * myMultipliers[column-3]));

            // if(column != myParams+2) { datastring_msg += ","; }           // add commas between data points
            // else { datastring_msg += ":"; }     // add a colon only to data from last column

            // manage selection of which parameters to send with multipliers
            if(multipliers[column-3] != 0){         // want to send this in the message
                floatOut = (float *)cp[column];
                datastring_msg += String(round(floatOut[row] * myMultipliers[column-3]));

                datastring_msg += ",";         // add commas between 
            }
            datastring_msg.setCharAt(datastring_msg.length()-1, ":");       // set the last character 

            column++;       //go to the next column
        }
    }
    // Serial.println();       /** TODO: remove */

    // free up memory
    delete floatOut;
    // delete headerIndex;
    
    return datastring_msg;
}




/* SAMPLING FUNCTIONS */

/**
 * sample from Hydros21 sensor
 * supply with bus and address of sensor 
 * 
 * bus: valid SDI12 bus initialized with the data pin attached to Hydros sensor, must have had begin() called already
 * sensor_address: SDI-12 address of the Hydros sensor, usually assumed to be 0
*/
String RemoteLogger::sample_hydros_M(SDI12 bus, int sensor_address){
    sdiResponse = "";
    myCommand = String(sensor_address) + "M!";       // first command to take a measurement

    bus.sendCommand(myCommand);
    delay(30);

    while (bus.available()) {
        char c = bus.read();
        if ((c != '\n') && (c != '\r')) {
            sdiResponse += c;
            delay(10);      // 1 character ~ 7.5ms
        }
    }

    /* clear buffer */
    if (sdiResponse.length() > 1) {
        bus.clearBuffer();
    }
    delay(2000);        // delay between taking reading and requesting data
    sdiResponse = "";    // clear response string (to get ready to read data)

    myCommand = String(sensor_address) + "D0!";         // string to request data from last measurement
    bus.sendCommand(myCommand);
    delay(30);      // wait for a response

    while (bus.available()) {
        char c = bus.read();
        if ((c != '\n') && (c != '\r')) {
            sdiResponse += c;
            delay(10);          // 1 character ~ 7.5ms
        }
    }

    sdiResponse = sdiResponse.substring(3);

    for (int i = 0; i < sdiResponse.length(); i++) {
        char c = sdiResponse.charAt(i);
        if (c == '+') {
            sdiResponse.setCharAt(i, ',');  // replace any + with ,
        }
    }

    /* clear buffer */
    if (sdiResponse.length() > 1) {
        bus.clearBuffer();
    }

    if (sdiResponse == ""){
        sdiResponse = "-9,-9,-9";   // no reading
    }

    bus.clearBuffer();

    return sdiResponse;
}

/**
 * sample measurements from OTT PLS
 * returns water level, water temp, ott status
 * 
 * bus: SDI12 object, has been started by the user with attached datapin
 * sensor_address: address of sensor, factory default 0 (see OTT docs to change)
 */
String RemoteLogger::sample_ott_M(SDI12 bus, int sensor_address){
    myCommand = String(sensor_address) + "M!";      // first command to take a measurement

    bus.sendCommand(myCommand);
    delay(30);

    while(bus.available()){     // build response string
        char c = bus.read();
        if ((c != '\n') && (c != '\r')) {
            sdiResponse += c;
            delay(10);      // 1 character = ~7.5 ms
        }
    }

    // clear buffer
    if(sdiResponse.length() > 1){
        bus.clearBuffer();
    }
    delay(2000);        // wait for sensor to take measurement
    sdiResponse = "";   // clear response string

    // next command to request data
    myCommand = String(sensor_address) + "D0!";

    bus.sendCommand(myCommand);
    delay(30);

    while(bus.available()){
        char c = bus.read();
        if ((c != '\n') && (c != '\r')) {
            sdiResponse += c;
            delay(10);      // 1 character = ~7.5 ms
        }
    }

    // subset string
    sdiResponse = sdiResponse.substring(3);

    // replace + with ,
    for (int i = 0; i < sdiResponse.length(); i++) {
        char c = sdiResponse.charAt(i);
        if (c == '+') {
            sdiResponse.setCharAt(i, ',');
        }
    }

    // clear buffer
    if(sdiResponse.length() > 1){
        bus.clearBuffer();
    }

    if(sdiResponse == ""){      // no data
        sdiResponse = "-9,-9,-9";
    }

    return sdiResponse;
}

/**
 * sample system test information 
 * returns relative humidity, dew (?), deg (?) of OTT - see docs
 * 
 * bus: SDI12 object, has been started by the user with attached datapin
 * sensor_address: address of sensor, factory default 0 (see OTT docs to change)
 */
String RemoteLogger::sample_ott_V(SDI12 bus, int sensor_address){
    myCommand = String(sensor_address) + "V!";      // first command to take a measurement

    bus.sendCommand(myCommand);
    delay(30);

    while(bus.available()){     // build response string
        char c = bus.read();
        if ((c != '\n') && (c != '\r')) {
            sdiResponse += c;
            delay(10);      // 1 character = ~7.5 ms
        }
    }

    // clear buffer
    if(sdiResponse.length() > 1){
        bus.clearBuffer();
    }
    delay(2000);        // wait for sensor to take measurement
    sdiResponse = "";   // clear response string

    // next command to request data
    myCommand = String(sensor_address) + "D0!";

    bus.sendCommand(myCommand);
    delay(30);

    while(bus.available()){
        char c = bus.read();
        if ((c != '\n') && (c != '\r')) {
            sdiResponse += c;
            delay(10);      // 1 character = ~7.5 ms
        }
    }

    // subset string
    sdiResponse = sdiResponse.substring(3);

    // replace + with ,
    for (int i = 0; i < sdiResponse.length(); i++) {
        char c = sdiResponse.charAt(i);
        if (c == '+') {
            sdiResponse.setCharAt(i, ',');
        }
    }

    // clear buffer
    if(sdiResponse.length() > 1){
        bus.clearBuffer();
    }

    if(sdiResponse == ""){      // no data
        sdiResponse = "-9,-9,-9";
    }

    return sdiResponse;
}

/**
 * sample all 6 parameters from OTT PLS
 * returns water level, water temp, status, RH, dew, deg 
 * corresponds to default OTT header (see docs)
 * 
 * bus: SDI12 object, has been started by the user with attached datapin
 * sensor_address: address of sensor, factory default 0 (see OTT docs to change)
 */
String RemoteLogger::sample_ott(SDI12 bus, int sensor_address){
    String sample;
    sample.reserve(30);
    String sample = sample_ott_M(bus, sensor_address) + "," + sample_ott_V(bus, sensor_address);

    return sample;
}

/**
 * sample turbidity from Analite 195 sensor
 * must be attached to two digital outputs (wiper set/unset) and one analog input (data pin)
 * the analog input pin does not need to be set to input 
 * digital pins must be set as output pins
 * 
 * wiper frequency is tied to hourly writes to the CSV
 * will run wiper once every four power cycles (once per hour with 15 min TPL)
 * 
 * analogDataPin: analog input for data read from Analite 195
 * wiperSetPin: digital output pin for wiper set on Analite 195 (see Analite docs for setup)
 * wiperUnsetPin: digital output pin for wiper unset on Analite 195 (see Analite docs for setup)
 */
String RemoteLogger::sample_analite_195(int analogDataPin, int wiperSetPin, int wiperUnsetPin){
    // set up pins in case the user didn't
    pinMode(wiperSetPin, OUTPUT);
    pinMode(wiperUnsetPin, OUTPUT);

    analogReadResolution(12);
    float values[10];

    // check how many lines in tracker file
    int samplesSinceHourly = num_samples();
    if (samplesSinceHourly == 4){     // it's been an hour -- time to wipe
        digitalWrite(wiperSetPin, HIGH); delay(150); 
        digitalWrite(wiperSetPin, LOW); delay(50):
        digitalWrite(wiperUnsetPin, HIGH); delay(50);
        digitalWrite(wiperUnsetPin, LOW); delay(50); delay(14000);      // wait for wipe cycle - 6 seconds ish
    } else {    // not wiping, just make sure it's off
        digitalWrite(wiperUnsetPin, HIGH); delay(20);
        digitalWrite(wiperUnsetPin, LOW); delay(20);
    }

    // sample 10 values from sensor
    for(int i = 0; i < 10; i++){
        values[i] = (float)analogRead(analogDataPin);       // read from probe
        delay(5);
    }

    float medTurbAlog = stats.median(values, 10);       // compute median 12-bit analog value

    // convert from analog value to NTU with provided calibration coefficients
    /** TODO: this just reads it straight across - need to add the calibration stuff */
    float ntuAnalog = medTurbAlog;
    int ntuInt = round(ntuAnalog);      // round to an integer

    analogReadResolution(10);       /** TODO: what is this useful for? */

    return String(ntuInt);
}

/**
 * sample range from MaxBotix MB7369 ultrasonic ranger
 * attach to 2 digital outputs and 1 digital input 
 * use a PWM pin for input (all pins but A1, A5 on Feather M0 Adalogger, see docs for other boards)
 * returns minimum of 10 samples
 * 
 * powerPin: digital output controlling ranger power, see ranger docs for setup
 * triggerPin: digital output controlling ranger active time, see ranger docs for setup
 * pulseInputPin: digital input to collect data, see ranger docs for setup
 * 
 * TODO: should this be returned as a string for continuity? or left as long for memory efficiency?
 */
long RemoteLogger::sample_ultrasonic(int powerPin, int triggerPin, int pulseInputPin){
    pinMode(powerPin, OUTPUT);
    digitalWrite(powerPin, HIGH); delay(500);   // turn on the ranger

    // set up pins
    pinMode(triggerPin, OUTPUT);
    pinMode(pulseInputPin, INPUT);

    float values[10];
    digitalWrite(triggerPin, HIGH);     // start the ranger
    delay(30);

    for(int i = 0; i < 10; i++){
        int32_t duration = pulseIn(pulseInputPin, HIGH);    // get pulse duration -- time of flight
        values[i] = duration;
        delay(150);     // don't sample too quickly < 7.5Hz
    }
    digitalWrite(triggerPin, LOW);      // stop the ranger

    long minDistance = stats.minimum(values, 10);       // get the minimum of the sampled values

    digitalWrite(powerPin, LOW); delay(50);     // turn off the ranger
    return minDistance;
}




/* PRIVATE HELPERS */

/**
 * helper function 
 * sync RTC to system time from Iridium RockBlock modem
*/
void RemoteLogger::sync_clock(){
    struct tm t;
    int err_time = modem.getSystemTime(t);
    if (err_time == ISBD_SUCCESS) {
        String pre_time = rtc.now().timestamp();
        Serial.print("pretime: "); Serial.println(pre_time);
        rtc.adjust(DateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec));
        String post_time = rtc.now().timestamp();
        Serial.print("posttime: "); Serial.println(post_time);
    }
}

/**
 * helper function
 * generate string for argument to CSV parser 
 * format e.g. "sffff" for a string column and four float columns
 * 
 * TODO: could change this so it directly produces a char array rather than a String -- convert it every time anyway
*/
String RemoteLogger::produce_csv_setting(){
    String setting = "s";
    for (int i = 1; i < myParams+3; i++){
        setting += "f";
    }
    return setting;
}




/* PIN SETTERS */

/**
 * LED pin
 * default pin 8 - built-in green LED on Feather M0 Adalogger 
 * can modify if additional LEDs are added 
*/
void RemoteLogger::setLedPin(byte pin){ ledPin = pin; }

/**
 * input for checking battery voltage
 * pin 9 on Feather M0 Adalogger (check docs for other boards)
*/
void RemoteLogger::setBattPin(byte pin){ vbatPin = pin; }

/**
 * TPL done pin 
 * pin A0 on Feather M0 Adalogger - only analog output pin
 * check docs for other boards (need analog output)
*/
void RemoteLogger::setTplPin(byte pin){ tplPin = pin; }

/**
 * Iridium sleep pin (grey - pin 7)
 * default pin 13 - modify if changing wiring
*/
void RemoteLogger::setIridSlpPin(byte pin){ IridSlpPin = pin; }

/**
 * chip select pin for SD card
 * pin 4 on Feather M0 Adalogger (check docs for other boards)
*/
void RemoteLogger::setSDSelectPin(byte pin){ chipSelect = pin; }
