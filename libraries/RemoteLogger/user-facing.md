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
[**Library functions**](#library-functions)<br>
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
#### I plugged in the Feather but no USB ports are showing up.
Make sure both ends of the USB connection are secure. If the Feather is in the FeatherWing terminal, make sure the switch is turned to ON or remove the Feather from the FeatherWing. If it still does not show up, you may be using a micro-USB cable that only has two wires (power + and -) rather than four (power and data lines). Try a different cord. Data micro-USB cables are usually thicker. 
#### The Feather is connected and the code compiles, but when I try to upload it does not show any output and eventually fails.
Occasionally the Feather needs to be put into "bootloader" mode to accept uploaded code. Hit the upload button and wait for the code to compile. Once it says "Uploading..." on the screen, double-press the Reset button on the Feather. The red LED next to the micro-USB port should start pulsing rapidly and the upload should start. 

## Building a datalogger
## Library functions
## Designing your own datalogger networks

## Acknowledgements and Credits