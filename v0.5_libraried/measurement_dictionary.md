## Dictionary for measurements in messages:

 Measurement | Units | Code | Header Name | Sensors
 ----------- | ----- | ---- | ----------- | -------
 Water Level | mm | A | water_level_mm | Ultrasonic, Hydros21, OTT
 Water Temp | ⁰C | B | water_temp_c | OTT, Hydros21, DS18B20
 Electrical Conductivity | dS/m | C | water_ec_dcm | Hydros21
 Turbidity | NTU | D | ntu | Analite195
 Snow Depth | mm | E | snow_depth_mm | Ultrasonic
 Air Temp @ 2m | ⁰C | F | air_2m_temp_deg_c | SHT31
 Relative Humidity @ 2m | % | G | air_2m_temp_rh_prct | SHT31

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


