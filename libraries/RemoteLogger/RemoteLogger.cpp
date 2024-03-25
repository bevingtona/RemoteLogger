/*
Version 0.1 of RemoteLogger library for modular remote data loggers (originally for hydrometric sensors)
Author: Rachel Pagdin
January 25, 2024
*/

#include "Arduino.h"
#include "RemoteLogger.h"

// no-argument constructor (for testing purposes)
RemoteLogger::RemoteLogger(){
    // global to control amount of memory used for reassigning these all the time
    myCommand = "";
    sdiResponse = "";

    // constants 
    blink_freq_s = 10;
    watchdog_timer = 30000;

    // global 
    analite_wiper_cnt = 0; 
}

//constructor
RemoteLogger::RemoteLogger(String letters, String header){

    // set message preamble (letters/header)
    my_letter = letters;
    my_header = header;
    
    // global to control amount of memory used for reassigning these all the time
    myCommand = "";
    sdiResponse = "";

    // constants 
    blink_freq_s = 10;
    watchdog_timer = 30000;

    // global 
    analite_wiper_cnt = 0; 
}


//constructor with sensor list
RemoteLogger::RemoteLogger(String header, char sensor_names[][12]){
    // sensors = end(sensor_names) - begin(sensor_names);  //number of sensors = length of names list
    sensors = 2; /** TODO: this is bad do not have constants in here */

    snames = sensor_names;

    /* build letters for message header */
    my_letter = "";
    String temp = "";
    /** TODO: make this force alphabetical order (could maybe just sort after?)*/
    for(int i = 0; i < length; i++){
        temp = snames[i];
        if (temp == HYDROS) {my_letter += HYDROS_DATA;}
        else if (temp == ANALITE) {my_letter += ANALITE_DATA;}
    }


    /* build header */
    // header based on letters in the letter code (my_letter)
    // start with "datetime,batt_v,memory" --> add on to that
    my_header = header;
}


/**
 * Standard startup
 * start SDI-12
 * check RTC and SD card
*/
void RemoteLogger::start_checks(){
    // START SDI-12 PROTOCOL
    start_data_bus();

    // CHECK RTC
    check_clock();

    // CHECK SD CARD
    check_card();
}

/**
 * Individual start/check functions 
 * if calling all three can use start_checks()
*/
void RemoteLogger::start_data_bus(){
    Serial.println(" - check sdi12");
    mySDI12.begin();
}
void RemoteLogger::check_card(){
    Serial.println(" - check card");
    while (!SD.begin(SD_CHIP_SELECT_PIN)) { blinky(2, 200, 200, 2000); }
}
void RemoteLogger::check_clock(){
    Serial.println(" - check clock");
    while (!rtc.begin()) { blinky(1, 200, 200, 2000); }
}


void RemoteLogger::read_params(){
    //burner variables (delete at end)
    int16_t *sample_freq_m;
    int16_t *irid_freq_h;
    char **test_mode;
    int16_t *onstart_samples;

    CSV_Parser cp("ddsd", true, ',');
    Serial.println(" - check param.txt");
    while(!cp.readSDfile("/PARAM.txt")) { blinky(3, 200, 200, 1000); } //blink while reading the file from SD
    cp.parseLeftover();

    /* Assign to class member variables (public) */
    sample_freq_m = (int16_t *) cp["sample_freq_m"];
    sample_freq_m_16 = sample_freq_m[0];
    irid_freq_h = (int16_t *)cp["irid_freq_h"];
    irid_freq_h_16 = irid_freq_h[0];
    test_mode = (char **)cp["test_mode"];
    test_mode_string = String(test_mode[0]);
    onstart_samples = (int16_t *)cp["onstart_samples"];
    onstart_samples_16 = onstart_samples[0];

    /* Get rid of all the stuff we don't need (save space) */
    delete sample_freq_m;
    delete irid_freq_h;
    delete test_mode;
    delete onstart_samples;
}

void RemoteLogger::blinky(int16_t n, int16_t high_ms, int16_t low_ms, int16_t btw_ms){
    for(int i = 1; i <= n; i++){
        digitalWrite(LED_PIN, HIGH);
        delay(high_ms);
        digitalWrite(LED_PIN, LOW);
        delay(low_ms);
    }
    delay(btw_ms);
}

void RemoteLogger::write_to_csv(String header, String datastring_for_csv, String outname){
    //instance of File object - used to be global
    File dataFile;

    // IF FILE DOES NOT EXIST, WRITE HEADER AND DATA, ELSE, WRITE DATA
    if (!SD.exists(outname)){  //Write header if first time writing to the logfile
    
        dataFile = SD.open(outname, FILE_WRITE);  //Open file under filestr name from parameter file
        if (dataFile) {
            dataFile.println(header);
            dataFile.println(datastring_for_csv);
        }
        dataFile.close();  //Close the dataFile
    } else { //file already exists (don't need to write header)
         dataFile = SD.open(outname, FILE_WRITE);
        if (dataFile) {
            dataFile.println(datastring_for_csv);
            dataFile.close();
        }
  }
}

