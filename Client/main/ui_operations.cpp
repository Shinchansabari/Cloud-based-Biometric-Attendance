#include "ui_operations.h"
#include "lcd_operations.h"
#include "fingerprint_operations.h"
#include "cloud_operations.h"

String daysInaWeek[7] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
// Define lecture periods time ranges (24-hour format)
const int hourStartTimes[] = {830, 925, 1030, 1125, 1310, 1405, 1500, 1555}; // Example start times (HHMM format)
const int hourEndTimes[] = {925, 1020, 1125, 1220, 1405, 1500, 1555, 1730};   // Example end times (HHMM format)
String* todaysTimetable = new String[8];
//char hourNames[] = {"hour_1", "hour_2", "hour_3", "hour_4", "hour_5", "hour_6", "hour_7", "hour_8"}; // Period names
String  hourName;


// Define a global JSON document
DynamicJsonDocument attendance_ForStudents_Json(40960);  // Adjust the size as needed
DynamicJsonDocument attendance_ForStaff_Json(40960);  // Adjust the size as needed


RTC_DS3231 rtc;
DateTime now;
int year = 0;
int month = 0;
int day = 0;
int dayOfTheWeek = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;
int hour = 0;
String currentTime;
String currentDate;
String currentDay;
String serialData;
String roomNum;
String batchNum;
String batchDetails;


// Define keypad constants
const char ROWS = 4; // Four rows
const char COLS = 4; // Four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
char rowPins[ROWS] = {33,35,37,39}; // Row pins connected to Arduino
char colPins[COLS] = {41,43,45,47}; // Column pins connected to Arduino
// Define the keypad object
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

bool updated = true; 
int cursor = 1;
String page = "home";
int pageNum = 1;
char keyPressed = false;


void setupRTC() {
  // Initialize the RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  // Check if the RTC is running, otherwise set the time
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");

    // Set the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    // To set the time manually, uncomment the following line and adjust the parameters
    // rtc.adjust(DateTime(2024, 7, 15, 12, 0, 0)); // Year, Month, Day, Hour, Minute, Second
  }
  now = rtc.now();
}


void getTime() {
  now = rtc.now();
  year = now.year();
  month = now.month();
  day = now.day();
  dayOfTheWeek = now.dayOfTheWeek();
  hours = now.hour();
  minutes = now.minute();
  seconds = now.second();

  currentTime = String(hours) + ":" + String(minutes);
  currentDate = String(day) + "/" + String(month) + "/" + String(year);
  currentDay = daysInaWeek[dayOfTheWeek];
}

void getHour() {
  hour = 0;
  int calculationTime = (hours * 100) + minutes;
  // Determine current period based on time ranges
  for (int i = 0; i <= (sizeof(hourStartTimes) / sizeof(hourStartTimes[0])); i++) {
    if (calculationTime > hourStartTimes[i] && calculationTime <= hourEndTimes[i]) {
      //strcpy(hourName, hourNames[i]);
      hourName = todaysTimetable[i];
      hour = i+1;
      break; // Exit loop once current period is found
    }
    else {
      hourName = "Free time";
      hour = 0;
    }
  }
  Serial.println(calculationTime);
  Serial.println(hour);
  Serial.println(hourName);
}

// Function to append or modify data in the JSON
void updateAttendance(String batchName, String rollNum, String date, int hour, String target) {
  
  if (hour < 1 || hour > 8) {
    return;
  }
  String currentHour = todaysTimetable[hour-1];
  String nextHour = todaysTimetable[hour];


  if (target == "Students") {
    attendance_ForStudents_Json.clear();
    JsonObject batch = attendance_ForStudents_Json[batchName];
    if (!batch) {
      batch = attendance_ForStudents_Json.createNestedObject(batchName);
    }
    JsonObject student = batch[rollNum];
    if (!student) {
      student = batch.createNestedObject(rollNum);
    }
      
    JsonArray attendance = student[date].as<JsonArray>();
    if (!attendance) {
        attendance = student.createNestedArray(date);
    }
      
    attendance.add(currentHour + "/" + String(hour));
    if ((hour < 7) && (currentHour == nextHour)) {
      attendance.add(nextHour + "/" + String(hour+1));
    }
  }
  else if (target == "Staff") {
    attendance_ForStaff_Json.clear();
    JsonObject batch = attendance_ForStaff_Json[batchName];
    if (!batch) {
      batch = attendance_ForStaff_Json.createNestedObject(batchName);
    }
    JsonObject period = batch[currentHour];
    if (!period) {
        period = batch.createNestedObject(currentHour);
    }

    int numOfClasses;
    if ((hour < 7) && (todaysTimetable[hour-1] == todaysTimetable[hour])) {
      numOfClasses = 2;
    }
    else {
      numOfClasses = 1;
    }


    JsonArray students = period[date + "//" + String(numOfClasses)].as<JsonArray>();
    if (!students) {
        students = period.createNestedArray(date + "//" + String(numOfClasses));
    }

    if ((hour < 8) && (todaysTimetable[hour-1] == todaysTimetable[hour])) {
      students.add(rollNum + "/2");
    }
    else {
      students.add(rollNum + "/1");
    }
  }
  
}


