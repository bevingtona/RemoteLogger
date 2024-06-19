# RemoteLogger Arduino Library

## Using the RemoteLogger library

### Settings to prepare messages




## Setting up the Arduino IDE for the first time

### 1. Install the Arduino IDE on your computer 
Install the Arduino IDE from https://www.arduino.cc/en/software for the latest version. This tutorial follows Version 2 of the IDE; some things may be different for Version 1.  


### 2. Set up to support Adafruit SAMD boards 
To support the Feather M0 Adalogger, you have to install additional board managers for the IDE. 

#### Get the board manager package
In the Arduino IDE, go to File > Preferences (or possibly Arduino IDE > Preferences on Mac).<br>
Paste https://adafruit.github.io/arduino-board-index/package_adafruit_index.json into the box for Addition Board Manager URLS. <br>
Click OK.

#### Install board managers
In the Arduino IDE, go to Tools > Board > Boards Manager<br>
Search and install the following: 
- Arduino SAMD Boards (32-bits ARM Cortex-M0+)
- Adafruit SAMD Boards<br>
Restart the Arduino IDE.<br>
To check whether the board managers installed correctly, under Tools > Board you should see an option for Adafruit SAMD Boards, under which you can select Adafruit Feather M0 (SAMD21). If this is not an option, uninstall and reinstall the board managers and restart the IDE.


### 3. Install necessary libraries
Install these libraries through the library manager, accessed through Tools > Manage Libraries, Sketch > Include Library > Manage Libraries, or the books icon on the side in Version 2. Search for the name and install the library, as well as any dependencies if prompted. Authors are provided below to resolve any ambiguity.
- Arduino Low Power (by Arduino)
- OneWire (by Jim Studt et al.)
- DallasTemperature (by Miles Burton)
- SD (by Arduino, SparkFun)
- IridiumSBD (by Mikal Hart)
- CSV Parser (by Michal Borowski)
- SDI-12 (by Kevin M. Smith or Sara Damiano)
- QuickStats (by David Dubins)

Install these libraries by downloading the source code from the linked GitHub repositories. On the GitHub page, press the green Code button and download as ZIP. Do not extract the zipped files.<br>
To install the library, go to Sketch > Include Library > Add .ZIP Library in the Arduino IDE and select the downloaded zip file. 
- Arduino MemoryFree from https://github.com/mpflaga/Arduino-MemoryFree
- RTClib from https://github.com/adafruit/RTClib

**Note:** It is important to install the exact libraries listed here, as different libraries of the same or similar names do exist but will not be compatible with the RemoteLogger library. 


### 4. Upload code to the Feather M0 Adalogger

#### Connect to the MCU (the Adalogger)
Plug your Adalogger into a USB port on your computer with a provided micro-USB cord. If the Adalogger is plugged into the FeatherWing (which has screw terminals along either side), make sure the power switch is turned to ON.<br>
Open the board selector dropdown in the top left corner of the IDE or go to Tools > Board. When the Adalogger is connected, its board and port should appear automatically. If they do not, select Adafruit Feather M0 (SAMD21) from Tools > Board > Adafruit SAMD Boards and a port labelled "COM # (USB)" under Tools > Port. If no such port appears, make sure the power switch is on and that your USB cord connection is secure on both ends. It is safe to unplug and plug back in.

#### Compile code
This is an optional step, but a good one to confirm that you have all the necessary libraries and that there are no errors in your code before attempting to upload. Ensure that you have selected the Adafruit Feather M0 board (the port doesn't matter for compiling as it will not actually communicate with the MCU).<br>
Compile the code with the checkmark button in the top left or at Sketch > Verify/Compile. The output panel at the bottom should show progress of the compilation. There may be some red text; if there are errors the compilation will fail. However, if the last line of output shows "Sketch uses XXX bytes (XX%) of program storage space", compilation was successful and there are no mistakes in your code. If you get errors saying that you are missing libraries, go back to step 3 to install the missing libraries. 

#### Upload code
To upload code to the MCU, press the right-facing arrow button in the top left corner of the IDE or use Sketch > Upload. The code will first compule, then attempt to upload to the selected board over the selected port. You should get quite a bit of output in the output panel, including progress bars of the upload. The red LED next to the micro-USB port on the Adalogger should turn on and pulse quickly while the code is uploading.<br>
If the upload fails, check that you have selected the correct board and are connected to the USB COM port. This information should be visible in the bottom right corner of the IDE. You can also put the Adalogger in "bootloader" mode by quickly double-pressing the Reset button (the only button on top of the Adalogger). Turn to bootloader mode only once compilation is finished and the upload has started.<br>
Once the code is uploaded, it will start executing immediately and continue to loop until power is interrupted (either by turning off the power or pressing the Reset button).