/**
 * TODO: specific to hydros --> need to take out of library
*/
String RemoteLogger::prep_msg(){

    SD.begin(chipSelect);
    CSV_Parser cp("sfffff", true, ',');  // Set paramters for parsing the log file
    cp.readSDfile("/HOURLY.csv"); //open CSV file of hourly readings 
    int num_rows = cp.getRowsCount();  //Get # of rows
    
    //get information from the CSV  
    char **out_datetimes = (char **)cp["datetime"];
    float *out_mem = (float *)cp["memory"];
    float *out_batt_v = (float *)cp["batt_v"];
    float *out_water_level_mm = (float *)cp["water_level_mm"];
    float *out_water_temp_c = (float *)cp["water_temp_c"];
    float *out_water_ec_dcm = (float *)cp["water_ec_dcm"];
  
    //format the message according to message pattern (see docs)
    String datastring_msg = 
        my_letter + ":" +
        String(out_datetimes[0]).substring(2, 4) + 
        String(out_datetimes[0]).substring(5, 7) + 
        String(out_datetimes[0]).substring(8, 10) + 
        String(out_datetimes[0]).substring(11, 13) + ":" +
        String(round(out_batt_v[num_rows-1] * 100)) + ":" +
        String(round(out_mem[num_rows-1] / 100)) + ":";
  
    //attach the readings to the message preamble
    for (int i = 0; i < num_rows; i++) {  //For each observation in the IRID.csv
        datastring_msg = 
        datastring_msg + 
        String(round(out_water_level_mm[i])) + ',' + 
        String(round(out_water_temp_c[i]*10)) + ',' + 
        String(round(out_water_ec_dcm[i])) + ':';              
    }

    return datastring_msg;
}

float RemoteLogger::sample_batt_v(){
    pinMode(vbatPin, INPUT);
    float batt_v = (analogRead(vbatPin) * 2 * 3.3) / 1024;
    return batt_v;
}


String RemoteLogger::sample(String sensor_name){
    if (sensor_name==EMPTY){
        return "";
    }
    else if (sensor_name==HYDROS){
        return sample_hydros_M();
    }
    else if (sensor_name==ANALITE){
        return sample_analite_195();
    }
}


/**
 * M: SDI-12 communication protocol command (measure)
*/
String RemoteLogger::sample_hydros_M(){

    digitalWrite(HYDROS_SET_PIN, HIGH); delay(50);
    digitalWrite(HYDROS_SET_PIN, LOW); delay(1000);

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

    while (mySDI12.available()) {  // build string from response (coming character by character along data bus)
        char c = mySDI12.read();
        if ((c != '\n') && (c != '\r')) {
            sdiResponse += c;
            delay(10);  // 1 character ~ 7.5ms
        }
    }

    sdiResponse = sdiResponse.substring(3); //cut off the first 3 characters

    //replace all + with ,
    for (int i = 0; i < sdiResponse.length(); i++) {
        char c = sdiResponse.charAt(i);
        if (c == '+') {
            sdiResponse.setCharAt(i, ',');
        }
    }

    // clear buffer
    if (sdiResponse.length() > 1)
        mySDI12.clearBuffer();

    if (sdiResponse == "")
        sdiResponse = "-9,-9,-9"; // no reading

    digitalWrite(HYDROS_UNSET_PIN, HIGH); delay(50);
    digitalWrite(HYDROS_UNSET_PIN, LOW); delay(50);

    return sdiResponse;
}

/**
 * M: protocol command (first three)
*/
String RemoteLogger::sample_ott_M(){

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

    if(sdiResponse == "")
        sdiResponse = "-9,-9,-9";
    
    return sdiResponse;
}


/**
 * V: protocol command (next three measurements)
*/
String RemoteLogger::sample_ott_V(){

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
    
    if(sdiResponse == "")
        sdiResponse = "-9,-9,-9";

    return sdiResponse;
}

