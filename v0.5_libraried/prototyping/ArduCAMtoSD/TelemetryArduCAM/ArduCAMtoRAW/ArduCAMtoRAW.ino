/**
 * read images as RAW format from ArduCAM to SD card
 * eventually want to extract just one colour (maybe green?) and send that (possibly scaled down to 4 bit colour)
 * uses code from example ArduCAM_Mini_5MP_Plus_OV5642_RAW.ino (from ArduCAM library)
 * 
 * not 100% sure how but this produces 8-bit grayscale images when run through python script
 * 
 * Author: Rachel Pagdin
 * July 2, 2024
 */

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>
#include "memorysaver.h"

#define   FRAMES_NUM    0x00        // number of photos to take per loop - 0x00: 1, 0x01: 2, ..., 0x06: 7, 0xFF: until FIFO full
#define SD_CS 4                 // SD chip select (4 on Feather M0 Adalogger)
const int CS = 13;              // pin attached to CS on ArduCAM

bool is_header = false;
int total_time = 0;

uint8_t resolution = OV5642_640x480;
uint32_t line,column;
ArduCAM myCAM(OV5642, CS);
//uint8_t saveRAW(void);


void setup(){
    Wire.begin();       // I2C bus

    Serial.begin(115200);
    while(!Serial);
    Serial.println(F("ArduCAM Start!"));

    pinMode(CS, OUTPUT);    // select pin for SPI bus
    digitalWrite(CS, HIGH);
    SPI.begin();

    //Reset the CPLD (complex programmable logic device)
    myCAM.write_reg(0x07, 0x80);
    delay(100);
    myCAM.write_reg(0x07, 0x00);
    delay(100); 

    // check SPI bus
    uint8_t temp;
    while(1){
        myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
        temp = myCAM.read_reg(ARDUCHIP_TEST1);
        if(temp != 0x55){
            Serial.println(F("SPI interface Error!"));
            delay(1000);continue;
        }else{
            Serial.println(F("SPI interface OK."));break;
        }
    }

    // check camera module is OV5642
    uint8_t vid, pid;
    while(1){
        myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
        myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
        if ((vid != 0x56) || (pid != 0x42)){
            Serial.println(F("Can't find OV5642 module!"));
            delay(1000);continue;
        }else{
            Serial.println(F("OV5642 detected."));break;      
        }
    }

    //Initialize SD Card
    while(!SD.begin(SD_CS)){
        Serial.println(F("SD Card Error!"));delay(1000);
    }
    Serial.println(F("SD Card detected."));

    myCAM.set_format(RAW);          //set capture format to RAW
    myCAM.InitCAM();                //init module
    myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);      //timing settings
} 


void loop(){
    File outFile;
    char VL;
    char str[8];
    byte buf[256];
    static int k = 0,m = 0;
    int i,j = 0;

    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();
    myCAM.OV5642_set_RAW_size(resolution);delay(1000);  

    // capture
    myCAM.start_capture();
    Serial.println(F("start capture."));
    total_time = millis();
    while ( !myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)); 
    Serial.println(F("CAM Capture Done."));
    total_time = millis() - total_time;
    Serial.print(F("capture total_time used (in miliseconds):"));
    Serial.println(total_time, DEC);   

    Serial.println(F("Saving the image, please wait..."));
    total_time = millis();

    k = k + 1;
    itoa(k, str, 10); 
    strcat(str,".raw");        //Generate file name
    outFile = SD.open(str,O_WRITE | O_CREAT | O_TRUNC);
    if (! outFile) {
      Serial.println(F("File open error"));
      return;
    }
    if(resolution == OV5642_640x480 ){
        line = 640;column = 480;
    }else if( resolution == OV5642_1280x960 ){
        line = 1280;column = 960;
    }else if( resolution == OV5642_1920x1080 ){
        line = 1920;column = 1080;
    }else if( resolution == OV5642_2592x1944 ){
        line = 2592;column = 1944;
    }
    //Save as RAW format
    for(i = 0; i < line; i++){
        for(j = 0; j < column; j++){
            VL = myCAM.read_fifo();
            buf[m++] = VL;
            if(m >= 256){
                //Write 256 bytes image data to file from buffer
                outFile.write(buf,256);
                m = 0;
            }
        }
    }
    if(m > 0 )//Write the left image data to file from buffer
      outFile.write( buf, m );m = 0;
    //Close the file  
    outFile.close(); 
    Serial.println("Image save OK.");
    //Clear the capture done flag
    myCAM.clear_fifo_flag();
    delay(10000);       // wait between captures

}
