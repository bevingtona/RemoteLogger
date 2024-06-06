/**
 * v0.2 of RemoteLogger library for modular remote data loggers
 * Author: Rachel Pagdin
 * June 4, 2024
*/

#include <Arduino.h>
#include <RemoteLogger.h>

/* CONSTRUCTORS AND STARTUP */

RemoteLogger::RemoteLogger(){
    modem(IridiumSerial);        // initialize Iridium object

    /** TODO: remove, this is default header for Hydros21 */
    myHeader = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";
}

RemoteLogger::RemoteLogger(String header){
    modem(IridiumSerial);       //initialize Iridium object

    myHeader = header;
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
    File dataFile;      // File instance -- only used within this function

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




/* TELEMETRY */

/**
 * send provided message over the Iridium network
 * connect Iridium sleep pin (7 - grey) to pin 13 or change value of IridSlpPin
 * TODO: investigate -- did removing the Watchdog mess things up? try it with the TPL
*/
int RemoteLogger::send_msg(String my_msg){
    digitalWrite(IridSlpPin, HIGH);     // wake up the modem
    delay(2000);        // wait for RockBlock to power on

    IridiumSerial.begin(19200);     // Iridium serial at 19200 baud
    modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);

    int err = modem.begin();        // start up the modem
    if (err == ISBD_IS_ASLEEP){
        err = modem.begin();    // try to start up again
    }

    err = modem.sendSBDText(my_msg.c_str());    // try to send the message

    if (err != ISBD_SUCCESS) { // if unsuccessful try again
        err = modem.begin();
        err = modem.sendSBDText(my_msg.c_str());
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
    int num_params = count_params(myHeader);
    String csv_setting = produce_csv_setting(num_params);

    SD.begin(chipSelect);       //start the SD card connection
    
    char buf[csv_setting.length()+1];
    csv_setting.toCharArray(buf, csv_setting.length()+1);
    CSV_Parser cp(buf, true, ',');
    cp.readSDfile("/HOURLY.csv");
    int num_rows = cp.getRowsCount();

    // figure out where each parameter's info is in the dictionary
    int **headerIndex;
    headerIndex[num_params];
    populate_header_index(headerIndex);

    // generate the letters
    String letters = "";
    for(int i = 0; i < num_params; i++){
        letters += LETTERS[*headerIndex[i]];
    }

    String datastring_msg = letters + ":";

    int column = 0;        // position of the header we're on
    String delim = ",";

    //datetime (of first measurement in message)
    char **out_datetimes = (char **)cp[column];       // get list of datetimes 
    datastring_msg = datastring_msg + 
        String(out_datetimes[0]).substring(2,4) +
        String(out_datetimes[0]).substring(5,7) +
        String(out_datetimes[0]).substring(8,10) +
        String(out_datetimes[0]).substring(11,13) + ":";

    column++;

    float *floatOut;        //temporary variable for float columns to live in

    //battery voltage (most recent)
    floatOut = (float *)cp[column];
    datastring_msg = datastring_msg + String(round(floatOut[num_rows-1] * MULTIPLIERS[*headerIndex[column]])) + ":";

    column++;

    //free memory (most recent)
    floatOut = (float *)cp[column];
    datastring_msg = datastring_msg + String(round(floatOut[num_rows-1] * MULTIPLIERS[*headerIndex[column]])) + ":";

    //sampled data
    for (int row = 0; row < num_rows; row++) {        // for each row in the data file
        column = 3;    //start each time at the start of the sampled data (fourth column)

        while (column < num_params) {      // for each column (sampled data point) in the row 
            floatOut = (float *)cp[column];
            datastring_msg = datastring_msg + String(round(floatOut[row] * MULTIPLIERS[*headerIndex[column]]));

            if(column != num_params-1) { datastring_msg += ","; }           // add commas between data points
            else { datastring_msg += ":"; }     // add a colon only to data from last column

            column++;       //go to the next column
        }
    }
    
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
    String sdiResponse = "";
    String myCommand = String(sensor_address) + "M!";       // first command to take a measurement

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
    if (sdiReponse.length() > 1) {
        bus.clearBuffer();
    }
    delay(2000);        // delay between taking reading and requesting data
    sdiReponse = "";    // clear response string (to get ready to read data)

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

    sdiReponse = sdiResponse.substring(3);

    for (int i = 0; i < sdiReponse.length(); i++) {
        char c = sdiReponse.charAt(i);
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

    return sdiResponse;
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
        rtc.adjust(DateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec));
        String post_time = rtc.now().timestamp();
    }
}

/**
 * helper function
 * count number of comma-separated parameters in header for CSV file
*/
int RemoteLogger::count_params(String header){
    if (header.length() < 1) { return 0; }          // no parameters - empty string

    int ctr = 1, index, prevIndex;
    String delim = ",";

    index = header.indexOf(delim);
    while(index != -1){
        ctr++;
        prevIndex = index;
        index = header.indexOf(delim, prevIndex+1);
    }

    return ctr;
}

/**
 * helper function
 * generate string for argument to CSV parser 
 * format e.g. "sffff" for a string column and four float columns
*/
String RemoteLogger::produce_csv_setting(int n){
    String setting = "s";
    for (int i = 1; i < n; i++){
        setting += "f";
    }
    return setting;
}

/**
 * helper function
 * headerIndex array contains the index for metadata about the header title in the library dictionary (set of arrays)
 * e.g. header "water_level_mm" at index 0 in dictionary, stored at index 3 in header 
 * so headerIndex array would contain value 0 at position 3
*/
void RemoteLogger::populate_header_index(int **headerIndex){
    int index, prevIndex = -1, temp;
    String delim = ",", columnName;

    for(int column = 0; column < num_params-1; column++) {
        index = myHeader.indexOf(delim, prevIndex+1);       //find first comma
        columnName = myHeader.substring(prevIndex+1, index);        //first column name      
        *headerIndex[column] = find_key(columnName);            //find address of this column header in dictionary

        prevIndex = index;
    }
    index = myHeader.indexOf(delim, prevIndex+1);
    columnName = header.substring(prevIndex+1);     //last column name
    *headerIndex[num_params-1] = find_key(columnName);
}

/**
 * helper function
 * find index location of key (column header name) in dictionary headers array
*/
int RemoteLogger::find_key(String key){
    for (int i = 0; i < TOTAL_KEYS; i++){
        if (HEADERS[i] == key){
            return i;       // return index where key was found
        }
    }
    return -1;    //not found
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

/**
 * first input data pin for SDI-12
 * may add another for second SDI-12 device
 * default pin 12 - modify if changing wiring
*/
//void RemoteLogger::setDataPin(byte pin){ dataPin = pin; }