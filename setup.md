# Setting up the MCU the first time

### 1. Install Arduino IDE
Install from https://learn.adafruit.com/adafruit-feather-m0-basic-proto/setup download link (or just Google it)

### 2. Set up package to support Adafruit boards 
See tutorial https://learn.adafruit.com/adafruit-feather-m0-basic-proto/setup for details, basics are here

#### Get the board manager package
In Arduino IDE, go to File > Preferences<br>
Paste https://adafruit.github.io/arduino-board-index/package_adafruit_index.json into Additional Board Manager URLs box<br>
Click OK

#### Install board manager
In Arduino IDE, go to Tools > Board > Boards Manager<br>
Search and install Arduino SAMD Boards and Adafruit SAMD Boards (latest versions)<br>
Restart the Arduino IDE<br>
Now you should be able to select the Adafruit Feather M0 as the board you want through the Board Manager


### 3. Install necessary libraries 
Libraries can be installed by searching in the Library Manager in Arduino IDE or, if necessary, downloading .ZIP files and installing (see below).<br>

Install the following libraries for the Remote Logger:
- Arduino Low Power by Arduino
- OneWire 
- DallasTemperature by Miles Burton
- Arduino MemoryFree from https://github.com/mpflaga/Arduino-MemoryFree (download as zip)
- SD by Arduino, Sparkfun
- IridiumSBD by Mikal Hart
- CSV Parser by Michal Borowski
- SDI-12 by Kevin M. Smith (or Sara Damiano)
- QuickStats by David Dubins
- RTClib from https://github.com/adafruit/RTClib (download as zip)

Install any dependencies it gives you the option to install.

#### To install from a zip: 
In Arduino IDE, go to Sketch > Include Library > Add a .ZIP Library<br>
Select your zip file containing the library files and follow prompts.

#### To check where your library files are installed (or change it):
In Arduino IDE, go to File > Preferences<br>
The Sketchbook Location path is where your libraries folder is (named /libraries)

### 4. Upload code to the MCU
Plug the MCU (Feather M0 board) into one of the USB ports. Select the board and port from the top left dropdown.<br>
Click the Upload button (circle with right-facing arrow) to compile and send the code to the device. Once it has successfully uploaded the MCU should run the setup code once and then run the loop code repeatedly as long as power is supplied. Resetting the board or interrrupting the power supply will start it at the beginning of the setup code again. 