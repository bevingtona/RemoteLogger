/*
Version 0.1 of WeatherStation library for modular hydrometric weather stations 
Author: Rachel Pagdin
December 20, 2023
includes support for blinky function
*/

#include "Arduino.h"
#include "WeatherStation.h"
#include "CSV_Parser.h"

//constructor
WeatherStation::WeatherStation(String letters, String header){
    // values for pins on Feather M0
    chipSelect = 4;         // pin for SD card
    SensorSetPin = 5;       // Power relay set pin to HYDROS21
    SensorUnsetPin = 6;     // Power relay unset pin to HYDROS21
    led = 8;                // pin 8 is LED on Feather M0
    vbatPin = 9;            // batt pin
    dataPin = 12;           // pin for SDI-12 data bus
    IridPwrPin = 13;        // Power base PN222 2 transistor pin to Iridium modem

    // set message preamble (letters/header)
    my_letter = letters;
    my_header = header;

    // library instances assignment
    SDI12 mySDI12 = SDI12(dataPin); 
}

void WeatherStation::begin(){
    pinMode(led, OUTPUT);
}

void WeatherStation::read_params(){
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

void WeatherStation::blinky(int16_t n, int16_t high_ms, int16_t low_ms, int16_t btw_ms){
    for(int i = 1; i <= n; i++){
        digitalWrite(led, HIGH);
        delay(high_ms);
        digitalWrite(led, LOW);
        delay(low_ms);
    }
    delay(btw_ms);
}

void WeatherStation::write_to_csv(String header, String datastring_for_csv, String outname){
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

String WeatherStation::prep_msg(){

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

float WeatherStation::sample_batt_v(){
    pinMode(vbatPin, INPUT);
    float batt_v = (analogRead(vbatPin) * 2 * 3.3) / 1024;
    return batt_v;
}

String WeatherStation::sample_hydros_M(){
    //moved from global to local variables (only used here)
    String myCommand = "";
    String sdiResponse = "";

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

    return sdiResponse;
}

String WeatherStation::take_measurement(){
    digitalWrite(SensorSetPin, HIGH); delay(50);
    digitalWrite(SensorSetPin, LOW); delay(1000);
  
    String msmt = String(sample_batt_v()) + "," + 
        freeMemory() + "," + 
        sample_hydros_M();

    digitalWrite(SensorUnsetPin, HIGH); delay(50);
    digitalWrite(SensorUnsetPin, LOW); delay(50);

    return msmt;
}
