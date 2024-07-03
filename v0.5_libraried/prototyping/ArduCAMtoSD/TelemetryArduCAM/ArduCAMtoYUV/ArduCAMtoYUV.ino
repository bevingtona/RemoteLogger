/**
 * read images as YUV format from ArduCAM to SD card
 * 
 * Author: Rachel Pagdin
 * June 27, 2024
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

ArduCAM myCAM(OV5642, CS);

uint8_t read_fifo_burst(ArduCAM myCAM);

void setup(){
    uint8_t vid, pid;
    uint8_t temp;

    Wire.begin();           // I2C communication bus

    Serial.begin(115200);
    while(!Serial);
    Serial.println(F("ArduCAM Start!"));

    pinMode(CS, OUTPUT);
    digitalWrite(CS, HIGH);
    SPI.begin();        // SPI communication bus

    // reset the CPLD (complex programmable logic device)
    myCAM.write_reg(0x07, 0x80);    // this writes to SPI bus
    delay(100);
    myCAM.write_reg(0x07, 0x00);    
    delay(100);

    // check the SPI bus
    while(1){
        myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
        temp = myCAM.read_reg(ARDUCHIP_TEST1);
        if(temp != 0x55) {
            Serial.println(F("SPI interface Error!"));
            delay(1000);
            continue;
        }else{
            Serial.println(F("SPI interface OK."));
            break;
        }
    }

    // check the module is OV5642
    while(1){
        myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
        myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
        if ((vid != 0x56) || (pid != 0x42)){
            Serial.println(F("Can't find OV5642 module!"));
            delay(1000);
            continue;
        }else{
            Serial.println(F("OV5642 detected."));
            break;      
        }
    }

    // initialize SD card
    while(!SD.begin(SD_CS)){
        Serial.println(F("SD Card Error!"));delay(1000);
    }
    Serial.println(F("SD Card detected."));

    initCam();      // set up for YUV input
    myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);          // SPI
    myCAM.clear_fifo_flag();        // SPI
    myCAM.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
}


void loop(){
    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();
    //set_YUV_640x480();
    myCAM.OV5642_set_RAW_size(OV5642_640x480);
    delay(1000);

    myCAM.start_capture();          //start the FIFO -- change SPI register
    Serial.println(F("start capture."));
    total_time = millis();
    while ( !myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)){
          Serial.print(myCAM.read_reg(ARDUCHIP_TRIG)); Serial.print(F(" "));  }   // wait for capture to be finished
    Serial.println(F("CAM Capture Done."));
    total_time = millis() - total_time;
    Serial.print(F("capture total time used (ms): "));
    Serial.println(total_time, DEC);
    
    total_time = millis();
    read_fifo_burst(myCAM);       // write to the SD card
    total_time = millis() - total_time;
    Serial.print(F("save capture total time used (ms): "));
    Serial.println(total_time, DEC);

    //Clear the capture done flag
    myCAM.clear_fifo_flag();
    delay(10000);      // wait between captures

}


/**
 * initialize camera for YUV output
 * replicate ArduCAM::InitCAM() but for raw YUV output 
 * see OV5642 datasheet and software application notes (PDFs) for details on registers
 */
void initCam(){
    byte reg_val;
    myCAM.wrSensorReg16_8(0x3008, 0x80);        //put in reset software mode
    myCAM.wrSensorRegs16_8(OV5642_QVGA_Preview);        // turn to 15fps YUV (QGVA preview) - resolution 320x240

    myCAM.wrSensorReg16_8(0x4740, 0x21);        // polarity stuff (DS p.105)
    myCAM.wrSensorReg16_8(0x501e, 0x6a);        // RGB dither control (set to system control)
    myCAM.wrSensorReg16_8(0x5002, 0xf0);        // disable YUV to RGB (0xf8 to enable)
    myCAM.wrSensorReg16_8(0x501f, 0x00);        // select ISP YUV
    myCAM.wrSensorReg16_8(0x4300, 0x30);        // YUV422 with Y U Y V ... pattern

    // set mirror and vertical flip
    myCAM.rdSensorReg16_8(0x3818, &reg_val);
    myCAM.wrSensorReg16_8(0x3818, (reg_val | 0x60) & 0xff);
    myCAM.rdSensorReg16_8(0x3621, &reg_val);
    myCAM.wrSensorReg16_8(0x3621, reg_val & 0xdf);
}


/**
 * register definitions in OV5642 datasheet p.123
 */
void set_YUV_640x480(){
    // set horizontal width to 680 pixels
    myCAM.wrSensorReg16_8(0x3808, 0x02);
    myCAM.wrSensorReg16_8(0x3809, 0x80);

    // set vertical width to 480 pixels
    myCAM.wrSensorReg16_8(0x380a, 0x01);
    myCAM.wrSensorReg16_8(0x380b, 0xe0);

    myCAM.wrSensorReg16_8(0xffff, 0xff);        //this is in all the dimension set functions
}

/**
 * read FIFO burst and save to .YUV file
 */
uint8_t read_fifo_burst(ArduCAM myCAM){
  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  static int i = 0;
  static int k = 0;
  char str[8];
  File outFile;
  byte buf[256]; 

  length = myCAM.read_fifo_length();
  Serial.print(F("The fifo length is: "));
  Serial.println(length, DEC);
  if (length >= MAX_FIFO_SIZE) {    // 8M
    Serial.println("Over size.");
    return 0;
  }
  if (length == 0 ) {       //0 kb
    Serial.println(F("Size is 0."));
    return 0;
  } 

  myCAM.CS_LOW();
  myCAM.set_fifo_burst();//Set fifo burst mode
  i = 0;
  while ( length-- ) {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    Serial.print(temp); Serial.print(F(" ")); 

    //Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ){ //If find the end ,break while,
  
      buf[i++] = temp;  //save the last  0XD9     
      //Write the remain bytes in the buffer
      myCAM.CS_HIGH();
      outFile.write(buf, i);    
      //Close the file
      outFile.close();
      Serial.println(F("OK"));
      is_header = false;
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();
      i = 0;
    }  
    if (is_header == true) { 
      //Write image data to buffer if not full
      if (i < 256) {
        buf[i++] = temp;
      } else {
        //Write 256 bytes image data to file
        myCAM.CS_HIGH();
        outFile.write(buf, 256);
        i = 0;
        buf[i++] = temp;
        myCAM.CS_LOW();
        myCAM.set_fifo_burst();
      }        
    } else if ((temp == 0xD8) & (temp_last == 0xFF)) {
      is_header = true;
      myCAM.CS_HIGH();
      //Create a avi file
      k = k + 1;
      itoa(k, str, 10);
      strcat(str, ".yuv");
      //Open the new file
      outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
      if (! outFile) {
        Serial.println(F("File open failed"));
        while (1);
      }
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();   
      buf[i++] = temp_last;
      buf[i++] = temp;   
    }
  }
  myCAM.CS_HIGH();
  return 1;
}
