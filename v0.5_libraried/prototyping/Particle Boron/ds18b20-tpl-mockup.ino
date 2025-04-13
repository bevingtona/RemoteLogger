/**
 * sample from DS18B20 temperature sensor
 * publishes message through Particle (appears on console) that adheres to format set in RemoteLogger library
 * samples battery voltage and free RAM as well
 * 
 * note: must be compiled in Particle IDE (web-based) - libraries are different
 * 
 * Author: Rachel Pagdin
 * June 21, 2024
*/


// This #include statement was automatically added by the Particle IDE.
#include <DS18B20.h>

// Include Particle Device OS APIs
#include "Particle.h"
#include <time.h>


// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

const int dataPin = D4;
const int tplPin = A0;
const int led = D7;
DS18B20 sensor(dataPin, true);      // make a sensor object
FuelGauge fuel;

String header = "datetime,batt_v,memory,temp_c";
String letters = "A";


String take_measurement(){
    String msmt = String((int)fuel.getVCell()*100) + ":" +
        String((int)System.freeMemory()*0.01) + ":" +
        String((int)sensor.getTemperature()*100);
        
    return msmt;
}


void setup() {
  // set up all the stuff
  Serial.begin(115200);
  delay(50);
  
  Serial.println("starting up...");
  
  pinMode(dataPin, INPUT);
  
}


void loop() {
    // wake up - try to connect (time out and go back to sleep if you can't connect)
    Serial.println("checking the time");
    time_t presentTime = Time.now();
    String timeStamp = Time.format(presentTime, "%y%m%d%H");
    
    // sample (battery, memory, temperature)
    Serial.println("sampling...");
    String sample = take_measurement();
    Serial.print("sample: "); Serial.println(sample);
    
    // prep message to send
    Serial.println("prepping message...");
    String msg = letters + ":" + timeStamp + ":" + sample + ":";
    Serial.print("message: "); Serial.println(msg);
    
    // send (publish) message
    Particle.publish(msg);
    Serial.println("message sent, turning off");
    
    // turn off (send TPL done)
    // tpl_done();
    blinky(3, 500, 500, 500);
    while(1);
  
}


void tpl_done(){
    pinMode(tplPin, OUTPUT);
    
    digitalWrite(tplPin, LOW); delay(50); digitalWrite(tplPin, HIGH); delay(50);
    digitalWrite(tplPin, LOW); delay(50); digitalWrite(tplPin, HIGH); delay(50);
    digitalWrite(tplPin, LOW); delay(50); digitalWrite(tplPin, HIGH); delay(50);
    digitalWrite(tplPin, LOW); delay(50); digitalWrite(tplPin, HIGH); delay(50);
}


void blinky(int n, int high_ms, int low_ms, int btw_ms){
    pinMode(led, OUTPUT);
    
    for(int i = 0; i < n; i++){
        digitalWrite(led, HIGH);
        delay(high_ms);
        digitalWrite(led, LOW);
        delay(low_ms);
    }
    delay(btw_ms);
}