/**
 * Timelapse camera with telemetry using ArduCAM
 * designed for use with TPL set to 2 hour intervals
 * 
 * Author: Rachel Pagdin
 * July 5, 2024
 */

/**
 * CIRCUIT:
 * TPL: Done --> A0
 * ArduCAM: CS --> 13
 * RockBlock: no sleep pin
 */

#include <SPI.h>                    // for SD card and communication with ArduCAM
#include <SD.h>                     // for SD card
#include <Wire.h>                   // for I2C communication with ArduCAM
#include <ArduCAM.h>                // for ArduCAM
#include <memorysaver.h>            // for ArduCAM
#include <stdio.h>                  // not 100% sure what this is for
#include <IridiumSBD.h>             // for Iridium RockBlock modem
#include <CSV_Parser.h>             // for tracking file
#include <RTClib.h>                 // for clock
#include <time.h>

#define SD_CS 4                         // chip select pin for SD card
#define CAM_CS 13                       // chip select pin for ArduCAM
#define IridiumSerial Serial1           // Serial1 is UART TX/RX pins on Feather M0 Adalogger
#define HEADER_LENGTH 14                // header for outgoing messages
#define PAYLOAD_LENGTH 320              // payload on outgoing image messages
#define LED 8                           // builtin led on Adalogger
#define TPL_DONE A0                     // done pin on TPL
#define VERTICAL_COMPRESS_FACTOR 2
#define HORIZONTAL_COMPRESS_FACTOR 2
#define FRAMES_NUM 0x00                 // take one JPEG photo per loop

// variables for ArduCAM operation
uint8_t resolution = OV5642_640x480;        // resolution, lines, and columns must match
const uint16_t line = 480, column = 640;    // resolution of captured RAW images
uint16_t imageSize = (uint16_t) (line / VERTICAL_COMPRESS_FACTOR) * (column / HORIZONTAL_COMPRESS_FACTOR) / 2;
uint16_t outLine = (uint16_t) (line / VERTICAL_COMPRESS_FACTOR), outColumn = (uint16_t) (column / HORIZONTAL_COMPRESS_FACTOR);          // resolution of compressed image (to reconstruct after transmission)
ArduCAM myCAM(OV5642, CAM_CS);
bool is_header = false;

// variables for message management
File infile, outfile;
char binName[13], imageName[13];
byte header[HEADER_LENGTH];             // buffer for header
byte unselected[PAYLOAD_LENGTH];        // buffer for unselected 8-bit data  /**TODO: does this cause RAM issues? could read in in batches*/
byte uncompressed[PAYLOAD_LENGTH * 2];  // buffer for uncompressed 8-bit data (every fourth pixel)
byte transfer[PAYLOAD_LENGTH];          // buffer for transferring data from RAW to BIN
byte id;
char idFile[] = "ID.CSV";
char targetSuffix[] = ".BIN";
char tempSuffix[5];                     // hold the suffix of the filename under examination    

// variables for telemetry
byte toSend[HEADER_LENGTH + PAYLOAD_LENGTH];
int err, signalQuality = -1;
File file;
File logFile;
char fileName[13];
byte year, month, day, hour;
IridiumSBD modem(IridiumSerial);

char trackingFile[] = "TRACKING.CSV";        // track hours
RTC_PCF8523 rtc;                         // real time clock


