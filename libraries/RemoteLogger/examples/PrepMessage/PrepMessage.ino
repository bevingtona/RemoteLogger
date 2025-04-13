/**
 * prepare a message from the hourly data file to send over the Iridium connection
 * hourly data file must have at least one entry to produce meaningful message output
 * 
 * this example uses header settings for basic HYDROS21 with TPL (see Hydros21 example for full code)
 * see documentation to generate settings for different sensors and sampling configurations
 * 
 * things are working well if the message is prepared correctly the first time around
 * there may be memory bugs that disrupt further loops without clearing volatile memory
 * reset the board's power (RESET button or on/off switch) to run it through on empty volatile memory
 * 
 * Author: Rachel Pagdin
 * June 17, 2024
 */

#include <RemoteLogger.h>

String header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";      // header for CSV file
const byte num_params = 3;                          // number of sampled parameters (start counting after memory)
float multipliers[num_params] = {1, 10, 1};         // multipliers for parameters (in order) to remove decimals
String letters = "ABC";                             // letters for start of message, correspond to sampled parameters

RemoteLogger logger(header, num_params, multipliers, letters);

void setup(){
    delay(50);
    Serial.begin(115200);       // start Serial to see message
    while(!Serial);             // wait until Serial Monitor is open so you don't miss output

    delay(50);
    logger.begin();
}

void loop(){
    String message = logger.prep_msg();
    Serial.print(F("message: ")); Serial.println(message);

    delay(20000);
}