/**
 * remove lines from the head of a file, excluding header
 * for removing lines from hourly data file in datalogger code
 * 
 * Author: Rachel Pagdin
 * August 2, 2024
 */

#include <SD.h> 
#include <SPI.h>

File file;
const int chipSelect = 4;       // SD card select

void setup(){
    delay(50);
    Serial.begin(115200);
    Serial.flush();
    delay(100);

    SD.begin(chipSelect);
    delay(100);
}

void loop(){
    // open the file
    file = SD.open("/test.csv", FILE_READ);
    int size = file.size();

    // read the contents into a buffer
    char contents[size];
    file.read(contents, sizeof(contents));
    char *end = strrchr(contents, '\n');
    contents[end-contents+1] = '\0';
    Serial.println(contents);

    // isolate the header and save in buffer
    char *header = strtok(contents, "\n");       // get first line
    Serial.println(header);

    // iterate through 24 lines to find the beginning of the data you want to keep
    int ctr = 0;
    char *temp;
    temp = strchr(contents, '\n');      // look for the first new line - this gets rid of the header
    temp = strchr(temp, '\n');
    while((ctr < 24) && (temp != NULL)){
        ctr++;
        temp = strchr(temp + 1, '\n');
    }

    int newSize = end - temp;
    char keep[newSize+1];       // extra space for null char
    strncpy(keep, temp+1, newSize);
    keep[newSize] = '\0';       // terminate with null character
    Serial.println(keep);

    Serial.println(sizeof(header));
    Serial.println(sizeof(keep));

    file.close();

    delay(10000);
}