void setup(){
    /* SETUP */
    // Serial startup
    Serial.begin(115200);
    while(!Serial);

    // TPL setup
    pinMode(TPL_DONE, OUTPUT);
    digitalWrite(TPL_DONE, LOW);        // hold done pin low

    // RTC setup        
    Wire.begin();           // start I2C (also for RTC)
    while(!rtc.begin()){            // start RTC
        Serial.println(F("RTC error"));
    };   
    Serial.println(F("RTC detected")); 

    // startup SD card
    while(!SD.begin(SD_CS)){
        Serial.println(F("SD Card Error!"));delay(1000);
    }
    Serial.println(F("SD Card detected."));

    // startup ArduCAM (JPEG format)
    pinMode(CAM_CS, HIGH);
    digitalWrite(CAM_CS, HIGH);
    SPI.begin();
    cameraStartupChecks();          // check busses, reset CPLD
    myCAM.set_format(JPEG);
    myCAM.InitCAM();
    myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
    myCAM.clear_fifo_flag();
    myCAM.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);


    /* OPERATION */
    // check the time
    DateTime presentTime = rtc.now();
    year = (uint8_t) (presentTime.year() - 2000);
    month = presentTime.month();
    day = presentTime.day();
    hour = presentTime.hour();

    // capture JPEG image
    captureJPEGImage();
    incrementSamples();

    // if it's time to capture image to send: (24 hours)
    if(numSamples() == 12){
        // change ArduCAM to RAW mode
        myCAM.set_format(RAW);
        myCAM.InitCAM();
        myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);      // timing settings

        // capture RAW image and save
        captureRAWImage();
        resetSampleCounter();

        // generate messages from RAW image
        makeMessages();
    }

    // if there are messages waiting to be sent:
    byte ctr = 0, err_time = ISBD_NO_NETWORK;           // error initiated arbitrarily - just can't be 0
    while((messagesWaiting() == true) && (ctr < 12)){

        if (ctr == 0){ 
            startModem();           // first time through -- start the modem
            for(int i = 0; i < 5; i++){     // try 5 times to get a signal          /** TODO: this could be smarter */
                signalQuality = testSignalQuality();
                delay(1000);    
            }
            if(signalQuality < 1) break;        // don't bother trying to send messages if no signal
        }
        if (err_time != ISBD_SUCCESS){      // sync clock if you haven't yet on this cycle
            err_time = syncClock();
        }      

        // send & delete 12 messages (probably roughly 8 minutes - wait ~40 sec btwn messages)
        sendNextMessage(fileName);

        ctr++;
        delay(40 * 1000UL);         // wait 40 seconds (don't overload the network)
    }
}

void loop(){
    // TPL done
    digitalWrite(TPL_DONE, LOW); delay(50);
    digitalWrite(TPL_DONE, HIGH); delay(50);
}


/* ARDUCAM */

void cameraStartupChecks(){
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
 * capture JPEG image and save to SD card with timestamp as file name
 */
void captureJPEGImage(){
    uint8_t temp = 0, temp_last = 0;
    uint32_t length = 0;
    static int i = 0;
    static int k = 0;
    byte buf[256];

    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();
    myCAM.OV5642_set_JPEG_size(resolution);       // set photo resolution
    delay(1000);

    //Start capture
    myCAM.start_capture();
    Serial.println(F("start JPEG capture."));
    while ( !myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));      // wait for capture to be finished
    Serial.println(F("CAM Capture Done."));

    // save to the SD card
    length = myCAM.read_fifo_length();
    Serial.print(F("The fifo length is: "));  Serial.println(length, DEC);
    if (length >= MAX_FIFO_SIZE) {    // 8M
        Serial.println("Over size.");
        return;
    }
    if (length == 0 ) {       //0 kb
        Serial.println(F("Size is 0."));
        return;
    } 

    myCAM.CS_LOW();
    myCAM.set_fifo_burst();     //Set fifo burst mode
    i = 0;

    while ( length-- ) {
        temp_last = temp;
        temp =  SPI.transfer(0x00);

        //Read JPEG data from FIFO
        if ( (temp == 0xD9) && (temp_last == 0xFF) ){ //If find the end ,break while,
  
            buf[i++] = temp;  //save the last  0XD9     
            //Write the remain bytes in the buffer
            myCAM.CS_HIGH();
            outfile.write(buf, i);    
            //Close the file
            outfile.close();
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
                outfile.write(buf, 256);
                i = 0;
                buf[i++] = temp;
                myCAM.CS_LOW();
                myCAM.set_fifo_burst();
            }        
        } else if ((temp == 0xD8) & (temp_last == 0xFF)) {
        is_header = true;
        myCAM.CS_HIGH();

        // Create a JPG file
        snprintf(imageName, 13, "%02d%02d%02d%02d.jpg", year, month, day, hour); 
        outfile = SD.open(imageName, O_WRITE | O_CREAT | O_TRUNC);
        if (! outfile) {
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

    //Clear the capture done flag
    myCAM.clear_fifo_flag();
}

/**
 * capture RAW image and save to SD card with timestamp as file name
 */
void captureRAWImage(){
    char VL;
    byte buf[256];
    static int k = 0,m = 0;
    int i,j = 0;

    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();
    myCAM.OV5642_set_RAW_size(resolution);
    delay(1000);  

    // capture
    myCAM.start_capture();
    Serial.println(F("start capture."));
    while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)); 
    Serial.println(F("CAM Capture Done.")); 

    Serial.println(F("Saving the image, please wait..."));

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
    if(m > 0 )//Write the remaining image data to file from buffer
        outfile.write( buf, m );m = 0;
    //Close the file  
    outfile.close(); 
    Serial.println("Image save OK.");
    //Clear the capture done flag
    myCAM.clear_fifo_flag();
}



