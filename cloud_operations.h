#ifndef CLOUD_OPERATIONS_H
#define CLOUD_OPERATIONS_H

#include "fingerprint_operations.h"
#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

// Undefine the CLOSED macro from the Keypad library
#undef CLOSED

#include <WiFiEsp.h>
#include <SD.h>
#include <SPI.h>
#include <time.h>
#include "arduino_base64.hpp"
#include <Base64.h>

//#define espSerial Serial3


extern const char* ssid; 
extern const char* password;
extern const char* serverName;
extern const int serverPort;

extern const char* ntpServer;
extern const int timeZoneOffset; // Offset in seconds (3600 = 1 hour)

void connectWiFi() ;
String getIPAddress();
//void setupSDcard();
//void uploadJsonFile(String fileName);
//void uploadFolder(String folderName); 
String convertToBase64(Adafruit_Fingerprint_Packet * Packet, size_t packetSize);
void initializeTimetableArray();
bool fetchTimetable(String batch, String day);
bool fetchBatchDetails(String roomNum);
bool uploadAttendanceToServer(String jsonPayload, String endpoint);
bool uploadFingerprintToServer(String field, Adafruit_Fingerprint_Packet * Packet, size_t packetSize, String header);
bool sendCommand(String command, int timeout, String expectedResponse);  
void getNTPTime(const char* ntpServer, int timeZoneOffset, unsigned long epoch, unsigned long hour, unsigned long minute, unsigned long second);
void newTimeFunc();


#endif