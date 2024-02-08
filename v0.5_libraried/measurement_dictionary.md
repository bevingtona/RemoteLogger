## Dictionary for measurements in messages:

 Measurement | Units | Code | Header Name | Sensors
 ----------- | ----- | ---- | ----------- | -------
 Water Level | mm | A | water_level_mm | Ultrasonic, Hydros21, OTT
 Water Level (is this a typo?) | m | A | water_level_m | OTT 
 Water Temp | ⁰C | B | water_temp_c | OTT, Hydros21
 Electrical Conductivity | dS/m | C | water_ec_dcm | Hydros21
 Turbidity | NTU | D | ntu | Analite195
 Snow Depth | mm | E | snow_depth_mm | Ultrasonic

 <br>

### Other Measurements: 

#### Preamble
Measurement | Units | Header Name
----------- | ----- | -----------
Date/Time | | datetime
Battery Voltage | V | batt_v
Memory (used?) | | memory

#### Sensor-specific
Sensor | Measurement | Units | Header Name
------ | ----------- | ----- | -----------
OTT | Status | | ott_status
OTT | RH (?) | | ott_rh
OTT | Dew (?) | | ott_dew
OTT | Deg (temp?) | | ott_deg


