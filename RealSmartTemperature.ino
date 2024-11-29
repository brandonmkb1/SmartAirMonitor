#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// Pin definitions
#define DHTPIN 2           // DHT11 sensor pin
#define DHTTYPE DHT11      // DHT11 type
#define MQ135PIN A0        // MQ-135 sensor analog pin
#define FLAMEPIN A1        // Flame sensor pin
#define GREEN_LED 8        // Green LED pin
#define YELLOW_LED 9       // Yellow LED pin
#define RED_LED 10         // Red LED pin
#define BUZZER 11          // Buzzer pin

// Thresholds for air quality
#define GAS_GOOD 500       // Good air quality threshold
#define GAS_MEDIUM 700     // Medium air quality threshold
#define GAS_BAD 900        // Bad/harmful air quality threshold
#define FLAME_THRESHOLD 150 // Flame sensor detection threshold (analog value)

// Calibration offsets for DHT11
float tempOffset = -12.0;  // Adjust temperature (°C)
float humidOffset = 0.0;   // Adjust humidity (%)

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Initialize LCD (I2C address 0x27, 16 chars, 2 lines)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables for rotating display
unsigned long previousMillis = 0;    // Store last time screen was updated
const long interval = 2000;          // Interval for rotating display (2 seconds)
int displayState = 0;                // Track current state (0 = temp/humid, 1 = gas level)

// Variables to hold sensor data
float temperature;
float humidity;
int gasLevel;
int flameValue;

// Flags for LED and buzzer states
bool flameActive = false;
bool gasHarmActive = false;

void setup() {
  Serial.begin(115200);  // Communication with ESP8266
  dht.begin();
  lcd.init();
  lcd.backlight();
  
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(FLAMEPIN, INPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  noTone(BUZZER);

  Serial.println("System initialized. Starting sensor readings...");
}

void loop() {
  flameValue = analogRead(FLAMEPIN);
  gasLevel = analogRead(MQ135PIN);

  // Handle Flame Sensor
  if (flameValue < FLAME_THRESHOLD) {
    flameActive = true;
  } else {
    flameActive = false;
  }

  // Handle Gas Sensor
  if (gasLevel >= GAS_BAD) {
    gasHarmActive = true;
  } else {
    gasHarmActive = false;
  }

  // Priority Control
  if (flameActive) {
    activateRedAlert();
    return;  // Exit early, as flame detection takes priority
  } 
  else if (gasHarmActive) {
    activateRedAlert();
  } 
  else {
    deactivateRedAlert();
    handleGasIndicators();
  }

  // Update Display and Serial Output
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    float rawTemp = dht.readTemperature();
    float rawHumid = dht.readHumidity();
    temperature = rawTemp + tempOffset;
    humidity = rawHumid + humidOffset;

    if (displayState == 0) {
      displayTemperatureHumidity();
      displayState = 1;
    } else {
      displayGasLevel();
      displayState = 0;
    }

    // Print to Serial for debugging
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(", Humidity: ");
    Serial.print(humidity);
    Serial.print(", Gas Level: ");
    Serial.println(gasLevel);

    // Send data to ESP8266
    sendDataToNodeMCU();
  }
}

void sendDataToNodeMCU() {
  // Use dtostrf for float to string conversion
  char tempStr[6], humidStr[6];
  dtostrf(temperature, 4, 2, tempStr);
  dtostrf(humidity, 4, 2, humidStr);

  // Construct a clean, comma-separated string
  String dataString = String(tempStr) + "," + 
                      String(humidStr) + "," + 
                      String(gasLevel) + "," + 
                      String(flameValue);
  
  Serial.println(dataString);
}



void activateRedAlert() {
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  tone(BUZZER, 1000);  // Emit 1kHz tone consistently
}

void deactivateRedAlert() {
  digitalWrite(RED_LED, LOW);
  noTone(BUZZER);
}

void handleGasIndicators() {
  if (gasLevel >= GAS_MEDIUM) {
    // Medium or bad air quality
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
  } else if (gasLevel < GAS_MEDIUM && gasLevel >= GAS_GOOD) {
    // Good air quality
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
  } else {
    // Very good air quality (below GAS_GOOD)
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
  }
}

void displayTemperatureHumidity() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Humid: ");
  lcd.print(humidity);
  lcd.print("%");
}

void displayGasLevel() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gas Level: ");
  lcd.print(gasLevel);
  lcd.setCursor(0, 1);

  if (gasLevel < GAS_GOOD) {
    lcd.print("Air: Good ");
  } else if (gasLevel >= GAS_GOOD && gasLevel < GAS_MEDIUM) {
    lcd.print("Air: Med  ");
  } else if (gasLevel >= GAS_MEDIUM && gasLevel < GAS_BAD) {
    lcd.print("Air: Bad  ");
  } else {
    lcd.print("Air: Harm ");
  }
}

