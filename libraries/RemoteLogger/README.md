# RemoteLogger - Low-Cost Remote Dataloggers

This is legacy documentation as of August 30, 2024. See https://github.com/rpagdin/RemoteLogger for updated library documentation.

<img src='https://github.com/bevingtona/RemoteLogger/assets/9651129/34783a47-727d-43ba-a6db-63897659f26c' width='250'>

## Table of Contents
[**Introduction**](#introduction)<br>
&ensp;&ensp;[Features](#features)<br>
&ensp;&ensp;[*Licensing](#licensing)<br>
[**Getting started**](#getting-started)<br>
&ensp;&ensp;[Setting up the Arduino IDE](#setting-up-the-arduino-ide)<br>
&ensp;&ensp;[*Installing the RemoteLogger library](#installing-the-remotelogger-library)<br>
&ensp;&ensp;[Installing library dependencies](#installing-library-dependencies)<br>
&ensp;&ensp;[Upload code to your Feather M0](#upload-code-to-your-feather-m0)<br>
&ensp;&ensp;[*Troubleshooting](#troubleshooting)<br>
[**Building a datalogger**](#building-a-datalogger)<br>
&ensp;&ensp;[Supported sensors](#supported-sensors)<br>
&ensp;&ensp;[*Wiring diagrams and materials lists](#wiring-diagrams-and-materials-lists)<br>
&ensp;&ensp;[*Hardware tips and tricks](#hardware-tips-and-tricks)<br>
&ensp;&ensp;[*Build instructions](#build-instructions)<br>
&ensp;&ensp;[*Recommended bench tests](#recommended-bench-tests)<br>
&ensp;&ensp;[*Field deployment and testing](#field-deployment-and-testing)<br>
&ensp;&ensp;[Accessing data from MoF database](#accessing-data-from-mof-database)<br>
&ensp;&ensp;[*Maintenance and troubleshooting](#maintenance-and-troubleshooting)<br>
[**Library functions**](#library-functions)<br>
&ensp;&ensp;[Constructors and startup](#constructors-and-startup)<br>
&ensp;&ensp;[Basic functionality](#basic-functionality)<br>
&ensp;&ensp;[Sample tracking](#sample-tracking)<br>
&ensp;&ensp;[Telemetry](#telemetry)<br>
&ensp;&ensp;[Sampling](#sampling)<br>
&ensp;&ensp;[Pin assignment](#pin-assignment)<br>
[**Designing your own datalogger networks**](#designing-your-own-datalogger-networks)<br>
&ensp;&ensp;[Writing your own sketches for supported sensors](#writing-sketches-for-combinations-of-supported-sensors)<br>
&ensp;&ensp;[Writing your own sketches for unsupported sensors](#writing-sketches-with-sensors-not-supported-by-the-library)<br>
&ensp;&ensp;[*Setting up Iridium RockBlock system](#setting-up-iridium-rockblock-system)<br>
&ensp;&ensp;[Swapping hardware peripherals](#swapping-hardware-peripherals)<br>
&ensp;&ensp;[*Setting up a database](#setting-up-a-database)<br>
[**\*Acknowledgements and Credits**](#acknowledgements-and-credits)<br>

----

## Introduction
This project that allows you to measure and log hydrometric data in real-time using an Arduino Feather M0 microcontroller. This project is perfect for environmental monitoring applications, such as river water level measurements, flood monitoring, and water quality assessment.

With this hydrometric data logger, you can collect water level data and transmit it to your e-mail or a cloud service for real-time monitoring and analysis. The use of the Arduino Feather M0 ensures low power consumption and compatibility with various sensors, making it a versatile and reliable solution for your hydrometric data logging needs.

### Features 
-   Realtime measurement (Water level, water temperature, electrical conductivity, turbidity, ...)
-   Low power consumption and solar charging for extended operation
-   Customizable data logging intervals
-   Data transmission capabilities for remote monitoring with satellite telemetry
-   Easily expandable for additional sensors (e.g., temperature, pH)
-   Open-source and relatively low-cost

### Licensing 

[back to top](#table-of-contents)

----

## Getting started
### Setting up the Arduino IDE
The Arduino IDE (Integrated Development Environment) allows you to edit, compile, and upload code to your datalogger. Adafruit boards (MCUs) such as the Feather M0 are compatible with the Arduino IDE but need some additional configuration before code can be compiled and uploaded.

**First, install the Arduino IDE on your computer.** You can download the IDE from [Arduino's website](https://www.arduino.cc/en/software) or find it through a Google search. Try to download the most recent version, although if you have an older computer you may be required to use an older version. Most of the features are the same but if you are using a 1.x.x version things will look different than they do in this tutorial. Also, if you are using a Mac computer, some settings may be under the Arduino IDE tab rather than the File tab as they are in Windows.

**Next, install the board manager to support Adafruit SAMD boards.** The IDE needs additional board managers to know how to deal with the Feather M0. First, open File > Preferences (it may be under the main Arduino IDE tab for Mac) and paste this link into the box marked Additional Board Manager URLs: https://adafruit.github.io/arduino-board-index/package_adafruit_index.json. Click OK. This lets the IDE know where to download the board managers when you try to install them in the next step.<br>
To install the board managers, open the Boards Manager panel through Tools > Board > Board Managers. Search and install the latest versions of these managers:
1. Arduino SAMD Boards (32-bits ARM Cortex-M0+)
2. Adafruit SAMD Boards<br>

Restart (close and reopen) the Arduino IDE to apply the changes.<br>
To check whether the board managers installed correctly, under Tools > Board you should see an option for Adafruit SAMD Boards, under which you can select Adafruit Feather M0 (SAMD21). If this is not an option, uninstall and reinstall the board managers and restart the IDE.

### Installing the RemoteLogger library
### Installing library dependencies
The RemoteLogger library depends on several other Arduino libraries that must be installed before code written with the RemoteLogger library can be used. Most standard libraries can be downloaded through the Arduino IDE's built-in library manager, but some must be downloaded and installed as ZIP libraries following the same steps as for the RemoteLogger library in the previous section. When downloading libraries, it may ask if you want to download with dependencies. Select yes.<br>
Open the library manager and install the following libraries:
- Adafruit SHT31 Library (by Adafruit)
- CSV Parser (by Michal Borowski et al.)
- DallasTemperature (by Miles Burton et al.)
- Iridium SBD (by Mikal Hart)
- OneWire (by Jim Studt et al.)
- QuickStats (by David Dubins)
- RTClib (by Adafruit)
- SD (by Arduino, SparkFun)
- SDI-12 (by Kevin M. Smith or by Sara Damiano)

Install the following libraries from ZIP folders downloaded from the GitHub (for instructions on how to install libraries from a ZIP file see the [instructions on installing the RemoteLogger library](#installing-the-remotelogger-library)):
- Arduino MemoryFree from https://github.com/mpflaga/Arduino-MemoryFree

**Note:** It is important to install the exact libraries listed here, as different libraries of the same or similar names do exist but will not be compatible with the RemoteLogger library. 

### Upload code to your Feather M0
Now you should be ready to upload code to your Feather M0. Open the Blinky example from the RemoteLogger library through File > Examples > RemoteLogger > Blinky. This program (or sketch) blinks the built-in green LED on the Feather as long as power is provided. The example code will open in a new window.<br>

**Connect your Feather to the computer.** Plug your Adalogger into a USB port on your computer with a provided micro-USB cord. If the Adalogger is plugged into the FeatherWing (which has screw terminals along either side), make sure the power switch is turned to ON.<br>
Open the board selector dropdown in the top left corner of the IDE or go to Tools > Board. When the Adalogger is connected, its board and port should appear automatically. If they do not, select Adafruit Feather M0 (SAMD21) from Tools > Board > Adafruit SAMD Boards and a port labelled "COM # (USB)" under Tools > Port. If no such port appears, make sure the power switch is on and that your USB cord connection is secure on both ends. It is safe to unplug and plug back in.

**Compile the code** This is an optional step, but a good one to confirm that you have all the necessary libraries and that there are no errors in your code before attempting to upload. Ensure that you have selected the Adafruit Feather M0 board (the port doesn't matter for compiling as it will not actually communicate with the MCU).<br>
Compile the code with the checkmark button in the top left or at Sketch > Verify/Compile. The output panel at the bottom should show progress of the compilation. There may be some red text; if there are errors the compilation will fail. However, if the last line of output shows "Sketch uses XXX bytes (XX%) of program storage space", compilation was successful and there are no mistakes in your code. If you get errors saying that you are missing libraries, go back to the [previous section](#installing-library-dependencies) to install the missing libraries. 

**Next, you can upload the code.** To upload code to the MCU, press the right-facing arrow button in the top left corner of the IDE or use Sketch > Upload. The code will first compile, then attempt to upload to the selected board over the selected port. You should get quite a bit of output in the output panel, including progress bars of the upload. The red LED next to the micro-USB port on the Adalogger should turn on and pulse quickly while the code is uploading.<br>
If the upload fails, check that you have selected the correct board and are connected to the USB COM port. This information should be visible in the bottom right corner of the IDE. You can also put the Adalogger in "bootloader" mode by quickly double-pressing the Reset button (the only button on top of the Adalogger). Turn to bootloader mode only once compilation is finished and the upload has started.<br>
Once the code is uploaded, it will start executing immediately and continue to loop until power is interrupted (either by turning off the power or pressing the Reset button). Since you uploaded the Blinky code, the green LED on top of the Feather (near the SD card slot) should be blinking steadily as long as the Feather is plugged into your computer or a charged battery.

### Troubleshooting
#### **I plugged in the Feather but no USB ports are showing up.**
Make sure both ends of the USB connection are secure. If the Feather is in the FeatherWing terminal, make sure the switch is turned to ON or remove the Feather from the FeatherWing. If it still does not show up, you may be using a micro-USB cable that only has two wires (power + and -) rather than four (power and data lines). Try a different cord. Data micro-USB cables are usually thicker. 
#### **The Feather is connected and the code compiles, but when I try to upload it does not show any output and eventually fails.**
Occasionally the Feather needs to be put into "bootloader" mode to accept uploaded code. Hit the upload button and wait for the code to compile. Once it says "Uploading..." on the screen, double-press the Reset button on the Feather. The red LED next to the micro-USB port should start pulsing rapidly and the upload should start. 

[back to top](#table-of-contents)

----

## Building a datalogger
### Supported sensors

| Sensor | Parameters | Approx. Cost | Library Sampling Function | 
| --- | --- | --- | --- | 
| Hydros 21 | water level; water temp; electrical conductivity | $600 | sample_hydros_M |
| OTT PLS 500 | water level; water temp | $1,200 | sample_ott |
| Analite 195 | turbidity | | sample_analite_195 |
| MaxBotix MB7369 | water/snow level (distance) | $153 | sample_ultrasonic |
| Waterproof DS18B20 | water/air temp | $10 | sample_DS18B20 |
| Adafruit SHT31 | air temp; relative humidity | $19 | sample_sht31 |

Examples are provided for the first four of these sensors along with the library. Once the library is installed, example code is available through File > Examples > RemoteLogger. The loggers can also be easily built to incorporate two or more sensors. For instructions on designing your own sensor combinations or extending to unsupported sensors, see [this section](#designing-your-own-datalogger-networks).

### Wiring diagrams and materials lists
### Hardware tips and tricks
#### TPL Nano Timer
The TPL timer is a vital part of the data logger system but it can be finicky and easily damaged by incorrect or careless wiring that may not harm other components. It can also be hard to tell whether or not the TPL chip has been damaged. To avoid frying the component,
1. Do not plug USB power into the solar charge controller while the TPL is connected to the Feather MCU. 
2. Do not plug USB power into the Feather MCU (via the micro-USB slot beside the RESET button) while the TPL chip is wired to the Feather using the BATT and GND pins. While it is possible to use the TPL in this way, plugging in the USB power can cause current to run the wrong way through the TPL chip, frying the component. 

The TPL chip is susceptible to this kind of damage because it does not have built-in protections against current flowing the wrong way. This can be fixed using a Schotky diode (somewhat like a one-way valve for current) but it is not yet built in to the basic remote logger design.<br>
Be careful when wiring your TPL chip and using a Feather that has a TPL chip attached. Once fried, the TPL chips can not be used. 

### Build instructions
### Recommended bench tests
### Field deployment and testing
### Accessing data from MoF database
If you have arranged with Alex Bevington to have your RockBlock units send data to his database hosted and managed by the BC Ministry of Forests, or if you are just interested in seeing what some of the data looks like from the units that are active, data from the remote logger network can be accessed via an [online app](https://bcgov-env.shinyapps.io/nbchydro/). RockBlock IMEIs (IDs) must be known by the database to accept data, and messages must follow a set format so that the data can be extracted to store in the database.<br>
*INSERT IMAGE*<br>
<img src=docs/data-website-homepage.png width='500'><br>
<img src=docs/data-website-homepage-2.png width='500'><br>
A map is provided on the website to show the locations of active remote logger units and the time since their last message was received. <br>
Not every active remote logger unit is outfitted with the same sensors or is even collecting the same type of data. Data can be selected by collected parameters or name so it is possible to download the data in CSV format only for relevant sites and dates. Sites can also be selected to view graphs including the most recent data.<br>
For more information about the data and database, contact Alex Bevington at alexandre.bevington@gov.bc.ca. 

### Maintenance and troubleshooting

[back to top](#table-of-contents)

----

## Library functions
### Constructors and startup
#### `RemoteLogger()`
Default no-arg constructor. Deprecated, only for testing purposes. Sets header for Hydros21 setup.
#### `RemoteLogger(String header)`
Deprecated. Do not use. Sets header to provided string but does not provide adequate information for message preparation.
#### `RemoteLogger(String header, byte num_params, float *multipliers, String letters)`
Initiates a RemoteLogger object with the provided header and parameters for message preparation. Headers are internal and will not affect how messages are prepared, sent, or received by the external endpoint; however, for proper message preparation the first three parameters should be: timestamp, battery voltage, free memory. Headers should contain no spaces.<br>
The number of parameters must correspond to the total number of sampled parameters from sensors (i.e. excluding the battery voltage and free memory). The multipliers argument, of length num_params, provides the multipliers to remove decimal places from parameters to send, or to truncate zeroes (e.g. if temperature is measured to two decimal places, multiply by 100 to remove decimal places. Likewise, to truncate a 5-digit measurement to 3 digits, multiply by 0.01). These multipliers should be reflected on the external endpoint for messages so that data can be extracted. For parameters that are measured and saved to the data file but not intended to be sent, provide a multiplier of 0.<br>
Letters start the message to alert the endpoint to the order and identity of the measurements. They correspond to the parameters that will be sent. 
```c++
String header = "datetime,batt_v,memory,water_level_mm,water_temp_c,water_ec_dcm";      
const byte num_params = 3;          // measured parameters: water level, water temp, electrical conductivity
float multipliers[num_params] = {1, 10, 1};         // level x 1, temp x 10, EC x 1
String letters = "ABC";             // A: level, B: temp, C: EC

RemoteLogger logger(header, num_params, multipliers, letters);          // initialize remote logger
```
#### `void begin()`<br>
Sets up pins and starts peripherals of logger (clock, SD card). If any of the following pins are not in wired to their defaults, change pin values using [pin change functions](#pin-assignment) before calling this function.
| Peripheral | Default Pin | Assignment Function | Notes |
| --- | --- | --- | --- |
| Built-in LED | 8 | `setLedPin` | Default is built-in LED pin for Adalogger |
| Battery voltage | 9 | `setBattPin` | Default is battery pin for Adalogger |
| TPL done | A0 | `setTplPin` | Must be an analog output pin (A0 is only analog output on Adalogger) |
| Iridium modem sleep pin | 13 | `setIridSlpPin` | Match to wiring |
| SD card select pin | 4 | `setSDSelectPin` | Default is SD select on Adalogger |
```c++
void setup(){
    // change any pins here

    logger.begin();
}
```


### Basic functionality
#### `void blinky(int n, int high_ms, int low_ms, int btw_ms)`
Blink the onboard LED n times. Useful for visual feedback of progress while the datalogger is not connected to a laptop for Serial output.<br>
&ensp;**high_ms:** ms for the LED to be on each blink<br>
&ensp;**low_ms:** ms for the LED to be off between each blink<br>
&ensp;**btw_ms:** ms between series of n blinks (set to 0 if you just want one set of blinks without delay afterwards)<br>
If using an MCU other than the Feather M0 Adalogger, check the pin for the onboard LED and change from the default if necessary.
#### `void write_to_csv(String header, String datastring_for_csv, String outname)`
Write the provided datastring to the CSV file outname. The header will be written only if the file has to be created before writing (i.e. this is the first datastring for the CSV file). If the datastring is empty, an empty line will be added to the file.
#### `float sample_batt_v()`
Returns the battery voltage from the battery pin. 
#### `int sample_memory()`
Returns the amount of available volatile memory (RAM) on the MCU.
#### `void tpl_done()`
Notifies the TPL chip that execution is finished and the TPL should turn off the power to the MCU. 
#### `void wipe_files()`
Removes data and tracking files from the SD card. <br>
**Ensure any data is saved before making use of this function.**

### Sample tracking
Because the power to the MCU is interrupted completely by the TPL chip between measurements, counters are stored in hard memory on the SD card and managed through the following functions.
#### `void increment_samples()`
Increments the count of how many samples have been taken since the last write to the hourly data file. The hourly data file holds data to be sent via telemetry and is removed with each successful send, while the main data file holds every sample that is collected. This counter should be incremented every time a sample is taken. The counter can be reset with the `reset_sample_counter` function.<br>
The sample count is stored in hard memory on the SD card. Do not tamper with the file TRACKING.csv.
#### `int num_samples()`
Getter for the sample counter, which stores the number of samples collected since the last write to the hourly data file.<br>
The sample count is stored in hard memory on the SD card. Do not tamper with the file TRACKING.csv.
#### `int num_hours()`
Getter for the hour counter, which stores the number of samples recorded in the hourly data file. This corresponds to the number of hours since a message was successfully sent, or since a data wipe occurred.<br>
The hour count is stored in hard memory on the SD card, generated from the HOURLY.csv file.
#### `void reset_sample_counter()`
Reset the sample counter. Call this function after writing to the hourly data file.
#### `void reset_hourly()`
Reset the hour counter. This will remove the file HOURLY.csv from the SD card - be cautious of data loss, though the same data will be included in DATA.csv.

### Telemetry
The RemoteLogger library supports satellite transmission using the RockBlock 9603 modem, through the Iridium satellite network. For more information on the modem, click [here](https://www.groundcontrol.com/product/rockblock-9603-compact-plug-and-play-satellite-transmitter/?srsltid=AfmBOooiVTBYLxPfS9IsmQUdQ4sU2M8UNVesH6zPvnmTGXfwfqXJq8Gu).
#### `int send_msg(String myMsg)`
Send the provided message over the satellite network. Returns error codes corresponding to the error codes used in the Arduino IridiumSBD library. Some frequently seen codes are below:
| Code | Integer Value |
| --- | --- |
| ISBD_SUCCESS | 0 |
| ISBD_ALREADY_AWAKE | 1 |
| ISBD_NO_MODEM_DETECTED | 5 |
| ISBD_SENDRECEIVE_TIMEOUT | 7 |
| ISBD_IS_ASLEEP | 10 |

Messages provided to this function must be strings and less than 340 characters long.<br>
This function syncs the datalogger's clock to the Iridium clock once roughly every five days to account for drift.<br>
Sending messages uses credits; make sure you have credits on your account before attempting to send messages or they will fail.
#### `void irid_test(String msg)`
Test the RockBlock modem. If the MCU is connected to a laptop with the Serial monitor open, the firmware version and signal quality will be displayed. This function will attempt to send the provided message, with "Hello world " appended to the front. Messages provided to this function must be less than 329 characters in length or the message will be too long to send.<br> 
It will also sync the datalogger clock to the Iridium clock. This uses credits.
#### `String prep_msg()`
Prepares a message to send over the satellite network. The message is generated from the hourly data file according to the following format:<br>
>\<letters\>:\<datetime\>:\<battery\>:\<memory\>:\<sample\>:\<sample\>:\<sample\>:...<br>

Samples each consist of the parameters selected to be sent, multiplied by their respective multipliers. Parameters from a single sample are separated by commas. The entire message string is terminated by a colon.<br>
Example:<br>
> **Hourly file:** <br>datetime,batt_v,mem,water_temp_c,rh_pct<br>2024-07-13T12:05:34,4.32,23400,15.4,23<br>2024-07-13T13:06:23,4.31,23400,15.2,27<br>2024-07-13T14:05:49,4.37,23410,16.1,45
>
> **Message:** BG:24071312:437:234:154,23:152,27:161,45:

In the example above, water temperature and relative humidity are the sampled parameters. The letters B and G represent these parameters respectively. This should be reflected in the external endpoint for the messages. For more information on the letter-parameter relationships accepted by the established MoF database, contact Alex Bevington for detailed source code documentation.<br>
The transmitted message contains only one date and time, battery measurement, and memory measurement. The date and time correspond to the time of the *earliest* measurement in the transmission, while the battery and memory correspond to the *most recent* measurement in the transmission (i.e. the last one). Each sample is assumed to be timestamped an hour after the preceding sample.

### Sampling
It is recommended to declare a `take_measurement` function to collect samples of battery voltage, free memory, and whatever sampled parameters in one place. This is the structure written in the example code provided with the library. Any sampling can be done here, and the timestamp can be added to the beginning of the string before writing to the data file. Click [here](#writing-sketches-for-combinations-of-supported-sensors) for more information on combining multiple sensors.
```c++
String take_measurement(){
    String msmt = String(logger.sample_batt_v()) + "," +
        logger.sample_memory() + "," +
        // insert additional sampling here
    return msmt;
}
```
#### `String sample_hydros_M(SDI12 bus, int sensor_address)`
Sample from the Hydros 21 sensor. Must be provided an SDI12 bus object from the Arduino SDI12 library. It also needs the sensor address; this is generally assumed to be 0. Returns three parameters - water level (mm), water temperature (degrees C), and electrical conductivity - separated by commas without spaces in a single string.
```c++
SDI12 sdi12(12);        // create SDI-12 object with data pin 12 (can be any digital pin)
sdi12.begin();
...
String sample = logger.sample_hydros_M(sdi12, 0);       // pass SDI-12 object and sensor address to sample
```
#### `String sample_ott(SDI12 bus, int sensor_address)`
Sample from the OTT PLS 500 sensor. Must be provided an SDI12 bus object from the Arduino SDI12 library. It also needs the sensor address; this is generally assumed to be 0. Returns six parameters - water level (mm), water temperature (degrees C), sensor status, sensor internal relative humidity, sensor dew, and sensor deg - separated by commas without spaces in a single string.
```c++
SDI12 sdi12(12);        // create SDI-12 object with data pin 12 (can be any digital pin)
sdi12.begin();
...
String sample = logger.sample_ott(sdi12, 0);        // pass SDI-12 object and sensor address to sample
```
#### `String sample_analite_195(int analogDataPin, int wiperSetPin, int wiperUnsetPin)`
Sample from Analite 195 turbidity sensor. Provide 3 pins: 
- `analogDataPin`: data pin for ranger, must be analog input
- `wiperSetPin`: ON pin for built-in wiper, any digital pin
- `wiperUnsetPin`: OFF pin for built-in wiper, any digital pin

This function manages pin assignment to output - no need to designate before passing to the function.
#### `String sample_ultrasonic(int powerPin, int triggerPin, int pulseInputPin)`
Sample from MaxBotix MB7369 ultrasonic ranger. Provide 3 pins:
- `powerPin`: ON/OFF pin for ranger, any digital pin
- `triggerPin`: trigger pin to start measurement, any digital pin
- `pulseInputPin`: input pin to read pulse from ranger; must be PWM compatible (all pins except A1, A5 on Feather M0 Adalogger)

This function manages pin assignment to output - no need to designate before passing to the function.
#### `String sample_sht31(Adafruit_SHT31 sensor, int sensorAddress)`
Sample from Adafruit SHT31 temperature/relative humidity sensor. Provide an Adafruit_SHT31 sensor object. Set sensorAddress as 0x44 for default.<br>
This function will set up the sensor - no need to begin the SHT31 object.
```c++
Adafruit_SHT31 sht31 = Adafruit_SHT31();   // create the SHT31 object
...
String sample = logger.sample_sht31(sht31, 0x44);       // pass SHT31 and address to sampling function
```
#### `String sample_DS18B20(DallasTemperature sensors, int sensorIndex)`
Sample from Adafruit DS18B20 waterproof temperature sensor. Provide DallasTemperature object to contain sensor and sensor index, generally assumed to be 0. Multiple sensors can be daisy-chained to the same DallasTemperature object and accessed by index -- for more information, see DallasTemperature library documentation. <br>
Create a DallasTemperature object with a OneWire object created with the digital pin attached to data wire on DS18B20. 
```c++
OneWire oneWire(12);        // OneWire object created on digital pin 12
DallasTemperature sensors(&oneWire);
sensors.begin();        // start up the DallasTemperature object
...
String sample = logger.sample_DS18B20(sensors, 0);      // pass DallasTemperature object and index to sample
```

### Pin assignment
Pins are set to defaults for Adafruit Feather M0 Adalogger. If any pins need to be changed from the defaults, change them before calling `logger.begin()`.
| Peripheral | Default Pin | Assignment Function | Notes |
| --- | --- | --- | --- |
| Built-in LED | 8 | `setLedPin` | Default is built-in LED pin for Adalogger |
| Battery voltage | 9 | `setBattPin` | Default is battery pin for Adalogger |
| TPL done | A0 | `setTplPin` | Must be an analog output pin (A0 is only analog output on Adalogger) |
| Iridium modem sleep pin | 13 | `setIridSlpPin` | Match to wiring |
| SD card select pin | 4 | `setSDSelectPin` | Default is SD select on Adalogger | 
```c++
void setup(){
    // change any pins here

    logger.begin();
}
```
#### `void setLedPin(byte pin)`
Change LED pin. Provide digital pin attached to LED.
#### `void setBattPin(byte pin)`
Change pin to read battery voltage. Likely only change this if using an MCU with a different battery read pin than the Adalogger.
#### `void setTplPin(byte pin)`
Change pin to alert TPL timer chip to shut off power. Needs an analog pin capable of output (only A0 on Adalogger).
#### `void setIridSlpPin(byte pin)`
Change pin for Iridium modem sleep pin (pin 7 from RockBlock 9603 modem). Provide digital pin attached to sleep pin on RockBlock modem. 
#### `void setSDSelectPin(byte pin)`
Change pin to select SD card reader. Change this if using an MCU with a different SD chip select pin than Adalogger or using an external SD slot breakout. If using an external SD breakout, provide digital pin attached to CS pin.

[back to top](#table-of-contents)

----

## Designing your own datalogger networks
### Writing sketches for combinations of supported sensors
Example code is provided for systems using one of a subset of the sensors supported by the library. However, applications of remote data loggers are rarely limited to a single sensor, and the library framework can support combining two or more of the sensors supported with functions in the library. For the list of supported sensors, click [here](#supported-sensors). If you want to include sensors that do not have functions in the library, see [the next section](#writing-sketches-with-sensors-not-supported-by-the-library).<br>
Writing your own code has the potential of introducing bugs into the system. Sensor combinations can also introduce new challenges of memory and power usage. Bench testing is highly recommended, as well as some knowledge of writing and testing code for Arduino systems.<br><br>
Three main changes must be made to provided code to accomodate multiple supported sensors:
1. The header, letters, multipliers, and number of parameters variables defined at the start of the program must be changed to reflect the parameters that will be sampled and, of those sampled, the parameters that will be sent. 
2. The sampling functions used must be provided with the information and objects required to function (communication objects, pin numbers, addresses, etc). 
3. To maximize simplicity, the `take_measurement` function should be changed to contain the sampling of all parameters as well as the battery voltage and free memory. This allows the `loop` function to be copied directly from example code using a single sensor without modification. 
#### Setting up the header, letters, multipliers, and number of parameters
The header you provide to the RemoteLogger object is used to define the CSV files that hold the data. This header must reflect **all** parameters that are sampled, not just the parameters that will be sent over the satellite network. It must follow CSV format: header strings separated by commas and without spaces surrounding the commas. Ideally, the header strings themselves should not contain spaces; common practice is to replace spaces with underscores. The first three header items should be date/time ("datetime" in example code), battery voltage ("batt_v" in example code), and free memory ("memory" in example code). After this, sampled parameters must be listed in the order they are sampled in the `take_measurement` function. The order in which samples are returned from sensor sampling functions can be found in the [sampling function documentation](#sampling). The header should not end with a comma as this indicates an empty column.
> **Example header:** `datetime,batt_v,memory,snow_depth_mm,air_2m_temp_deg_c,air_2m_temp_rh_prct`<br>
This header is for a system that samples first from the MaxBotix ultrasonic ranger for `snow_depth_mm` and then from the Adafruit SHT31 for `air_2m_temp_deg_c` and `air_2m_temp_rh_prct`. 

The header that you provide will only be visible internally; it will not be sent by message over the satellite network. It will be stored as the header on the data file that keeps all samples on the SD card to be retrieved later. It is, however, very important to make sure that it is constructed correctly as it will introduce issues with reading the CSV files if it has the wrong number of parameters.<br><br>
The following two pieces--the letters and multipliers--correspond to only the parameters that will be **sent** in a message. Consider the example above: you want to collect snow depth, air temperature, and relative humidity. However, you are not interested in real-time data for humidity and want to shorten your messages. So though you are collecting all three parameters, you only want to send snow depth and air temperature. <br>
You can customize this using the letters and multipliers. The most critical thing to remember when modifying these values is that **both need to match**.<br><br>
The letters you provide will lead each message and they show what sampled parameters will be included as well as their order. The term "sampled parameters" excludes the date and time, battery voltage, and free memory as these will always be included in a message. In the case of our example, we have three sampled parameters and they are snow depth in mm, air temperature in degrees C, and relative humidity in percent saturation. Of these sampled parameters, we want to send two of them: snow depth and air temperature. For simplicity, we will send them in the message in the same order that they are sampled: snow depth and then air temperature.<br>
If you are not using the MoF database, you can define your own dictionary of letters and parameters to use and decode them on the other side. The MoF database is set up to decode according to the following dictionary: 
 Measurement | Units | Code | Header Name | Sensors
 ----------- | ----- | ---- | ----------- | -------
 Water Level | mm | A | water_level_mm | Ultrasonic, Hydros21, OTT
 Water Temp | ⁰C | B | water_temp_c | OTT, Hydros21, DS18B20
 Electrical Conductivity | dS/m | C | water_ec_dcm | Hydros21
 Turbidity | NTU | D | ntu | Analite195
 Snow Depth | mm | E | snow_depth_mm | Ultrasonic
 Air Temp @ 2m | ⁰C | F | air_2m_temp_deg_c | SHT31
 Relative Humidity @ 2m | % | G | air_2m_temp_rh_prct | SHT31  

The `letters` value must be provided to the RemoteLogger object as a string of uppercase letters with no spaces. In our example, we are sending snow depth and then air temperature, so we need the letters E and F.<br><br>
The `multipliers` list serves two purposes: first, as a selector for which sampled parameters to send in messages; and second, as an option to remove decimal points or extra zeroes from messages to save space. The list maps directly onto the order of the sampled parameters in the provided header.<br>
A multiplier value of 0 means that you do not want the corresponding parameter to be sent in the message. Multiplier values of 1 do not modify the sampled values before they are written into the message; they retain decimals or extra zeroes. Sometimes it is a case of trial and error to determine how many zeroes or decimal places you can remove; if you want to change the multipliers from those provided in example code for the sensors, consult the documentation that is provided by the manufacturer of the sensor.<br>
In the case of our example, we have three sampled parameters so our multiplier list will have three positions. We don't want to send our last parameter--the relative humidity--so we will put a zero in the third position. Snow depth is measured in whole numbers so we will just leave it be and put a 1 in the first position. Air temperature is sampled with a single decimal place on the SHT31, so we will put a 10 in the second position to remove the decimal place from the message.<br>
> **Example multiplier list:** `{1, 10, 0}`

It is important to note that any changes to the sampled values by multipliers must be reflected on the receiving side for the messages so that the original values can be recovered before being stored.<br>
This may seem complex, but it allows the library to automatically create messages that adhere to the same format. If you do not want to customize at all--you are happy to send all the sampled parameters in their original forms--put a 1 in every position of the multipliers list.<br><br>
The number of parameters--defined as `num_params`--is an integer indicating the number of **sampled parameters** (remember from the definition above that "sampled parameters" excludes the date and time, battery voltage, and free memory). This value can be determined by counting the number of sampled parameters, or subtracting 3 from the total number of parameters (columns) in the header. It is also the length of the list of multipliers.<br><br>
Once these four things are defined, they can be used to create a RemoteLogger object. For our example, it would look like this: 
```c++
String header = "datetime,batt_v,memory,snow_depth_mm,air_2m_temp_deg_c,air_2m_temp_rh_prct";
const byte num_params = 3;
float multipliers[num_params] = {1, 10, 0};
String letters = "EF";

RemoteLogger logger(header, num_params, multipliers, letters);
```
#### Providing the sampling functions with what they need
Each sampling function requires different objects to be passed to it so that it can work. What these are depends on the individual requirements of each sensor. In simpler cases, they are just the pin numbers corresponding to the wiring of the sensor to the MCU; in other cases, sensors can require an object representing a communication port (for example, SDI-12 or I2C, both communication protocols that use other libraries to convey data).<br>
The requirements for each sampling function can be found in the [sampling function documentation](#sampling). Pin numbers can be defined as variables or passed directly as integers (though this is not recommended for readability and editability of your code). Communication objects each have their own requirements; some only need to be created, while others need to be "started" as well. This information is also available in the documentation of each sampling function. Make sure you read and understand what is required for each before using the function.<br>
It is useful to define pin numbers and communication objects at the head of your code outside of any of your defined functions. They will then be accessible from within any function that you define. Communication objects that need to be "started" should have these functions (often named `begin`) called inside the required `setup` function in your Arduino program.<br>
Some sampling functions can require addresses for the sensors as well. If you are using multiple of the same sensor on one logger, this is used to differentiate between the two sensors. If this is the case, consult the documentation from the manufacturer on how to change the address of one of the sensors from the factory defaults. Otherwise, the defaults for each sensor can be found in the [sampling function documentation](#sampling).
#### Defining your own `take_measurement` function
It is highly recommended to define your own `take_measurement` function, usually at the top of your code right under variable definitions (see examples), that follows the same structure and return type as those provided in the examples. This should collect samples for the battery voltage, free memory, and all of your sampled parameters and stick them together into a comma-separated string of values (with no spaces). If this structure is followed, you can use the same `loop` function from the example code for any single sensor.<br>
For our example above (sampling snow depth from the MaxBotix ultrasonic ranger and air temperature and relative humidity from the SHT31), we would define a `take_measurement` function as follows:
```c++
String take_measurement(){
    String msmt = String(logger.sample_batt_v()) + "," +
        String(logger.sample_memory()) + "," +
        logger.sample_ultrasonic(ultrasonicPowerPin, triggerPin, pulseInputPin) + "," + 
        logger.sample_sht31(sht31, tempRHAddress);

    return msmt;
}
```
This example is provided in full in the examples folder of the RemoteLogger library in the folder MultipleSensorsPartialSend.<br>
Required objects/pin numbers/addresses for each sampling function would be defined as variables above this function (see [above](#providing-the-sampling-functions-with-what-they-need) for more information).
#### Changing the sending interval
Example code defaults to sending messages every four hours, and every hour up to 10 hours if the sends continue to be unsuccessful. These values are hard-coded into the logic in the `loop` function and can be changed with careful consideration. **Exercise extreme caution when changing any integer values.** Some integer values in the `loop` function are related to the measurement of an hour as four cycles of the 15-minute TPL timer. 

### Writing sketches with sensors not supported by the library 
For the list of supported sensors, click [here](#supported-sensors).<br><br>
There is a vast array of sensors available that are compatible with the Feather Adalogger and with a little knowledge of coding in Arduino, it is possible to use the RemoteLogger library to handle messages and telemetry for sensors that are not supported with their own functions within the library. Most sensors use one of a small set of communication protocols, many of which have their own Arduino libraries.<br>
It is also possible to swap out peripheral components for others while still using functions from the RemoteLogger library by "overriding" the functions that interact with those components. For more information on this, click [here](#overriding-library-functions).<br>
Here are some tips for writing sketches with sensors that do not have their own functions within the library:<br><br>
**1. Get comfortable with the sensor.**<br>
Before trying to integrate your new sensor into the remote logger system, make sure that you are comfortable with how to wire the sensor to the Feather Adalogger and how to sample from it. Any sensor should be tested independently before being integrated into a system of any kind. Make sure you know what setup the sensor needs, what function to use to sample from it, how it needs to be wired, and what the return type is from the sampling function. Does it have multiple decimal places? Do you need to call a function to request that the sensor takes a measurement before you can read from it? Are there any functions that need to be called periodically for maintenance? Does it have any extra hardware requirements? It can also be useful to check out its power consumption during this step.<br><br>
**2. Make sure you include the necessary libraries to use the sensor.**<br>
The sensor may have its own library (most Adafruit sensors have a specific Arduino library, for example) or libraries for communication protocols that you need to include in your sketch. Some libraries are already included with the RemoteLogger library, but if you're in doubt, it will not hurt anything to include it again. Include statements are usually placed at the top of the sketch, right under any comments.<br><br>
**3. Write your own sampling function.**<br>
Following a similar structure to the [sampling functions](#sampling) included in the library, write your own sampling function for the sensor. The better you match the format, the easier it will fit into the code that is provided as examples with the RemoteLogger library.<br>
The most important thing to match is the return type from the function. Like the other sampling functions, your function should return a String object of the sampled parameters separated by commas and no spaces. Then you can fit your sampling function right into your custom `take_measurement` function along with the others! Optionally, you can also set up the required objects, pin numbers, and addresses ahead of time and pass them into your sampling function just like the RemoteLogger sampling funtions. However, if you want to include all that stuff in your sampling function it won't really hurt anything, since it's all custom to you anyway. However, if you wanted to reuse your sampling function in a different setup, it may be desirable to make it transportable and allow the option to pass in parameters like the sensor address or the data pin.<br>
Include your sampling function in your custom `take_measurement` function, the same way that you would for supported sensors. You can easily use a supported sensor sampling function alongside your own function; as usual, though, be advised that any mixing will change power and memory consumption.<br><br> 
**4. Match your header, letters, and multipliers to your sensor setup.**<br>
Now that you have a `take_measurement` function defined, follow the [same steps](#writing-sketches-for-combinations-of-supported-sensors) you would for supported sensors to develop your header for your data files and your letters and multipliers for your messages. Match these to the string of comma-separated sampled values that will come out of your `take_measurement` function. Once you have these things defined, you can build your RemoteLogger object as usual, which will handle message prep and data management for you.<br><br>
**5. Make sure you can receive your messages on the receiving side.**<br>
The last thing is to make sure that the system that receives your messages is set up to catch and decode your messages, which may contain different parameters. You may not, however, actually have any different parameters, if you are already set up to handle air temperature and you've just swapped out one temperature sensor for another. Just don't forget about the receiving end once you've finished up your sending end!

### Setting up Iridium RockBlock system
### Swapping hardware peripherals
The RemoteLogger library uses the RockBlock 9603 modem connected to the Iridium satellite network. However, there are other telemetry options available (other satellite networks, WiFi, cellular, etc.) that have modems that are compatible with the Feather Adalogger. One of these other options may be more suitable than the Iridium network depending on location, accessibility, cost, etc. As long as the chosen modem can send text somehow, it can be used with the data logging and message preparation functionality of the RemoteLogger library.<br>
To use a different telemetry option, you will have to replace (or "override") the `send_msg` function from the RemoteLogger library. This function should send the provided message and return an integer success code (which can be defined according to whatever system you like). This function should also query the time from the network periodically to update the time on the RTC (real-time clock) on the unit. The RTC is otherwise managed by the RemoteLogger library but can be accessed through the RemoteLogger object to update the time manually. Without updates, the time will drift over time due to hardware imprecision. Every week or so is plenty if it costs "credits" to check the time; the drift over this time will not be significant. To sync the time, query the date and time from your chosen telemetry network, then access the RTC through the RemoteLogger object to update it:
```c++
logger.rtc.adjust(DateTime(2024, 08, 30, 10, 25, 0));       // update RTC time to Aug 30, 2024 10:25:00
```
You will have to read the documentation of the modem to understand how to access the integer values you want to pass to the `adjust` function.<br><br>
While technically it is possible to override any function in the RemoteLogger library, the RemoteLogger library cannot perform its basic functionality with a change in the RTC hardware. If the RTC hardware is changed to an RTC chip not compatible with the Adafruit RTClib library, the source code for the library will have to be modified to accomodate the change.<br><br>
It is theoretically possible to swap out the onboard SD card slot on the Adalogger for a separate SD breakout wired to the SPI pins on the Adalogger. Modify the pin assignment for the SD card chip select pin using the `setSDSelectPin` function (see the [pin assignment section](#pin-assignment)) before calling `logger.begin()` in the `setup` function.
### Setting up a database

[back to top](#table-of-contents)

----

## Acknowledgements and Credits

[back to top](#table-of-contents)