/* TRACKING */

/**
 * copied from RemoteLogger library
 * helper for trackers
 */
void writeToCSV(const char * header, const char * datastring, char * outname){
    /* If file doesn't exist, write header and data, otherwise only write data */
    if (!SD.exists(outname)){
        outfile = SD.open(outname, FILE_WRITE);
        if (outfile){
            outfile.println(header);       // write header to file
            outfile.println(datastring);       // write data to file
        }
        outfile.close();       // make sure the file is closed
    } else {
        outfile = SD.open(outname, FILE_WRITE);
        if (outfile) {
            outfile.println(datastring);       // write data to file
        }
        outfile.close();       // make sure the file is closed
    }
}

/**
 * functions to track number of cycles (images taken) since an image was captured for sending
 * increment every time a JPEG file is saved to SD
 * reset when a RAW image is saved
 */
void incrementSamples(){
    writeToCSV("n", "1", trackingFile);
}
uint8_t numSamples(){
    CSV_Parser cp("s", true, ',');     //for reading from TRACKING.csv
    cp.readSDfile(trackingFile);
    return cp.getRowsCount() - 1; //don't count the header
}
void resetSampleCounter(){
    SD.remove(trackingFile);
}

/**
 * functions to manage ID number in SD card nonvolatile memory
 * increment whenever a sequence of messages is generated
 * reset when it reaches 255 (will overflow a single byte after that)
 */
void incrementID(){
    writeToCSV("n", "2", idFile);
}
uint8_t getIDNum(){
    CSV_Parser cp("s", true, ',');     //for reading from TRACKING.csv
    cp.readSDfile(idFile);
    return cp.getRowsCount() - 1; //don't count the header
}
void resetIDNum(){
    SD.remove(idFile);
}



/* TELEMETRY */

void makeMessages(){
    uint32_t start = 0, extra, position = 0, i;
    uint8_t seq = 1, totSeq;
    bool even;
    byte first, second, comp;

    incrementID();
    id = getIDNum();
    if(id == 255){ resetIDNum(); }      // to avoid overflowing a single byte, resets every 255 cycles

    // determine total number of messages for image
    if((imageSize % PAYLOAD_LENGTH) == 0) { totSeq = imageSize / PAYLOAD_LENGTH;  extra = PAYLOAD_LENGTH; }       // divides evenly
    else {
        totSeq = (imageSize / PAYLOAD_LENGTH) + 1; 
        extra = imageSize - (PAYLOAD_LENGTH * (totSeq-1));      // length of last message
    }
    Serial.print(F("compressed image size (B): ")); Serial.println(imageSize);
    Serial.print(F("total messages in sequence: ")); Serial.println(totSeq);

    // build header
    header[1] = totSeq;
    header[2] = imageSize;      // 2 bytes
    header[4] = byte((outLine & 0x0ff0) >> 4);
    header[5] = byte((outLine & 0x000f) | ((outColumn & 0x0f00) >> 8));
    header[6] = byte(outColumn & 0x00ff);
    header[7] = year;
    header[8] = month;
    header[9] = day;
    header[10] = hour;
    header[11] = id;            
    header[12] = 0x0000;         // spacer (for checksum later)

    // open image file, generate sequence of messages
    infile = SD.open(imageName, FILE_READ);     // image name is generated when RAW image is saved -- still stored in global
    if(infile){
        while(start < (line * column)){       // haven't reached the end of the image file
            // below here --> compressing 
            i = 0;      // start at the beginning of the uncompressed buffer

            // read data to be compressed into payload in 8 separate batches to avoid RAM overflow
            for(int k = 0; k < 8; k++){         // 8 payload lengths --> uncompressed buffer (652 B)
                infile.seek(start);
                infile.read(unselected, PAYLOAD_LENGTH);

                start += PAYLOAD_LENGTH;            // go to next start position

                while((position < start) && (position < (line * column))){
                    if((position % column) == 0){       // at the start of a row
                        if((position % (column * 2) == 0)){         // at the start of an even row
                            even = true;
                        } else {
                            even = false;
                            position += (column-1);         // odd line --> skip over it
                        }
                    }
                    if((even == true) && ((position % 2) == 0)) {      // even row even column
                        uncompressed[i] = unselected[position-(start-PAYLOAD_LENGTH)];
                        i++;
                    }
                    position++;
                }
            }

            // compress each pair of bytes into one byte
            for(int j = 0; j < PAYLOAD_LENGTH; j++){
                first = uncompressed[2*j] & 0xf0;       // extract first half of first byte
                second = uncompressed[2*j+1] >> 4;      // extract first half of second byte
                comp = first | second;                  // OR together to combine into one byte

                transfer[j] = comp;
            }

            // save the message to a binary file 
            snprintf(binName, 13, "%02d%03d%03d.BIN", id, seq, totSeq);      // generate file name
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
        }
    } else {
        Serial.println(F("issue opening raw file to read"));
    }
    infile.close();
}

