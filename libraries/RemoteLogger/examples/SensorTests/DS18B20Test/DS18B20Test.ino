/**
 * sample continuously from DS18B20 temperature sensor and display to Serial
 * 
 * Author: Rachel Pagdin
 * June 17, 2024
 */

#include <RemoteLogger.h>

const int dataPin = 12;                     // connect DS18B20 data line to this digital pin
const int sensorIndex = 0;                  // temp sensor we want is at 0 by default
OneWire oneWire(dataPin);                   // set up a OneWire connection on the connected digital pin
DallasTemperature sensors(&oneWire);        // set up an array of sensors (in this case just one sensor)

RemoteLogger logger{};

void setup(){
    delay(50);
    Serial.begin(115200);
    delay(50);

    logger.begin();

    pinMode(dataPin, INPUT);
    sensors.begin();
}

void loop(){
    String temperature = logger.sample_DS18B20(sensors, sensorIndex);
    Serial.println(temperature);

    delay(3000);        // sample once every 3 seconds
}