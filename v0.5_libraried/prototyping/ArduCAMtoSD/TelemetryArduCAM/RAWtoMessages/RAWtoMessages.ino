/**
 * capture an image on ArduCAM 5MP Plus (OV5642) and save as RAW format
 * read from RAW format to produce messages of 326 bytes + 14 byte header (save as BIN) files
 * BIN files will be sent in order over Iridium satellite network
 * 
 * Author: Rachel Pagdin
 * July 2, 2024
 */

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>
#include <memorysaver.h>
#include <stdio.h>

#define SD_CS 4                     // chip select for SD card
#define CS 13                       // chip select for camera SPI
#define HEADER_LENGTH 14            // header length for Irid messages
#define PAYLOAD_LENGTH 326          // payload length for Irid messages

// sample date -- will eventually take real time from RTC
const byte year = 24;
const byte month = 7;
const byte day = 2;
const byte hour = 17;

int total_time = 0;

uint8_t resolution = OV5642_640x480;        // resolution, lines, and columns must match
const uint16_t line = 480, column = 640;
uint16_t imageSize = line * column;
ArduCAM myCAM(OV5642, CS);

File infile, outfile;
char binName[13], imageName[13];
byte header[HEADER_LENGTH];             // buffer for header
byte transfer[PAYLOAD_LENGTH];          // buffer for transferring data from RAW to BIN
byte id = 0;


void setup(){
    Serial.begin(115200);
    while(!Serial);         // wait for user to open serial monitor
    
    Serial.println(F("starting I2C..."));
    Wire.begin();

    Serial.println(F("ArduCAM Start!"));
    pinMode(CS, OUTPUT);    // select pin for SPI bus
    digitalWrite(CS, HIGH);
    SPI.begin();

    startupChecks();        // check busses, reset CPLD

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
    // capture an image and save it as a .raw file
    captureImage();

    // construct .bin with messages from .raw file
    makeMessages();

    delay(10000);
}

void startupChecks(){
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
}

/**
 * capture an image and write to RAW file
 */
void captureImage(){
    char VL;
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
    while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)); 
    Serial.println(F("CAM Capture Done."));
    total_time = millis() - total_time;
    Serial.print(F("capture total_time used (in miliseconds):"));
    Serial.println(total_time, DEC);   

    Serial.println(F("Saving the image, please wait..."));
    total_time = millis();

    snprintf(imageName, 13, "%02d%02d%02d%02d.raw", year, month, day, hour);           // produce filename from timestamp
    outfile = SD.open(imageName, O_WRITE | O_CREAT | O_TRUNC);
    if (!outfile) {
      Serial.println(F("File open error"));
      return;
    }
    
    //Save as RAW format
    for(i = 0; i < line; i++){
        for(j = 0; j < column; j++){
            VL = myCAM.read_fifo();
            buf[m++] = VL;
            if(m >= 256){
                //Write 256 bytes image data to file from buffer
                outfile.write(buf,256);
                m = 0;
            }
        }
    }
    if(m > 0 )//Write the left image data to file from buffer
        outfile.write( buf, m );m = 0;
    //Close the file  
    outfile.close(); 
    Serial.println("Image save OK.");
    //Clear the capture done flag
    myCAM.clear_fifo_flag();
}


void makeMessages(){
    uint32_t start = 0, extra;
    uint8_t seq = 1, totSeq;

    id++;

    // determine total number of messages for image
    if(imageSize % PAYLOAD_LENGTH == 0) { totSeq = imageSize / PAYLOAD_LENGTH;  extra = PAYLOAD_LENGTH; }       // divides evenly
    else {
        totSeq = (imageSize / PAYLOAD_LENGTH) + 1; 
        extra = imageSize - (PAYLOAD_LENGTH * (totSeq-1));      // length of last message
    }

    // build header
    header[1] = totSeq;
    header[2] = imageSize;      // 2 bytes
    header[4] = byte((line & 0x0ff0) >> 4);
    header[5] = byte((line & 0x000f) | ((column & 0x0f00) >> 8));
    header[6] = byte(column & 0x00ff);
    header[7] = year;
    header[8] = month;
    header[9] = day;
    header[10] = hour;
    header[11] = id;
    header[12] = 0x0000;      // spacer (for checksum later)

    // open image file
    infile = SD.open(imageName, FILE_READ);
    if(infile){
        while(start < imageSize){       // haven't reached the end of the image file
            // read 326 bytes of the image file to the transfer buffer
            infile.seek(start);     //go to starting position in file
            infile.read(transfer, PAYLOAD_LENGTH);

            snprintf(binName, 13, "%02d%03d%03d.bin", id, seq, totSeq);      // generate file name
            header[0] = seq;

            Serial.print(F("making file ")); Serial.println(binName);
            outfile = SD.open(binName, O_CREAT | O_WRITE);
            if(outfile){
                if(seq != totSeq){      // not at the last -- messages full length
                    outfile.write(&header[0], HEADER_LENGTH);
                    outfile.write(&transfer[0], PAYLOAD_LENGTH);
                } else {
                    outfile.write(&header[0], HEADER_LENGTH);
                    outfile.write(&transfer[0], extra);
                }
            } else {
                Serial.print(F("issue opening bin file ")); Serial.println(binName);
            }
            outfile.close();

            seq++;
            start += PAYLOAD_LENGTH;        //skip ahead in the file to start of next payload
        }
    } else {
        Serial.println(F("issue opening raw file to read"));
    }
    infile.close();
}
