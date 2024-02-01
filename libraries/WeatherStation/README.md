# How to install WeatherStation library in Arduino IDE

## Installing from a ZIP file:
Arduino IDE supports adding libraries to sketches from an appropriate compressed ZIP file. 

### 1. Download library files from GitHub:

There are two easy options for downloading the library files from GitHub: 
1. Download files individually and compress on your device
2. Use a GitHub download tool to compress the folder before download (recommended)

#### Downloading files individually:
1. Navigate to https://github.com/bevingtona/diy_hydrometric/tree/rp_edits/libraries/WeatherStation. 
2. Download WeatherStation.cpp (the library source file) and WeatherStation.h (the library header file) from the repository.
3. Store both library files in a new folder named WeatherStation.
4. Make a compressed (zipped) folder containing the WeatherStation folder. It does not matter what you name the folder name but do not change the names of either library file. 

#### Compressing pre-download:
1. Navigate to https://github.com/bevingtona/diy_hydrometric/tree/rp_edits/libraries.
2. Copy the URL of the folder and open https://download-directory.github.io/ (or a similar GitHub download tool).
3. Paste the repository URL and press ENTER. The compressed file will download. 


### 2. Add library to sketch in Arduino IDE

Now that you have the library files in a compressed ZIP folder, you can install the library in the Arduino IDE.

1. Open the Arduino IDE with the sketch (.ino file) that includes the WeatherStation library. 
2. In Sketch > Include Library > Add .ZIP Library... select your compressed folder. <br><br>
![Alt text](<docs/Screenshot 2024-02-01 110252.png>)<br>

3. To check if the library has installed correctly, you can check where the Arduino IDE is storing library files through File > Preferences. If you navigate to the path shown under Sketchbook location, the WeatherStation folder should be there containing both the source and header files. <br><br>
![Alt text](<docs/Screenshot 2024-02-01 110518.png>)<br>
![Alt text](<docs/Screenshot 2024-02-01 110943.png>)<br>

If you're careful about the names you can transplant the WeatherStation folder directly into the folder where the Arduino IDE is storing sketchbook files. However, installing with a .ZIP file does this automatically. 