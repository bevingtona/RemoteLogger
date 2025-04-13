#include <SDI12.h>

// Pin definitions
const byte dataPin_OBS = 12;        // The pin of the SDI-12 data bus for Observator
const byte dataPin_CTD = 11;        // The pin of the SDI-12 data bus for CTD

// Define the SDI-12 bus objects
SDI12 mySDI12_OBS(dataPin_OBS);
SDI12 mySDI12_CTD(dataPin_CTD);

// Sensor addresses
#define SENSOR_ADDRESS_OBS 0
#define SENSOR_ADDRESS_CTD 0

void setup() {
  // Set pin modes
  pinMode(dataPin_CTD, INPUT); 
  pinMode(dataPin_OBS, INPUT); 
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Begin serial communication
  Serial.begin(9600);
  
  // Initialize SDI-12 buses
  // mySDI12_OBS.begin();
  // mySDI12_CTD.begin();
  
  Serial.println("Dual SDI-12 Sensor System Initialized");
}

void loop() {
  // Indicate measurement start with LED
  blinkLED(1, 200);
  
  // Sample CTD sensor
  Serial.println("=== Hydros CTD Measurement ===");
  String ctdData = sample_hydros_M();
  Serial.println("CTD Data: " + ctdData);
  delay(1000); // Brief pause between sensors
  
  // Wipe Observator sensor
  Serial.println("=== Observator Wipe ===");
  String wipeResult = sample_observator_M_wipe();
  Serial.println("Wipe Result: " + wipeResult);
  delay(1000); // Brief pause between operations
  
  // Get statistical data from Observator
  Serial.println("=== Observator Statistical Measurement ===");
  String obsData = sample_observator_M6_statistical();
  Serial.println("Observator Data: " + obsData);
  
  // Long delay between full measurement cycles
  Serial.println("Waiting for next measurement cycle...");
  blinkLED(3, 100);
  delay(10000);
}

// Function to blink LED
void blinkLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_BUILTIN, LOW);
    delay(delayMs);
  }
}

String sample_observator_M_wipe() {
  // Ensure both SDI12 objects are properly initialized
  mySDI12_OBS.begin();
  
  // Clear any previous data
  mySDI12_OBS.clearBuffer();
  
  // Create the command
  String myCommand = String(SENSOR_ADDRESS_OBS) + "M1!";
  
  // Send the wipe command
  mySDI12_OBS.sendCommand(myCommand);
  
  // Wait for wipe operation to complete
  delay(16000);
  
  // Clear buffer again
  mySDI12_OBS.clearBuffer();
  
  mySDI12_OBS.end();
  
  return "wiped";
}

String sample_observator_M6_statistical() {
  // Ensure both SDI12 objects are properly initialized
  mySDI12_OBS.begin();
  
  // Clear any previous data
  mySDI12_OBS.clearBuffer();
  
  String sdiResponse = "";  // Local response string
  
  // Create and send the measurement command
  String myCommand = String(SENSOR_ADDRESS_OBS) + "M6!";
  mySDI12_OBS.sendCommand(myCommand);
  delay(30);  // wait for initial response
  
  // Collect response to M6 command
  while (mySDI12_OBS.available()) {
    char c = mySDI12_OBS.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);
    }
  }
  
  Serial.println("M6 command response: " + sdiResponse);
  
  // Clear buffer after reading response
  mySDI12_OBS.clearBuffer();
  
  // Wait for measurement to complete (this is a long measurement)
  Serial.println("Waiting for statistical measurement to complete...");
  delay(70000);
  
  // Clear response string for data
  sdiResponse = "";
  
  // Request data
  myCommand = String(SENSOR_ADDRESS_OBS) + "D1!";
  mySDI12_OBS.sendCommand(myCommand);
  delay(30);
  
  // Collect data response
  while (mySDI12_OBS.available()) {
    char c = mySDI12_OBS.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);
    }
  }
  
  // Process response if valid
  if (sdiResponse.length() > 2) {
    // Remove address and first separator
    sdiResponse = sdiResponse.substring(2);
    
    // Replace + with commas for CSV format
    for (int i = 0; i < sdiResponse.length(); i++) {
      if (sdiResponse.charAt(i) == '+') {
        sdiResponse.setCharAt(i, ',');
      }
    }
  } else {
    sdiResponse = "Error: No valid data received";
  }
  
  // Final buffer clear
  mySDI12_OBS.clearBuffer();
  
  mySDI12_OBS.end();
  
  return sdiResponse;
}

String sample_hydros_M() {
  // Ensure both SDI12 objects are properly initialized
  mySDI12_CTD.begin();
  
  // Clear any previous data
  mySDI12_CTD.clearBuffer();
  
  String sdiResponse = "";  // Local response string
  
  // Create and send the measurement command
  String myCommand = String(SENSOR_ADDRESS_CTD) + "M!";
  mySDI12_CTD.sendCommand(myCommand);
  delay(30);
  
  // Collect response to M command
  while (mySDI12_CTD.available()) {
    char c = mySDI12_CTD.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);
    }
  }
  
  Serial.println("M command response: " + sdiResponse);
  
  // Clear buffer after reading response
  mySDI12_CTD.clearBuffer();
  
  // Wait for measurement to complete
  delay(2000);
  
  // Clear response string for data
  sdiResponse = "";
  
  // Request data
  myCommand = String(SENSOR_ADDRESS_CTD) + "D0!";
  mySDI12_CTD.sendCommand(myCommand);
  delay(30);
  
  // Collect data response
  while (mySDI12_CTD.available()) {
    char c = mySDI12_CTD.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(10);
    }
  }
  
  // Process response if valid
  if (sdiResponse.length() > 3) {
    // Remove address and first separators
    sdiResponse = sdiResponse.substring(3);
    
    // Replace + with commas for CSV format
    for (int i = 0; i < sdiResponse.length(); i++) {
      if (sdiResponse.charAt(i) == '+') {
        sdiResponse.setCharAt(i, ',');
      }
    }
  } else {
    sdiResponse = "Error: No valid data received";
  }
  
  // Final buffer clear
  mySDI12_CTD.clearBuffer();
 
 mySDI12_CTD.end();
   
  return sdiResponse;
}