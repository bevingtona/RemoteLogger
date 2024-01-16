# Version 0.5 - Adding Libraries

### Note: files in libraries/WeatherStation are the current files for the WeatherStation library applied across the .ino files. Do not change the name of the libraries or WeatherStation directories

### Note: Most comments, notes on code, and all edits to code are in general_purpose_hydros for the purpose of learning the code, testing compilation with libraries, etc.


Author: Rachel Pagdin


## Library add log: 

### Jan 15, 2024:
- added sample_batt_v to library --> compiled successfully in general_purpose_hydros.ino
- added sample_hydros_M to library --> compiled successfully in general_purpose_hydros.ino
- added take_measurement to library --> compiled successfully in general_purpose_hydros.ino


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


