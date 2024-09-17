#ifndef DISPLAY_OPERATIONS_H 
#define DISPLAY_OPERATIONS_H 

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

extern LiquidCrystal_I2C lcd;

// Function prototypes
void setupDisplay();
void clearRow(int row);
void displayStatusBar(String left, String right = "");
void displayTwoLineText(String line1, String line2 = "", int cursor = 0);
void enterFingerprintID();
void enterRoomNum();



#endif 