#include "cloud_operations.h"
#include "fingerprint_operations.h"
#include "ui_operations.h"
#include "lcd_operations.h"


// chipselect = 4
// WiFi and server credentials 
const char* ssid = "HP_PAVI_ 722";
const char* password = "87654321";
const char* serverName = "192.168.137.223"; 
const int serverPort = 5500;

#define SERVER "your_server_address"  // Replace with your server address
#define PORT "5500"                     // Replace with your server port if different
#define UPLOAD_ENDPOINT "/upload_Fingerprint"     // Replace with your server's upload endpoint
#define espSerial Serial3


void connectWiFi() 
{
    bool connected = false;
    
    while (!connected) {
        connected = true;  // Assume success unless proven otherwise
        
        // Attempt to execute each command
        if (!sendCommand("AT", 1000, "OK")) {
            connected = false;
        }
        
        if (!sendCommand("AT+CWMODE=1", 1000, "OK")) {
          Serial.println("Wifi mode not set");
          connected = false;
        }
        
        if (!sendCommand("AT+CWJAP=\"" + String(ssid) + "\",\"" + String(password) + "\"", 20000, "OK")) {
          Serial.println("Wifi not set");
          connected = false;
        }
        
        // If any command fails, connected will be false and the loop will retry
        
        if (!connected) {
            Serial.println("WiFi connection failed, retrying...");
            // You can add a delay here to avoid flooding the espSerial with commands
            delay(5000);  // Wait 5 seconds before retrying
        }
    }
    
    Serial.println("WiFi connected successfully!");
}

String getIPAddress() {
  espSerial.println("AT+CIFSR");
  
  String response = "";
  while (espSerial.available()) {
    char c = espSerial.read();
    response += c;
  }
  
  // Extract the IP address from the response
  int start = response.indexOf("\"") + 1;
  int end = response.lastIndexOf("\"");
  String ipAddress = response.substring(start, end);
  
  Serial.print("IP Address: ");
  Serial.println(ipAddress);
  
  return ipAddress;
}

void initializeTimetableArray() {
  for (int i = 0; i < 7; i++) {
    todaysTimetable[i] = "Hour_" + String(i+1);
  }
}


String convertToBase64(Adafruit_Fingerprint_Packet * Packet, size_t packetSize) {
  /// Convert fingerprint packet to base64
  char encodedPacket[Base64.encodedLength(packetSize)];
  Base64.encode(encodedPacket, (char*)Packet, packetSize);
  String  encodedPacketStr = String(encodedPacket);
  if (!encodedPacketStr) {
    Serial.println("Base64 conversion failed");
    return "";
  }
  return encodedPacketStr;
}

bool fetchBatchDetails(String roomNum) {
    String url = "/fetch_Batch_Details"; // URL for the POST request
    String postData = "{\"roomNum\": \"" + roomNum + "\"}"; // JSON payload
    String httpRequest = "POST " + url + " HTTP/1.1\r\n";
    httpRequest += "Host: " + String(serverName) + "\r\n";
    httpRequest += "Connection: close\r\n";
    httpRequest += "Content-Type: application/json\r\n";
    httpRequest += "Content-Length: " + String(postData.length()) + "\r\n";
    httpRequest += "\r\n";
    httpRequest += postData; // Include the JSON data in the body

    // Start TCP connection
    if (!sendCommand("AT+CIPSTART=\"TCP\",\"" + String(serverName) + "\"," + String(serverPort), 10000, "CONNECT")) {
        Serial.println("Failed to connect to server");
        return false;
    }

    // Send HTTP POST request
    if (!sendCommand("AT+CIPSEND=" + String(httpRequest.length()), 5000, ">")) {
        Serial.println("Failed to initiate data send");
        return false;
    }
    if (!sendCommand(httpRequest, 5000, "SEND OK")) {
        Serial.println("Failed to send data");
        return false;
    }

    String response = "";
    int stamp = millis();
    while ((millis() - stamp) < 10000) {
      while (espSerial.available()) {
      char c = espSerial.read();
      response += c;
      }
    }
    // Parse and display the response
    Serial.println(response);


    // Parse the JSON response
  DynamicJsonDocument doc(102400); // Adjust size as needed

  int bodyStart = response.indexOf("+IPD");
  int bodyEnd = response.indexOf("CLOSED");

  if (bodyStart != -1) {
    String responseBody = response.substring(bodyStart+9, bodyEnd);

    int bodyStart = responseBody.indexOf("+IPD");

    Serial.println(responseBody);

    if (bodyStart != -1) {
      String dataBody = responseBody.substring(bodyStart+9, bodyEnd);
      Serial.println(dataBody);

      DeserializationError error = deserializeJson(doc, dataBody);
      if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        lcd.clear();
        displayTwoLineText("Failed to", "fetch timetable");
        delay(2000);
        return false;
      }

      if (doc.containsKey("data")) {
        JsonObject data = doc["data"].as<JsonObject>();
        batchNum = data["Batch_Num"].as<String>();
        String dataString;
        serializeJson(data, dataString); // Convert JsonObject to String
        batchDetails = dataString;
      } else {
        Serial.println("Key 'data' not found in response.");
        lcd.clear();
        displayTwoLineText("Failed to", "fetch timetable");
        delay(2000);
        return false;
      }
    } else {
        Serial.println("Invalid response format.");
        lcd.clear();
        displayTwoLineText("Failed to", "fetch timetable");
        delay(2000);
        return false;
    }
    
  } else {
    Serial.println("Invalid response format.");
    lcd.clear();
    displayTwoLineText("Failed to", "fetch timetable");
    delay(2000);
    return false;
  }

    return true;
}




