/**
 * compare file names to see if they are binary files with correct name length (for messages for timelapse camera w/ telemetry)
 * most code taken from listfiles example from Adafruit SD library 
 * 
 * Author: Rachel Pagdin
 * July 5, 2024
*/
#include <SPI.h>
#include <SD.h>

#define SD_CS 4

char targetSuffix[] = ".BIN";
char tempSuffix[5];     // hold the suffix of the filename under examination

File root;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial);

  Serial.print("Initializing SD card...");

  if (!SD.begin(SD_CS)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  root = SD.open("/");

  printDirectoryWithCompare(root, 0);

  Serial.println("done!");
}

void loop() {
  // put your main code here, to run repeatedly:

}

void printDirectoryWithCompare(File dir, int numTabs){
  while(true){

    File entry = dir.openNextFile();
    if(!entry){   // no more files
      break;
    }
    Serial.print(entry.name());
    
    if (entry.isDirectory()) {
      Serial.println(F("/"));
      printDirectoryWithCompare(entry, numTabs + 1);
    } else {
      Serial.print("\t\t");
      Serial.println(compareNames(entry.name()));
    }
    entry.close();
    
  }
}


bool compareNames(char * name){      

  strncpy(tempSuffix, name+8, 4);       // get substring 
  Serial.print(tempSuffix);       Serial.print("\t\t");

  if(strncmp(targetSuffix, tempSuffix, 4) == 0) { return true; }
  else { return false; }
}
