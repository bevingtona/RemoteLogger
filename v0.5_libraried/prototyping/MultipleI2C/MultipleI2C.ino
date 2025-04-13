/**
 * interact with two devices over the same I2C bus
 * devices: Sparkfun ZED-F9P RTK GNSS, Sparkfun Distance VL53L1X
 * 
 * written from examples from GNSS library and VL53L1X library
 * works the same whether the two sensors are hardwired into SDA/SCL in parallel or strung along with Qwiic cable
 * 
 * Author: Rachel Pagdin
 * June 20, 2024
 */

#include <Wire.h>           // for I2C communication
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>       // GNSS
#include <SparkFun_VL53L1X.h>                           // distance sensor

SFE_UBLOX_GNSS myGNSS;
SFEVL53L1X distanceSensor;


void setup(){
    delay(50);
    Serial.begin(115200);       
    while(!Serial);     // wait for user to open serial monitor
    delay(50);

    Wire.begin();       // begin the I2C bus

    // start the GNSS
    if (myGNSS.begin(Wire, 0x42) == false){
        Serial.println(F("GNSS not detected at default I2C address"));       // default address 0x42 (can change - see GNSS lib example 9)
        while(1);
    }
    Serial.println(F("GNSS online!"));
    myGNSS.setI2COutput(COM_TYPE_UBX);          // cut out NMEA noise
    myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT);          // save only to communications port settings to flash and BBR

    // start the distance sensor
    if (distanceSensor.begin() != 0) {
        Serial.println(F("distance sensor failed to begin - check wiring"));        // default address 0x29 (can't change)
        while(1);
    }
    Serial.println(F("distance sensor online!"));
}

void loop(){
    // measure from GNSS
    long latitude = myGNSS.getLatitude();
    long longitude = myGNSS.getLongitude();
    long altitude = myGNSS.getAltitude();
    byte SIV = myGNSS.getSIV();         // satellites in view

    // measure distance
    distanceSensor.startRanging();
    while (!distanceSensor.checkForDataReady()) {
        delay(1);       // wait for data to be ready
    }
    int distance = distanceSensor.getDistance();
    distanceSensor.clearInterrupt();
    distanceSensor.stopRanging();

    // display to Serial
    Serial.print(F("Lat, long, alt, SIV: ")); 
    Serial.print(latitude); Serial.print(F(", "));
    Serial.print(longitude); Serial.print(F(", "));
    Serial.print(altitude); Serial.print(F(", "));
    Serial.println(SIV); 
    Serial.print(F("Distance (mm): ")); Serial.println(distance);
    Serial.println();

    delay(5000);        // 5 seconds between measurements
}