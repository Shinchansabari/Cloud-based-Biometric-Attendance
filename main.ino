#include "cloud_operations.h"
#include "fingerprint_operations.h" 
#include "lcd_operations.h"
#include "ui_operations.h"
#define espSerial Serial3

unsigned long lastTimeUpdate = 0;
int timeInterval = 30;


void setup() { 
  
  Serial.begin(9600); 
  while (!Serial); 
  espSerial.begin(115200);
  delay(2000);
  Wire.begin();
  setupDisplay();
  //setupSDcard() ;
  connectWiFi();
  setupFingerprintSensor(); 
  delay(1000);
  initializeTimetableArray();
  setupRTC();
  startupDisplay();
  delay(2000);
  getTime();
  getHour();
  enterRoomNum();
  fetchBatchDetails(roomNum);
  //batchNum = batchDetails["Batch_Num"];
  fetchTimetable("Batch_"+batchNum,currentDay);
  lcd.clear();
  homePage();
} 

void loop() { 
  
  if (((millis() - lastTimeUpdate) /1000) > timeInterval) {
    if (page == "attendance") {
      getTime();
      getHour();
      clearRow(1);
      displayStatusBar(String(hour), currentTime);
      lastTimeUpdate = millis();
    }
  }

  if (updated == true) {
    keyPressed = getKeypad();
    if (keyPressed) {
      updated = false;
      Serial.println(keyPressed);
    }
  }
  
  if (FPupdated == false) {
    fingerprintActions();
  }
  

  if (updated == false) {
    UIflow();
  }
  
  delay(200);  

} 


