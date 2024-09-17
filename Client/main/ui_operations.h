#ifndef UI_OPERATIONS_H
#define UI_OPERATIONS_H


#include <Wire.h>
#include <Keypad.h>
#include <RTClib.h>

extern RTC_DS3231 rtc;
extern Keypad keypad;
extern char keyPressed;
extern bool updated;
extern int cursor; 
extern String page;
extern int pageNum; 
extern DateTime now;
extern int year;
extern int month;
extern int day;
extern int dayOfTheWeek;
extern int hours;
extern int minutes;
extern int seconds;
extern String currentTime;
extern String currentDate;
extern String currentDay;
extern int hour;
extern String daysInaWeek[7];
extern String* todaysTimetable; // Pointer to the dynamic array of Strings
extern String hourName;
extern String serialData;
extern String roomNum;
extern String batchNum;

extern String batchDetails;



void startupDisplay();
void setupRTC();
char getKeypad();
void updateAttendance(String batchName, String rollNum, String date, int hour, String target);
void homePage();
void attendancePage();
void gotAttendancePage();
void menuPage();
void UIflow();
void getTime();
void getHour();
void fingerprintActions();

#endif