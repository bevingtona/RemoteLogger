/**
 * read distance from SparkFun distance ranging VL53L1X 4m ranger
 * written from examples in library (by Nathan Seidle)
 * 
 * June 19, 2024
 */

#include <Wire.h>                   // for I2C communication with distance sensor
#include <SparkFun_VL53L1X.h>       // library from https://github.com/sparkfun/SparkFun_VL53L1X_Arduino_Library

SFEVL53L1X distanceSensor;      // instance for distance sensor

void setup(void) {
    delay(50);
    Wire.begin();       // initiate I2C communication

    delay(50);
    Serial.begin(115200);
    while(!Serial);           // wait for serial monitor to be open
    Serial.println("VL53L1X Qwiic Test");

    if (distanceSensor.begin() != 0){ //Begin returns 0 on a good init
        Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
        while (1);
    }
    Serial.println("Sensor online!");
}

void loop(void) {
    distanceSensor.startRanging(); //Write configuration bytes to initiate measurement

    while (!distanceSensor.checkForDataReady()){        // wait for data to be ready to read
            delay(1);
    }

    int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
    distanceSensor.clearInterrupt();
    distanceSensor.stopRanging();

    Serial.print("Distance(mm): ");
    Serial.print(distance);

    // convert distance to feet
    float distanceInches = distance * 0.0393701;
    float distanceFeet = distanceInches / 12.0;
    Serial.print("\tDistance(ft): ");
    Serial.print(distanceFeet, 2);

    Serial.println();

    delay(3000);            // measure every 3 seconds
}