void startupDisplay() {
  lcd.clear();
  displayTwoLineText("Hello", "Students");
}

void homePage() {
  page = "home";
  lcd.clear();
  displayTwoLineText("ATTENDANCE", "MENU", cursor);
  updated = true;
}

char getKeypad() {
  char key = keypad.getKey(); // Get the key pressed
  if (key) {
    return key;
  }
  return false;
}

void attendancePage() {
  displayTwoLineText("Ready to log", "attendance");
  delay(1000);
}


void addFingerprintPage() {
  enterFingerprintID();
  lcd.clear();
  displayTwoLineText("Processing");
}


void gotAttendancePage() {
  lcd.clear();
  displayTwoLineText(String(finger.fingerID), "Attendance taken");
  updated = true;
  
}

void menuPage() {
  if (pageNum == 1) {
    displayTwoLineText("Dowload", "Timetable", 2);
  }
  else if (pageNum == 2) {
    displayTwoLineText("Upload", "Attendance", 2);
  }
  else if (pageNum == 3) {
    displayTwoLineText("Add", "Fingerprint", 2);
  }
  else if (pageNum == 4) {
    displayTwoLineText("Upload", "Fingerprint", 2);
  }
  else if (pageNum == 5) {
    displayTwoLineText("Change Room", "", 1);
  }
  updated = true;
}


void UIflow() {
  lcd.clear();
  if (page == "home") {
    switch (keyPressed) {

      case 'A':
        if (cursor == 2) {
          cursor = 1;
          homePage();
          break;
        }  
        else if (cursor == 1) {
          homePage();
          break;
        }
      case 'B':
        if (cursor == 1) {
          cursor = 2;
          homePage();
          break;
        }
        else if (cursor == 2) {
          homePage();
          break;
        }

      case '#':
        if (cursor == 1) {
          page = "attendance";
          attendancePage();
          FPupdated = false;
          break;
        }
        else if (cursor == 2) {
          page = "menu";
          cursor = 1; 
          pageNum = 1;
          menuPage();
          break;
        }
      default :
        homePage();
        break;
    }
    updated = true;
    return;
  }

  else if (page == "menu") {

    if (pageNum == 1) {
      switch(keyPressed) {
        case 'A':
          cursor = 1;
          menuPage();
          break;
            
        case 'B':
          pageNum = 2;
          menuPage();
          break;
          
        case '#':
          lcd.setCursor(0,0);
          lcd.print("Loading...");
          fetchTimetable("Batch_1", currentDay);
          lcd.clear();
          displayTwoLineText("Timetable","Loaded!");
          delay(2000);
          menuPage();
          break;
        case '*':
          page = "home";
          cursor = 1;
          homePage();
          break;
        default :
          menuPage();
          break;
        
      }
      updated = true;
      return;
    }

    else if (pageNum == 2) {
      switch(keyPressed) {
        case '*':
          page = "home";
          pageNum = 1;
          cursor = 1;
          homePage();
          break;
        
        case 'A':
          pageNum = 1;
          menuPage();
          break;
          
        case 'B':
          pageNum = 3;
          menuPage();
          break;
          
        case '#':
          lcd.print("Uploading...");

          String todays_Attendance_ForStudents;
          String todays_Attendance_ForStaff;

          serializeJson(attendance_ForStudents_Json, todays_Attendance_ForStudents);
          serializeJson(attendance_ForStaff_Json, todays_Attendance_ForStaff);

          if ( (!uploadAttendanceToServer(todays_Attendance_ForStudents,"upload_Attendance_ForStudents")) || (!uploadAttendanceToServer(todays_Attendance_ForStaff,"upload_Attendance_ForStaff")) ){
            lcd.clear();
            displayTwoLineText("ERROR at","uploading");
            delay(3000);
            page = "menu";
            pageNum = 2;
            lcd.clear();
            menuPage();
          }
          else {
            displayTwoLineText("Att upload","Done");
          delay(3000);
          menuPage();
          }
          break;
        
        default :
          menuPage();
          break;
      }
      updated = true;
      return;
    }

    else if (pageNum == 3) {
      switch(keyPressed) {
        case 'A':
            pageNum = 2;
            menuPage();
            break;
          
        case 'B':
            pageNum = 4;
            menuPage();
            break;
        
        case '#':
            page = "addFingerprint";
            addFingerprintPage();
            if (page == "menu") {
              lcd.clear();
              menuPage();
              break;
            }
            Serial.println("FPupdated = false");
            FPupdated = false;
            break;

        case '*':
          page = "home";
          cursor = 1;
          homePage();
          break;
        default :
          menuPage();
          break;
      }
      updated = true;
      return;
    }

    else if (pageNum == 4) {
      switch(keyPressed) {
        case 'A':
            pageNum = 3;
            menuPage();
            break;
          
        case 'B':
            pageNum = 5;
            menuPage();
            break;
          
        case '*':
          page = "home";
          cursor = 1;
          homePage();
          break;
        default :
          menuPage();
          break;
      }
      updated = true;
      return;
    }
    else if (pageNum == 5) {
      switch(keyPressed) {
        case 'A':
          pageNum = 4;
          menuPage();
          break;
        case '#':
          enterRoomNum();
          fetchBatchDetails(roomNum);
          batchNum = batchDetails["Batch_Num"];
          fetchTimetable("Batch_"+batchNum,currentDay);
          break;
        case '*':
          cursor = 1;
          homePage();
          break;
      }
      updated = true;
      return;
    }
  }
  else if (page == "attendance") {
    switch(keyPressed) {
      case '*':
        page = "home";
        pageNum = 1;
        cursor = 1;
        homePage();
        break;
      default :
        attendancePage();
        FPupdated = false;
        break;
    }
    updated = true;
    return;
  }

  updated = true;
}


