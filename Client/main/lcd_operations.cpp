#include "lcd_operations.h"
#include "cloud_operations.h"
#include "fingerprint_operations.h"
#include "ui_operations.h"

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setupDisplay() {

  lcd.begin(16,2);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  displayTwoLineText("Connecting", "to Wifi");
  delay(100);

}


void clearRow(int row) {
  lcd.setCursor(0, row-1); // Set the cursor to the beginning of the specified row
  for (int i = 0; i < 16; i++) { // Assumes a 16-character wide display
    lcd.print(" ");
  }
  lcd.setCursor(0, row-1); // Reset the cursor to the beginning of the row
}


void displayStatusBar(String left, String right) {
  int col = 16 - right.length(); // Calculate the starting column for right alignment
  if (col < 0) col = 0; // Ensure column is not negative
  clearRow(1);
  lcd.setCursor(0,0);
  lcd.print("Hour:" + left);
  lcd.setCursor(col, 0); // Set the cursor to the calculated column
  lcd.print(right);
}



void displayTwoLineText(String line1, String line2, int cursor) {
  lcd.clear();
  if (cursor == 0) {
    lcd.setCursor(0,0); // Adjust the y-coordinate as needed
    lcd.print(line1);
    lcd.setCursor(0,1); // Adjust the y-coordinate as needed
    lcd.print(line2);
  }
  
  else if (cursor == 1) {
    lcd.setCursor(0,0); // Adjust the y-coordinate as needed
    line1 = (line1 + "  <");
    lcd.print(line1);
    lcd.setCursor(0,1); // Adjust the y-coordinate as needed
    lcd.print(line2);
  }

  else if (cursor == 2) {
    lcd.setCursor(0,0); // Adjust the y-coordinate as needed
    lcd.print(line1);
    lcd.setCursor(0,1); // Adjust the y-coordinate as needed
    line2 = (line2 + "  <");
    lcd.print(line2);
  }
  
  lcd.display(); // Actually display all of the above
}





void enterFingerprintID() {
  String input = "";
  lcd.clear();
  lcd.print("Enter ID");
  while (input.length() <= 3) {
    if (updated == true) {
      keyPressed = getKeypad();
      if (keyPressed) {
        updated = false;

        if (keyPressed == '*') {
          page = "menu";
          return;
        }

        if ( (input.length() == 3) && (keyPressed == '#') ) {
          updated = true;
          break;
        }

        input = input + keyPressed;
      }
      if (updated == false) {
        lcd.clear();
        displayTwoLineText("Enter ID", input);
      }
    }
    updated = true;
  }
  fingerprintID = input.toInt();
}


void enterRoomNum() {
  String input = "";
  lcd.clear();
  lcd.print("Enter Room No.");
  while (input.length() <= 3) {
    if (updated == true) {
      keyPressed = getKeypad();
      if (keyPressed) {
        updated = false;

        if (keyPressed == '*') {
          page = "menu";
          return;
        }

        if (keyPressed == '#') {
          updated = true;
          lcd.clear();
          lcd.print("Wait...");
          break;
        }
        input = input + keyPressed;
      }
      if (updated == false) {
        lcd.clear();
        displayTwoLineText("Enter Room No.",input);
      }
    }
    updated = true;
  }
  roomNum = input;
}