
#include "fingerprint_operations.h"
#include "lcd_operations.h"
#include "ui_operations.h"

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);
int fingerprintID = 0;
uint8_t p = 0;
char serialBuffer[50];
Adafruit_Fingerprint_Packet * packet;
bool FPupdated = true;
const size_t bufferSize = 1024;

void setupFingerprintSensor() {
  // Initialization code for fingerprint sensor
  // Initialize fingerprint sensor
  Serial1.begin(57600);
  while (!Serial1); // Wait for Serial to initialize (Arduino Leonardo/Micro)
  delay(1000);

  finger.begin(57600);
  delay(1000);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor ");
    while (1);
  }
}


bool enrollFingerprint() {
  Serial.println("Place your finger");

  /*
  if (verifyLog()) {
    if (Serial.available() > 0) {
        serialData = Serial.readStringUntil('\n');
    }
    lcd.clear();
    displayTwoLineText(serialData);
    delay(1000);
    return false;
  }
  */

  // Implementation for enrolling fingerprint
  p = finger.getImage();

  switch (p) {

    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;

    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return false;

    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return false;

    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return false;

    default:
      Serial.println("Unknown error");
      return false;
  }

  // OK success!

  p = finger.image2Tz(1);

  switch (p) {

    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;

    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return false;

    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return false;

    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return false;

    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return false;

    default:
      Serial.println("Unknown error");
      return false;
  }

  Serial.print("Remove finger");
  lcd.clear();
  displayTwoLineText("Remove", "finger");
  p = 0;

  while (p != FINGERPRINT_NOFINGER) {
    delay(200);
    p = finger.getImage();
  }

  Serial.println("Keep your finger again...");
  lcd.clear();
  displayTwoLineText("Place finger", "again");
  p = finger.getImage();
  // Capture the image again
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      continue;
    }
    if (p != FINGERPRINT_OK) {
      Serial.println("Error capturing image");
      return false;
    }
  }

  Serial.println("Image taken again");

  // Convert the second image
  p = finger.image2Tz(2);
  switch (p) {

    case FINGERPRINT_OK:
      Serial.println("Second image converted");
      finger.fingerID = fingerprintID;
      break;

    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return false;

    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return false;

    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return false;

    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return false;

    default:
      Serial.println("Unknown error");
      return false;
  }

  // Create a model from the captured images
  p = finger.createModel();

  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } 
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return false;
  } 
  else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return false;
  } 
  else {
    Serial.println("Unknown error");
    return false;
  }

  // Store the model in the database
  p = finger.storeModel(fingerprintID);

  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    lcd.clear();
    displayTwoLineText("Fingerprint", "stored");
    delay(2000);
  }  

  return true;
}


Adafruit_Fingerprint_Packet* createFingerprintTemplate(Adafruit_Fingerprint_Packet * packet) {

  p = finger.getModel();
  
  p = finger.getStructuredPacket(packet, DEFAULTTIMEOUT);

  if (p != FINGERPRINT_OK) {
    Serial.println("Couldn't get template");
    return nullptr;
  }
  return packet;
}


bool saveFingerprintTemplate() {

  if (p == FINGERPRINT_OK) {

    if (createFingerprintTemplate(packet) == nullptr) {
        Serial.println("Template error");
        return false;
    }

    // Load the fingerprint template from the sensor's memory
    if (finger.loadModel(fingerprintID) != FINGERPRINT_OK) {
      Serial.println("Failed to load fingerprint model.");
      return false;
    }

    /*
    // Save fingerprint ID to SD card as CSV
    File myFile = SD.open("Fingerprint_Templates.csv", FILE_WRITE);

    // Move the file pointer to the end of the file
    myFile.seek(myFile.size());

    if (myFile) {

      myFile.print(fingerprintID);
      myFile.print(",");
      myFile.write(createFingerprintTemplate(packet));
      myFile.println();
      myFile.close();
      Serial.println("Log data saved to SD card.");
    } 
    else {
      Serial.println("Error opening file.");
      myFile.close();
      return false;
    }
    
    File myFile1 = SD.open("Log_Data.csv", FILE_READ);

    if (myFile1) {
      myFile.print(fingerprintID);
      myFile.println();
    }
    myFile1.close();
  }
  */
  }
}


// Function to convert Adafruit_Fingerprint_Packet to String
String packetToString(Adafruit_Fingerprint_Packet * packet) {
  String result = "";
  for (int i = 0; i < packet->length; i++) {
    result += String(packet->data[i], HEX);
    result += " ";
  }
  return result;
}


bool getFingerprintTemplate() {
  // Open the file for reading the binary template
  /*
  File myFile = SD.open(" Finger_Templates.csv", FILE_READ);
  if (!myFile) {
    Serial.println("Failed to open template file on SD card.");
    myFile.close();
    return false;
  }
  while (myFile.available()) {
    String line = myFile.readStringUntil("\n");
    line.trim();

    int commaIndex = line.indexOf(',');
    if (commaIndex != -1) {
      String fingerprintID = line.substring(0, commaIndex-1); // Extract the header
      String fingerprintTemplate = line.substring(commaIndex + 1); // Extract the value part after the comma
    }
    //Adafruit_Fingerprint_Packet* fingerprintTemplate = ;
  }
  

  // Read the binary template data from the file
  uint8_t fingerTemplate[512];
  if (myFile.read(fingerTemplate, sizeof(fingerTemplate)) != sizeof(fingerTemplate)) {
    Serial.println("Error reading template data from file.");
    myFile.close();
    return false;
  }
  
  myFile.close();
  */

  uint8_t fingerTemplate[512];
  // Create a packet with the data
  const Adafruit_Fingerprint_Packet packet(FINGERPRINT_ENDDATAPACKET, fingerTemplate, sizeof(fingerTemplate));

  // Send the packet to the sensor

  finger.writeStructuredPacket(packet);
  
  
  uint16_t p = finger.storeModel(finger.fingerID);

  if (p == FINGERPRINT_OK) {
    Serial.println("Template uploaded successfully.");
    return true;
  } else {
    Serial.println("Error uploading template to sensor.");
    return false;
  }
}


void deleteAllFingerprints() {

  // Implementation for deleting all fingerprints
}


bool getandverifyFingerprint() {
  p = finger.getImage();
  while(p != FINGERPRINT_OK) {
    // Implementation for identifying fingerprint
    p = finger.getImage();
    //Serial.print("Waiting for fingerprint");

    keyPressed = getKeypad();
    if (keyPressed) {
      if (keyPressed == '*') {
        return false;
      }
    }

  }
  Serial.print("Image taken");
  p = finger.image2Tz();

  if (p != FINGERPRINT_OK) {
    Serial.print("Image conversion failed");
    return false;  // Image conversion failed
  }

  p = finger.fingerFastSearch();

  if (p != FINGERPRINT_OK) {
    Serial.println("No match");
    return false;  // No match found
  }
  fingerprintID = finger.fingerID;
  // Return the found fingerprint ID
  return true;
}

