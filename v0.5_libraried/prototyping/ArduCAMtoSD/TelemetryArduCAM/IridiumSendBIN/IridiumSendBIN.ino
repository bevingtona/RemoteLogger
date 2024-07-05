/**
 * send contents of a BIN file over Iridium network
 * component of system to send images over Irid network from ArduCAM
 * 
 * Author: Rachel Pagdin
 * July 3, 2024
 */

#include <IridiumSBD.h>
#include <SPI.h>
#include <SD.h>

#define SD_CS 4             // chip select pin for SD card
#define IridiumSerial Serial1           // Serial1 is UART TX/RX pins on Feather M0 Adalogger
#define HEADER_LENGTH 14
#define PAYLOAD_LENGTH 320
#define LED 8

byte toSend[HEADER_LENGTH + PAYLOAD_LENGTH];
int err, signalQuality = -1;
File file;
File logFile;
char fileName[13];
const uint8_t totSeq = 2;
bool success[totSeq];           // which sends were successful

IridiumSBD modem(IridiumSerial);

void setup(){
    // start Serial
    Serial.begin(115200);
    while(!Serial);
    
    pinMode(LED, OUTPUT);     // status LED

    // start modem Serial
    IridiumSerial.begin(19200);
    modem.setPowerProfile(IridiumSBD::USB_POWER_PROFILE);

    while(!SD.begin(SD_CS)){
        Serial.println(F("SD card error")); 
        delay(1000);
    }
    Serial.println(F("SD card detected."));

    logFile = SD.open("IRIDLOG.txt", FILE_WRITE);

    startModem();
    firmwareVersion();
    // testSignalQuality();
    // delay(3000);
    while(signalQuality < 1){       // wait to get signal
        testSignalQuality();
        delay(3000);
    }
    digitalWrite(LED, HIGH);    //turn on the LED when signal is good

    uint8_t id = 1;

    for (uint8_t seq = 1; seq <= totSeq; seq++){

        // open file
        snprintf(fileName, 13, "%02d%03d%03d.bin", id, seq, totSeq);
        file = SD.open(fileName, FILE_READ);
        file.read(toSend, HEADER_LENGTH + PAYLOAD_LENGTH);
        file.close();

        if(sendBinary()){  // send the contents of the buffer
            success[seq-1] = true;
        } else {
            success[seq-1] = false;     // send unsuccessful
        }    
    }

    Serial.println(F("finished sending messages"));
    for(int i = 0; i < sizeof(success); i++){
      Serial.print(success[i]); Serial.print(F(" "));
    }
    Serial.println();
    logFile.write("\n");

    logFile.close();

    digitalWrite(LED, LOW);     // turn off LED
    delay(1000);

}

void loop(){
  // blink the LED once everything is done
  digitalWrite(LED, HIGH);
  delay(1000);
  digitalWrite(LED, LOW);
  delay(1000);
  
}

/**
 * start up the Irid modem
 */
int startModem(){
    Serial.println(F("starting modem..."));
    logFile.write("starting modem...\n");
    err = modem.begin();
    if (err != ISBD_SUCCESS){
        Serial.print(F("begin failed: error ")); Serial.println(err);
        if(err == ISBD_NO_MODEM_DETECTED){
            Serial.println(F("no modem detected -- check wiring"));
        }
        return err;
    }
    Serial.println(F("modem started!"));
    logFile.write("modem started!\n");
    return 1;
}


/**
 * get firmware version of Irid modem
 */
int firmwareVersion(){
    char version[12];
    Serial.println(F("checking firmware version..."));
    err = modem.getFirmwareVersion(version, sizeof(version));
    if(err != ISBD_SUCCESS){
        Serial.print(F("firmware version failed: error ")); Serial.println(err);
        return err;
    }
    Serial.print(F("firmware version is ")); Serial.println(version);
    logFile.write("firmware version "); logFile.write(version); logFile.write("\n");
    return 1;
}


/**
 * test signal quality of modem connection to satellite network 
 * scale of 0 to 5
 */
int testSignalQuality(){
  Serial.println(F("checking signal quality..."));
    err = modem.getSignalQuality(signalQuality);
    if(err != ISBD_SUCCESS){
        Serial.print(F("signal quality failed: error ")); Serial.println(err);
        return err;
    }
    Serial.print(F("on a scale of 0 to 5, signal quality is currently ")); Serial.println(signalQuality);
    logFile.write("signal quality ");  logFile.write(signalQuality); logFile.write("\n");
    return 1;
}



/** 
 * send contents of toSend buffer (assumed to be a packet containing image data)
 */
int sendBinary(){
    Serial.println(F("trying to send the message..."));
    logFile.write("trying to send message: "); logFile.write(toSend, HEADER_LENGTH + PAYLOAD_LENGTH); logFile.write("\n");
    Serial.println(sizeof(toSend));
    for(int i = 0; i < sizeof(toSend); i++){
      Serial.print(toSend[i]); Serial.print(F(" "));
    }
    Serial.println();
    err = modem.sendSBDBinary(toSend, HEADER_LENGTH + PAYLOAD_LENGTH);
    if(err != ISBD_SUCCESS){
        err = modem.sendSBDBinary(toSend, HEADER_LENGTH + PAYLOAD_LENGTH);      // try again to send
    }
    if(err != ISBD_SUCCESS){
        Serial.print(F("send failed: error ")); Serial.println(err);
        if(err == ISBD_SENDRECEIVE_TIMEOUT){
            Serial.println(F("try again with a better view of the sky"));
        }
        logFile.write(err); logFile.write("\n");
        return err;
    }
    Serial.println(F("send worked!"));
    logFile.write("successful send!\n");
    return 1;
}
