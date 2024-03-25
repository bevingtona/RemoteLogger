

#include "RemoteLogger.h"
#include <string>

String my_header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm,ntu";
char sensor_names[2][12] = {"hydros21", "analite195"};

RemoteLogger rl = RemoteLogger(my_header, sensor_names);

void setup() {
  // put your setup code here, to run once:
  pinMode(rl.LED_PIN, OUTPUT);

  rl.start_data_bus(); //start the SDI-12 data bus
}


void loop() {
  // put your main code here, to run repeatedly:
  rl.blinky(5, 100, 100, 300); //blinky test

  //power to hydros (thru relay)
  // digitalWrite(rl.HYDROS_SET_PIN, HIGH); delay(50);
  // digitalWrite(rl.HYDROS_SET_PIN, LOW); delay(1000);

  // take sample from hydros, print to console
  String message = rl.take_measurement();
  Serial.println(message);

  //cut power to hydros (thru relay)
  // digitalWrite(rl.HYDROS_UNSET_PIN, HIGH); delay(50);
  // digitalWrite(rl.HYDROS_UNSET_PIN, LOW); delay(50);
}
