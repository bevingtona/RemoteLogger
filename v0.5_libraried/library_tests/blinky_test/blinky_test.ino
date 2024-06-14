/**
 * Test for blinky in RemoteLogger library v0.2
 * Author: Rachel Pagdin
 * June 12, 2024
 */

#include <RemoteLogger.h>

RemoteLogger logger{};      // default constructor
int ctr = 0;

void setup(){
  Serial.begin(9600);
  delay(50);

  logger.begin();
}

void loop(){
  logger.blinky(3,500,500,1000);
  ctr++;
  Serial.println(ctr);
}