#ifndef FINGERPRINT_OPERATIONS_H 

  #define FINGERPRINT_OPERATIONS_H 

  #include <Arduino.h>
  #include <SoftwareSerial.h>
  #include <Adafruit_Fingerprint.h> // Include necessary libraries
  #include <SD.h>
  #include <SPI.h>
  #include <ArduinoJson.h>


  extern SoftwareSerial mySerial; // RX, TX for AS608 fingerprint sensor 
  extern Adafruit_Fingerprint finger; 
  extern int fingerprintID;
  extern uint8_t p;
  //extern Adafruit_Fingerprint_Packet * packet;
  extern Adafruit_Fingerprint_Packet * packet;
  extern String subjectCode;
  extern char serialBuffer[];
  extern bool FPupdated;

  void setupFingerprintSensor(); 

  bool enrollFingerprint(); 

  Adafruit_Fingerprint_Packet* createFingerprintTemplate(Adafruit_Fingerprint_Packet * packet);

  bool saveFingerprintTemplate();

  bool getFingerprintTemplate();

  String packetToString(Adafruit_Fingerprint_Packet * packet);

  void deleteAllFingerprints(); 

  bool getandverifyFingerprint(); 

  //bool verifyLog();



#endif 