uint8_t startModem(){
    IridiumSerial.begin(19200);
    modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);

    Serial.println(F("starting modem..."));
    err = modem.begin();
    if (err != ISBD_SUCCESS){
        Serial.print(F("begin failed: error ")); Serial.println(err);
        if(err == ISBD_NO_MODEM_DETECTED){
            Serial.println(F("no modem detected -- check wiring"));
        }
        return err;
    }
    Serial.println(F("modem started!"));
    return 1;
}

uint8_t syncClock(){
    struct tm t;
    int err_time = modem.getSystemTime(t);
    if (err_time == ISBD_SUCCESS) {
        String pre_time = rtc.now().timestamp();
        Serial.print("pretime: "); Serial.println(pre_time);
        rtc.adjust(DateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec));
        String post_time = rtc.now().timestamp();
        Serial.print("posttime: "); Serial.println(post_time);
    }
    return err_time;
}

/**
 * determine if there are messages waiting to be sent based on filename structure
 * if a message is found, the name of the file containing the message will be written to global fileName variable
 */
bool messagesWaiting(){
    // note to self: fileName (global) holds name of file that was found (next to send)
    File dir = SD.open("/");       // open root directory
    while(true){
        File entry = dir.openNextFile();
        if(!entry){    // no more files
            break;
        }
        if(isMessage(entry.name()) == true){
            //fileName = entry.name();
            strncpy(fileName, entry.name(), 12);
            return true;
        }
    }
    return false;
}
bool isMessage(char * name){
    strncpy(tempSuffix, name+8, 4);       // get substring (suffix)

    if(strncmp(targetSuffix, tempSuffix, 4) == 0) { return true; }
    else { return false; }
}

uint8_t sendNextMessage(char * fileName){
    outfile = SD.open(fileName, FILE_READ);         // open the file waiting to be sent
    outfile.read(toSend, HEADER_LENGTH + PAYLOAD_LENGTH);
    outfile.close();

    Serial.println(F("trying to send the message..."));
    err = modem.sendSBDBinary(toSend, HEADER_LENGTH + PAYLOAD_LENGTH);
    if(err != ISBD_SUCCESS){
        err = modem.sendSBDBinary(toSend, HEADER_LENGTH + PAYLOAD_LENGTH);      // try again to send
    }
    if(err != ISBD_SUCCESS){
        Serial.print(F("send failed: error ")); Serial.println(err);
        if(err == ISBD_SENDRECEIVE_TIMEOUT){
            Serial.println(F("try again with a better view of the sky"));
        }
        return err;
    }

    Serial.println(F("send worked!"));
    SD.remove(fileName);        // delete the message file from the SD card
    return err;
}

uint8_t testSignalQuality(){
    Serial.println(F("checking signal quality..."));
    err = modem.getSignalQuality(signalQuality);
    if(err != ISBD_SUCCESS){
        Serial.print(F("signal quality failed: error ")); Serial.println(err);
        return err;
    }
    Serial.print(F("on a scale of 0 to 5, signal quality is currently ")); Serial.println(signalQuality);
    return signalQuality;
}

