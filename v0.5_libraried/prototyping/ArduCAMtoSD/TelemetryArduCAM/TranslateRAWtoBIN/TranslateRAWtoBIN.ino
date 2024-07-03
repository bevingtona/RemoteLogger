/**
 * take .RAW file and convert to .BIN file
 * 
 * Author: Rachel Pagdin
 * July 2, 2024
 */

#include <SD.h>
#include <SPI.h>

File infile, outfile;
char *inName = "test.raw";
char *outName = "test.bin";

const byte arraySize = 16;
byte array[arraySize];
byte nibbles[arraySize / 2];

const byte SD_CS = 4;

void setup(){
    Serial.begin(115200);
    while(!Serial);
    Serial.print(F("initializing SD card..."));
    if(!SD.begin(SD_CS)){
        Serial.println(F("SD failed"));
        while(1);
    }
    Serial.println(F("SD card good"));

    // read from RAW file
    readRaw();

    // translate pairs of bytes to nibbles
    convertToNibbles();

    // write to BIN file
    writeToBin();
}

void loop(){

}

void readRaw(){
    // read from RAW file
    infile = SD.open(inName, FILE_READ);
    if(infile){
        int len = infile.available();
        infile.read(array, len);
        infile.close();
    } else {
        Serial.println(F("input file open issue"));
    }
}

void convertToNibbles(){
    byte first, second, comp;
    for(int i = 0; i < arraySize / 2; i++){
        first = array[2*i] & 0xf0;        //extract first half of first byte
        second = array[2*i+1] >> 4;       //extract first half of second byte
        comp = first | second;          //OR together to combine into one byte

        nibbles[i] = comp;
    }
}

void writeToBin(){
    // write nibbles to BIN file
    outfile = SD.open(outName, O_CREAT | O_WRITE);
    if(outfile){
        Serial.print(F("writing to ")); Serial.println(inName);
        Serial.print(F("writing "));
        Serial.write(&nibbles[0], arraySize/2);     // write array to Serial console
        Serial.println();
        outfile.write(&nibbles[0], arraySize/2);    // write array to file
        outfile.close();
        Serial.println(F("done writing"));
    } else {
        Serial.println(F("output file open issue"));
    }
}