bool fetchTimetable(String batch, String day) {
  // Construct the JSON payload
  String jsonPayload = "{\"batch\":\"" + batch + "\",\"day\":\"" + day + "\"}";
  
  // Create the TCP connection command
  String url = "AT+CIPSTART=\"TCP\",\"" + String(serverName) + "\"," + String(serverPort);
  
  // Check if the connection is established
  if (!sendCommand(url, 10000, "CONNECT")) {
    Serial.println("Failed to connect to server");
    lcd.clear();
    displayTwoLineText("Failed to", "fetch timetable");
    delay(2000);
    return false;
  }
  
  // Construct the HTTP request
  String httpRequest = "POST /fetch_Timetable HTTP/1.1\r\n";
  httpRequest += "Host: " + String(serverName) + "\r\n";
  httpRequest += "Content-Type: application/json\r\n";
  httpRequest += "Content-Length: " + String(jsonPayload.length()) + "\r\n";
  httpRequest += "\r\n";
  httpRequest += jsonPayload;
  
  //Send the length of the HTTP request to the ESP8266
  if (!sendCommand("AT+CIPSEND=" + String(httpRequest.length()),  5000 , ">")) {
    Serial.println("Failed to initiate data recieve");
    lcd.clear();
    displayTwoLineText("Failed to", "fetch timetable");
    delay(2000);
    return false;
  }
  
  // Send the actual POST request data
  if (!sendCommand(httpRequest, 5000, "SEND OK")) {
    Serial.println("Failed to send data");
    lcd.clear();
    displayTwoLineText("Failed to", "fetch timetable");
    delay(2000);
    return false;
  }

  // Read the response from the server
  String response = "";
  unsigned long startTime = millis();
  while (millis() - startTime < 10000) { // Timeout for response
    while (espSerial.available()) {
      response += char(espSerial.read());
    }
  }

  // Parse the JSON response
  DynamicJsonDocument doc(102400); // Adjust size as needed

  int bodyStart = response.indexOf("+IPD");
  int bodyEnd = response.indexOf("CLOSED");

  Serial.println(response);

  if (bodyStart != -1) {
    String responseBody = response.substring(bodyStart+9, bodyEnd);

    int bodyStart = responseBody.indexOf("+IPD");

    Serial.println(responseBody);

    if (bodyStart != -1) {
      String dataBody = responseBody.substring(bodyStart+9, bodyEnd);
      Serial.println(dataBody);

      DeserializationError error = deserializeJson(doc, dataBody);
      if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        lcd.clear();
        displayTwoLineText("Failed to", "fetch timetable");
        delay(2000);
        return false;
      }

      if (doc.containsKey("data")) {
        JsonArray array = doc["data"];
        int dataArraySize = array.size(); // Update the size of the array
        
        for (int i = 0; i < dataArraySize && i <= 7; i++) {
          todaysTimetable[i] = array[i].as<String>();
        }
      } else {
        Serial.println("Key 'data' not found in response.");
        lcd.clear();
        displayTwoLineText("Failed to", "fetch timetable");
        delay(2000);
        return false;
      }
    } else {
        Serial.println("Invalid response format.");
        lcd.clear();
        displayTwoLineText("Failed to", "fetch timetable");
        delay(2000);
        return false;
    }
    
  } else {
    Serial.println("Invalid response format.");
    lcd.clear();
    displayTwoLineText("Failed to", "fetch timetable");
    delay(2000);
    return false;
  }
  Serial.println("Timetable downloaded");
  return true;
}



