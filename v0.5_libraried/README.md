# Version 0.5 - Adding Libraries

### Note: files in libraries/WeatherStation are the current files for the WeatherStation library applied across the .ino files. Do not change the name of the libraries or WeatherStation directories


Author: Rachel Pagdin


## Library add log: 

### Jan 24-25, 2024:
- constants for pin numbers (specific to board --> needs to be documented)
- general_purpose_ott.ino compiled with more reasonable flexibility of library (as opposed to general_purpose_hydros, which is over-librarified)
- added sample_analite_95 to library --> compiled successfully in general_purpose_hydros_analite.ino
- added sample_ultrasonic to library --> compiled successfully in general_purpose_ultrasonic_sleep.ino
- got rid of overspecific begin() and run() functions --> fixed general_purpose_hydros.ino

Notes:
- may eventually be able to force implementation of necessary functions (i.e. take_measurement, prep_msg) using inheritance --> too much coupling? better to use params to select measurements to take?
- leaving loop mainly user-determined --> not much benefit in adding to library
- prep_msg should be standardized in the library at some point
    - need to determine how to customize measurements
- ultrasonic ranger does not use SDI-12 data bus for measurements --> made separate functions to start SDI-12, check SD card, and check real-time clock 


### Jan 18, 2024:
- general_purpose_hydros is entirely librarified --> will need to be taken back as too much stuff in run() --> not flexible for different sensors
    - intended as starting point --> divide into different stages 
- added sample_ott_M and sample_ott_V to library --> compiled successfully into general_purpose_ott.ino
- general_purpose_ott compiled with library versions of everything except prep_msg, take_measurement, and parts of setup/loop that use take_measurement
    - prep_msg and take_measurement are sensor-dependent --> need some way to make it flexible before you can put into library (may not be worth it --> put helper functions in library and leave the sensor-specific part to each .ino file)
    

Notes: 
- analite, ultrasonic (possibly others) have their own pin assignments for different things --> start setting as constants? (defined outside instances) --> might be easiest, give more descriptive names
- general_purpose_hydros needs to be changed to allow re-generalization of prep_msg and take_measurement --> may need to remove from library
    - change to helper functions in library, sensor-specific things in .ino files 
    - will need to document how to use this


### Jan 15, 2024:
- added sample_batt_v to library --> compiled successfully in general_purpose_hydros.ino
- added sample_hydros_M to library --> compiled successfully in general_purpose_hydros.ino
- added take_measurement to library --> compiled successfully in general_purpose_hydros.ino
- added irid_test & send_msg to library --> compiled successfully in general_purpose_hydros.ino
    - note: IridiumSBD library does not have header guards --> be careful not to doubly define it (causes problems)
- added begin() and run() to be called in .ino setup() and loop() functions --> compiled successfully in general_purpose_hydros.ino


Notes:
- want to split begin() and run() into more smaller functions so it's easier to see what's happening
- clean up comments in general_purpose_hydros
- next step: integrate library into other files, accomodate more kinds of sensors
- longer term: make more flexible to select which sensors are attached/which measurements you want (in param file? or some other kind of user input?)


### Jan 14, 2024:
- added write_to_csv to library --> compiled successfully in general_purpose_hydros.ino (again, tough to test indepently --> may need more testing)
- added prep_msg to library --> compiled successfully in general_purpose_hydros.ino 
    - note: my_letter and my_header are doubly represented -- want them to eventually come in as user determined (depending on which sensors are attached/which measurements you want taken)


### Dec 20, 2023:
- added blinky to library --> tested independently, integrated (compiled) into general_purpose_hydros.ino
- added read_params to library --> compiled fine with general_purpose_hydros.ino, not sure how to test independently


Notes:
- all of the setup could probably be moved to the library --> need to see if it is the same between all the .ino files
- how do I test functionality of the whole system? need access to the readings? 
- eventually add more comments, code will also be more readable with the library


