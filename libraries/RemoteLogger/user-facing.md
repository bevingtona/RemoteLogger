# RemoteLogger - Low-Cost Remote Dataloggers

## Table of Contents
[**Introduction**](#introduction)<br>
&ensp;&ensp;[Features](#features)<br>
&ensp;&ensp;[Licensing](#licensing)<br>
[**Getting started**](#getting-started)<br>
&ensp;&ensp;[Setting up the Arduino IDE](#setting-up-the-arduino-ide)<br>
&ensp;&ensp;[Installing the RemoteLogger library](#installing-the-remotelogger-library)<br>
&ensp;&ensp;[Installing library dependencies](#installing-library-dependencies)<br>
&ensp;&ensp;[Upload code to your Feather M0](#upload-code-to-your-feather-m0)<br>
&ensp;&ensp;[Troubleshooting](#troubleshooting)<br>
[**Building a datalogger**](#building-a-datalogger)<br>
&ensp;&ensp;[Supported sensors](#supported-sensors)<br>
[**Library functions**](#library-functions)<br>
&ensp;&ensp;[Constructors and startup](#constructors-and-startup)<br>
&ensp;&ensp;[Basic functionality](#basic-functionality)<br>
&ensp;&ensp;[Sample tracking](#sample-tracking)<br>
&ensp;&ensp;[Telemetry](#telemetry)<br>
&ensp;&ensp;[Sampling](#sampling)<br>
&ensp;&ensp;[Pin assignment](#pin-assignment)<br>
[**Designing your own datalogger networks**](#designing-your-own-datalogger-networks)<br>
[**Acknowledgements and Credits**](#acknowledgements-and-credits)<br>


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
#### `String sample_ott(SDI12 bus, int sensor_address)`
Sample from the OTT PLS 500 sensor. Must be provided an SDI12 bus object from the Arduino SDI12 library. It also needs the sensor address; this is generally assumed to be 0. Returns six parameters - water level (mm), water temperature (degrees C), sensor status, sensor internal relative humidity, sensor dew, and sensor deg - separated by commas without spaces in a single string.
#### `String sample_analite_195(int analogDataPin, int wiperSetPin, int wiperUnsetPin)`

### Pin assignment


----

## Designing your own datalogger networks
### Writing sketches for combinations of supported sensors
For the list of supported sensors, click [here](#supported-sensors).
### Writing sketches with sensors not supported by the library 
For the list of supported sensors, click [here](#supported-sensors).

----

## Acknowledgements and Credits