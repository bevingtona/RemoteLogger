# Version 0.5 - Adding Libraries

### Note: files in libraries/RemoteLogger are the current files for the RemoteLogger library applied across the .ino files. Do not change the name of the libraries or RemoteLogger directories


Author: Rachel Pagdin


## Library add log: 

### Jun 17, 2024:
- removed string parsing functionality from the library -- too many memory overflow issues 
- user needs to supply the RemoteLogger object with the following parameters:
    - the header for the CSV file
    - the number of sampled parameters (length of multipliers array and letters string)
    - the multipliers for each sampled parameter (not including battery voltage and free memory)
    - the letters for the start of each message to designate the parameters
- changed use of multipliers to allow selection of which sampled parameters to send 
    - put a zero corresponding to the parameters you don't want to send but will still sample
    - (e.g. OTT only sends first 2 of 6 sampled --> multiplier array: {1000, 10, 0, 0, 0, 0})
- sampling functions in library for OTT, Analite 195, ultrasonic, SHT31, DS18B20 
- examples:
    - blinky
    - changing utility pins
    - test Iridium 
    - prep message
    - test for DS18B20 (continuous measurement to serial)



### Jun 12, 2024:
- wrote a little instruction document for setting up the MCU (roughly follows the Adafruit setup tutorial with library installation specific to remote loggers) - see setup.md on rp_edits homepage
    - this needs edits to accomodate for Mac users, add photos



### Jun 5-6, 2024:
Functions of logger to add to library:
- [x] basic functions (blink, csv write, battery sample)
- [x] irid functions (send, test, update RTC)
- [x] prep message to send over iridium from CSV
    - needs: 
        - format of header string (e.g. "sffff") for CSV parser
        - the names of each column - Map object? (like a dictionary)
        - variable number of spaces to hold data points (up to 6 for OTT) - float array? --> ended up doing a loop, held in CSV parser until accessed each loop
- [ ] setup functions
    - [x] start data busses (SDI-12, I2C, etc) --> make this responsibility of user
    - [x] start SD card
    - [x] start RTC
    - [ ] read parameters from param file
    - [ ] run test mode function (test params, write to file, print a bunch of stuff to serial)
- [ ] sampling functions
    - [x] Hydros
    - [x] OTT (easy to do - same SDI-12 as HYDROS, only measurement command changed)
    - [x] Analite (analog)
    - [x] ultrasonic (digital - needs 3 pins, no bus)
- [ ] take measurement upper function - low priority
    - needs:
        - variable length list of sampling functions - callable objects
        - 
- [x] setters for pin assignments

#### Notes:
- end goal: want to be able to start a type of connection (SDI-12, I2C, etc) with connected pins and header info as input

sample_hydros_M:
- have sensor address and SDI12 object as input - could attach two Hydros units if you want, or another different SDI-12 compatible sensor 
- user is responsible for creating SDI12 object and beginning connection
- can do same thing for both OTT sample functions -- maybe could even make all into one sample_sdi12 function under the hood (supply measurement command as well as bus and sensor address)

prep_msg:
- changed to numbered indexing - faster, no type issues between String and char array types
- kept the header parsing to use sampled data headers to access letter headers and value multipliers (in dictionary)



### Jun 4, 2024:
- basic data logger functions into library: blinky, write_to_csv, sample_batt_v, tpl_done (new), send_msg + helper function sync_clock
- removed watchdog from Iridium send_msg -- look out for issues this may cause 
    - didn't fully understand how it worked -- seemed to be enabled and disabled around no code (so where would it get stuck?)
- all in library_v0.2 folder



### Mar 27, 2024:
- added clean code for hydros with library (library_clean folder)
- added hydros w/ TPL modifications and library

Note: 
- might it be good to somehow manage the time intervals within the library? seems like the hardest part



### Mar 25, 2024:
- removed set/unset pins from the library for hydros (as proof of concept)
- merged two sample_ott functions into one for better usability (since they're always used together anyway and it's not clear what the division is except running out of space on data bus)

Notes:
- need to take out take_measurement and make that the responsibility of the user for now
- need to write clean example for the basic sensors with the relay 
- examples for without the relay? -- ask Alex about the TPL chip (probably for after March)
- need to make a clean version of the library files to publish -- keep commented "dirty" version locally



### Jan 24-25, 2024:
- constants for pin numbers (specific to board --> needs to be documented)
- general_purpose_ott.ino compiled with more reasonable flexibility of library (as opposed to general_purpose_hydros, which is over-librarified)
- added sample_analite_95 to library --> compiled successfully in general_purpose_hydros_analite.ino
- added sample_ultrasonic to library --> compiled successfully in general_purpose_ultrasonic_sleep.ino
- got rid of overspecific begin() and run() functions --> fixed general_purpose_hydros.ino
- MILESTONE: library contains all functionality to convert all .ino files in v0.5_libraried folder (7 options), not all converted but easily could be (left to save time, since things need to be changed anyway re: watchdog, params, etc)

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


