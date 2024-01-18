
#include <WeatherStation.h>     // custom library

/*Define global vars */
String my_letter = "ABC"; //depends on sensors (what we're measuring) - order matters
String my_header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";

/*Define Iridium seriel communication as Serial1 */
#define IridiumSerial Serial1

/*SDI-12 sensor address, assumed to be 0*/
#define SENSOR_ADDRESS 0

/*Create library instances*/
WeatherStation ws(my_letter, my_header);                // instance of WeatherStation class

void setup(void){
  ws.begin();
}

void loop(void){
  ws.run();
}