/**
 * Analite 105 measures turbidity in NTU (Nephelometric Turbidity Units)
*/
String RemoteLogger::sample_analite_195(){

    analogReadResolution(12);

    float values[10];  //Array for storing sampled distances

    if (analite_wiper_cnt >= 5) {  // Probe will wipe after 6 power cycles (1 hr at 10 min interval)

        digitalWrite(ANALITE_WIPER_SET_PIN, HIGH); delay(50); delay(100);
        digitalWrite(ANALITE_WIPER_SET_PIN, LOW); delay(50); 
        digitalWrite(ANALITE_WIPER_UNSET_PIN, HIGH); delay(50); 
        digitalWrite(ANALITE_WIPER_UNSET_PIN, LOW); delay(50); delay(14000);  // wait for full rotation (about 6 seconds)

        analite_wiper_cnt = 0;  // Reset wiper count to zero
    
    } else {
        analite_wiper_cnt++;
        digitalWrite(ANALITE_WIPER_UNSET_PIN, HIGH); delay(20);  // Unnecessary, but good to double check it's off
        digitalWrite(ANALITE_WIPER_UNSET_PIN, LOW); delay(20);
    }

    for (int i = 0; i < 10; i++) {
        values[i] = (float)analogRead(ANALITE_TURB_ANALOG);  // Read analog value from probe
        delay(5);
    }

    float med_turb_alog = stats.median(values, 10);  // Compute median 12-bit analog val

    //Convert analog value (0-4096) to NTU from provided linear calibration coefficients
    float ntu_analog = med_turb_alog;  //(m * med_turb_alog) + b;

    int ntu_int = round(ntu_analog);

    analogReadResolution(10); //back to default

    return String(ntu_int);
}

/**
 * Ultrasonic ranging (distance)
*/
long RemoteLogger::sample_ultrasonic(){
    int n = 10; //number of samples

    pinMode(ULTRASONIC_TRIGGER_PIN, OUTPUT);       //Set ultrasonic ranging trigger pin as OUTPUT
    pinMode(ULTRASONIC_PULSE_PIN, INPUT);          //Set ultrasonic pulse width pin as INPUT
    
    float values[n];          //Array for storing sampled distances
    
    digitalWrite(ULTRASONIC_TRIGGER_PIN, HIGH);  //Write the ranging trigger pin HIGH
    delay(30);    
    for (int16_t i = 0; i < n; i++)  //Take N samples
    {
        int32_t duration = pulseIn(ULTRASONIC_PULSE_PIN, HIGH);  //Get the pulse duration (i.e.,time of flight)
        values[i] = duration;
        delay(150);  //Dont sample too quickly < 7.5 Htz
    }
    long med_distance = stats.minimum(values, n);  //Copute median distance
    digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);  //Write the ranging trigger pin LOW
    return med_distance;
}

/**
 * specific to hydros --> need to take out of library
*/
// String RemoteLogger::take_measurement(){
//     digitalWrite(SensorSetPin, HIGH); delay(50);
//     digitalWrite(SensorSetPin, LOW); delay(1000);
  
//     String msmt = String(sample_batt_v()) + "," + 
//         freeMemory() + "," + 
//         sample_hydros_M();

//     digitalWrite(SensorUnsetPin, HIGH); delay(50);
//     digitalWrite(SensorUnsetPin, LOW); delay(50);

//     return msmt;
// }

String RemoteLogger::take_measurement(){

    String msmt = String(sample_batt_v()) + "," +
        freeMemory();

    for(int i = 0; i < sensors; i++){
        msmt += ',' + sample(snames[i]);
    }

    return msmt;
}

void RemoteLogger::irid_test(String msg){
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

int RemoteLogger::send_msg(String my_msg){
    digitalWrite(IridPwrPin, HIGH);  //Drive iridium power pin LOW
    delay(2000);

    IridiumSerial.begin(19200);                            // Start the serial port connected to the satellite modem
    modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);  // This is a low power application
    //modem is instance of IridiumSBD

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
  
    //if it doesn't send try again
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

    //update local time (RTC) to time from Iridium
    // Serial.print(" time");
    if(rtc.now().hour() == 12 & rtc.now().day() % 5 == 0){
        struct tm t;
        int err_time = modem.getSystemTime(t);
        if (err_time == ISBD_SUCCESS) {
            String pre_time = rtc.now().timestamp();
            rtc.adjust(DateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec));
            String post_time = rtc.now().timestamp();
        }
    }
  
    digitalWrite(IridPwrPin, LOW);  //Drive iridium power pin LOW
    return err;    
}




/* SENSOR CLASS */
// Sensor::Sensor(){
//     name = EMPTY;
// }

// Sensor::Sensor(String sensor_name){
//     /** TODO: add some checks in here to see if the entered names are actually in the allowed names*/

//     name = sensor_name;

//     //assign header
//     assign_headers();
// }

// String Sensor::sample(){
//     if (name==EMPTY){
//         return "";
//     }
//     else if (name==HYDROS){
//         return sample_hydros_M();
//     }
//     else if (name==ANALITE){
//         return sample_analite_195();
//     }
// }


// void Sensor::assign_headers(){
//     //assign header
//     if (name==HYDROS) {data_points = HYDROS_DATA;}
//     else if (name==ANALITE) {data_points = ANALITE_DATA;}
// }