bool uploadAttendanceToServer(String jsonPayload, String endpoint) {
  // Create the TCP connection command
  String url = "AT+CIPSTART=\"TCP\",\"" + String(serverName) + "\"," + String(serverPort);
  
  // Check if the connection is established
  if (!sendCommand(url, 10000, "CONNECT")) {
    Serial.println("Failed to connect to server");
    lcd.clear();
    displayTwoLineText("Failed to", "upload attendance");
    delay(2000);
    return false;
  }
  // Construct the POST request
  String postData = "POST /" + endpoint + " HTTP/1.1\r\n";
  Serial.println(postData);
  
  
  postData += "Host: " + String(serverName) + "\r\n";
  postData += "Content-Type: application/json\r\n";
  postData += "Content-Length: " + String(jsonPayload.length()) + "\r\n";
  postData += "\r\n";
  postData += jsonPayload + "\r\n";

  // Send the length of the data
  if (!sendCommand("AT+CIPSEND=" + String(postData.length()), 1000, ">")) {
    Serial.println("Failed to initiate data send");
    lcd.clear();
    displayTwoLineText("Failed to", "upload attendance");
    delay(2000);
    return false;
  }
  delay(1000);

  // Send the actual POST request data
  if (!sendCommand(postData, 5000, "SEND OK")) {
    Serial.println("Failed to send data");
    lcd.clear();
    displayTwoLineText("Failed to", "upload attendance");
    delay(2000);
    return false;
  }

  // Close the TCP connection
  if (!sendCommand("AT+CIPCLOSE", 1000, "CLOSED")) {
    Serial.println("Failed to close the connection");
    lcd.clear();
    displayTwoLineText("Failed to", "upload attendance");
    delay(2000);
    return false;
  }

  Serial.println("Data uploaded successfully");
  return true;
}





bool uploadFingerprintToServer(String field, Adafruit_Fingerprint_Packet * Packet, size_t packetSize, String header) {

  String encodedPacketStr = convertToBase64(Packet, 512);
  if (!encodedPacketStr) {
    Serial.println("Cannot upload FP");
    return false;
  }

  String url = "AT+CIPSTART=\"TCP\",\"" + String(serverName) + "\"," + String(serverPort);

  if (!sendCommand(url, 10000, "CONNECT")) {
    Serial.println("1");
    return false;
  } // Connect to server

  String postData = "POST /upload_Fingerprint HTTP/1.1\r\n";
  postData += "Host: " + String(serverName) + "\r\n";
  postData += "Content-Type: application/json\r\n";
  postData += "Content-Length: " + String((field.length() + encodedPacketStr.length() + 20)) + "\r\n"; // Adjust content length
  postData += "\r\n";
  postData += "{\"" + header + "\": {\"" + field + "\": \"" + encodedPacketStr + "\"}}\r\n";

  if (!sendCommand("AT+CIPSEND=" + String(postData.length()), 1000, ">")) {
    Serial.println("2");
    return false;
  } // Send length of data
  if (!sendCommand(postData, 5000, "SEND OK")) {
    Serial.println("3");
    return false;
  } // Send actual data

  if (!sendCommand("AT+CIPCLOSE", 1000, "CLOSED")) {
    Serial.println("4");
    return false;
  } // Close the connection
  Serial.println("Test finish");
  return true;
}



bool sendCommand(String command, int timeout, String expectedResponse) {
  String response = "";
  espSerial.println(command);
  long int start = millis();
  while ((start + timeout) > millis()) {
    while (espSerial.available()) {
      char c = espSerial.read();
      response += c;
    }
    
    if (response.indexOf(expectedResponse) != -1) {
      return true;
    }
    
  }
  Serial.println(response);
  return false;
}
