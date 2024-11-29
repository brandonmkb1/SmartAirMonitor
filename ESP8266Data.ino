#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// WiFi and ThingSpeak settings
const char* ssid = "Whatdadawgdoin";          // Your WiFi SSID
const char* password = "MiloPogo";  // Your WiFi Password
const char* server = "api.thingspeak.com";
const String apiKey = "QB84T8YM0FQJ5GR8";     // ThingSpeak API key

WiFiClient client;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi.");
  
  delay(15000); // Add a 15-second delay after WiFi connection
  
}


void loop() {
  static unsigned long lastUpdateTime = 0;
  const unsigned long updateInterval = 15000; // 15 seconds

  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    data.trim();  // Remove any whitespace

    // Check if the data contains exactly 3 commas (4 fields)
    int commaCount = 0;
    for (int i = 0; i < data.length(); i++) {
      if (data.charAt(i) == ',') {
        commaCount++;
      }
    }

    if (commaCount == 3 && data.length() > 0) {
      Serial.println("Received valid data from Arduino: " + data);
      
      // Check if enough time has passed since the last update
      if (millis() - lastUpdateTime >= updateInterval) {
        sendToThingSpeak(data);
        lastUpdateTime = millis();
      } else {
        Serial.println("Skipping update, too soon since last update");
      }
    } else {
      Serial.println("Invalid data received: " + data);
    }
  }
}

void sendToThingSpeak(String data) {
  // Debug: Print the raw received data
  Serial.println("Raw received data: " + data);

  // Split the data manually and print each part
  String field1Val = getValue(data, ',', 0);
  String field2Val = getValue(data, ',', 1);
  String field3Val = getValue(data, ',', 2);
  String field4Val = getValue(data, ',', 3);

  Serial.println("Parsed values:");
  Serial.println("Field 1: " + field1Val);
  Serial.println("Field 2: " + field2Val);
  Serial.println("Field 3: " + field3Val);
  Serial.println("Field 4: " + field4Val);

  if (client.connect(server, 80)) {
    String getStr = "GET /update?api_key=";
    getStr += apiKey;
    getStr += "&field1=";
    getStr += field1Val;
    getStr += "&field2=";
    getStr += field2Val;
    getStr += "&field3=";
    getStr += field3Val;
    getStr += "&field4=";
    getStr += field4Val;
    getStr += " HTTP/1.1\r\nHost: ";
    getStr += server;
    getStr += "\r\nConnection: close\r\n\r\n";

    client.print(getStr);
    Serial.println("Sending to ThingSpeak: " + getStr);

    // Rest of the function remains the same...
  }
}


// Helper function to parse data
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  for (int i = 0; i < data.length() && found <= index; i++) {
    if (data.charAt(i) == separator || i == data.length() - 1) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == data.length() - 1) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