void fingerprintActions() {
  Serial.println("fingerprint actions");
  if (page == "attendance") {
    updated = false;
    FPupdated = true;
    lcd.clear();
    lcd.setCursor(0,0);
    getTime();
    getHour();
    displayStatusBar(String(hour), currentTime);
    lcd.setCursor(0,1);
    lcd.print("Place finger");

    keyPressed = getKeypad();
    Serial.println(keyPressed);
    switch (keyPressed) {
      case '*' :
        lcd.clear();
        pageNum = 3;
        page = "menu";
        menuPage();
        updated = false;
        FPupdated = true;
        return;
      default :
        ;
    }

    if (!getandverifyFingerprint()) {
      lcd.clear();
      displayTwoLineText("Try again");
      delay(500);
    }
    else {
      lcd.clear();
      displayTwoLineText(String(fingerprintID), "Attendance taken");
      updateAttendance("Batch_"+batchNum,String(fingerprintID), currentDate, hour, "Students");
      updateAttendance("Batch_"+batchNum,String(fingerprintID), currentDate, hour, "Staff");

      String todays_Attendance_ForStudents;
      String todays_Attendance_ForStaff;
      serializeJson(attendance_ForStudents_Json, todays_Attendance_ForStudents);
      serializeJson(attendance_ForStaff_Json, todays_Attendance_ForStaff);

      uploadAttendanceToServer(todays_Attendance_ForStudents,"upload_Attendance_ForStudents");
      uploadAttendanceToServer(todays_Attendance_ForStaff,"upload_Attendance_ForStaff");
      
      delay(2000);
        
      return;
    }
  }

  else if (page == "addFingerprint") {
    
    updated = false;
    FPupdated = true;

    while(true) {

      keyPressed = getKeypad();
        if (keyPressed) {
          if (keyPressed == '*') {
            pageNum = 3;
            page = "menu";
            updated = false;
            return;
          }
        }

      
      if (!enrollFingerprint()) {
        lcd.clear();
        displayTwoLineText("Waiting for", "fingerprint");
        delay(500);
      }

      else {
        lcd.clear();
        displayTwoLineText("Fingerprint", "taken");
        page = "menu";
        return;

      }

    }
    
  }
}

