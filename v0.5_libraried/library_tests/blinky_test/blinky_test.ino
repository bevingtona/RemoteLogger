/*
test sketch for blinky in WeatherStation library
Dec 20, 2023
*/

#include <WeatherStation.h>

WeatherStation rl; //note: no parentheses needed if there are no parameters

void setup() {
  // put your setup code here, to run once:
  rl.begin(); 
}

void loop() {
  // put your main code here, to run repeatedly:
  rl.blinky(5,500,200,1000);

  rl.blinky(3,100,300,1000);
}
