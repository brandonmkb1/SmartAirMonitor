#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// Pin definitions
#define DHTPIN 2           // DHT11 sensor pin
#define DHTTYPE DHT11      // DHT11 type
#define MQ135PIN A0        // MQ-135 sensor analog pin
#define GREEN_LED 8        // Green LED pin
#define YELLOW_LED 9       // Yellow LED pin
#define RED_LED 10         // Red LED pin
#define BUZZER 11          // Buzzer pin

// Thresholds for air quality (adjust these values as needed)
#define GAS_GOOD 200     // Good air quality threshold
#define GAS_MEDIUM 400 // Medium air quality threshold
#define GAS_BAD 600      // Bad/harmful air quality threshold

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Initialize LCD (I2C address 0x27, 16 chars, 2 lines)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables for rotating display
unsigned long previousMillis = 0;    // Store last time screen was updated
const long interval = 1000;          // Interval for rotating display (1 second)
int displayState = 0;                // Track current state (0 = temp/humid, 1 = gas level)

// Variables to hold sensor data
float temperature;
float humidity;
int gasLevel;

void setup() {
  // Start the serial communication
  Serial.begin(9600);

  // Initialize DHT11 sensor
  dht.begin();

  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  
  // Set LED and buzzer pins as output
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Ensure all LEDs and buzzer are off at start
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  noTone(BUZZER);  // Ensure buzzer is off at start

  // Initial message on the serial monitor
  Serial.println("System initialized. Starting sensor readings...");
}

void loop() {
  // Read the DHT11 sensor
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // Read the MQ-135 sensor value
  gasLevel = analogRead(MQ135PIN);

  // Update the display every 'interval' milliseconds
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Save the last time the display was updated
    
    // Rotate between different states (0, 1)
    if (displayState == 0) {
      displayTemperatureHumidity();
      displayState = 1;  // Next, show gas level
    } else if (displayState == 1) {
      displayGasLevel();
      displayState = 0;  // Loop back to temperature/humidity
    }
  }
}

void displayTemperatureHumidity() {
  // Clear the LCD and display temperature/humidity
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Humid: ");
  lcd.print(humidity);
  lcd.print("%");

  // Print to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}

void displayGasLevel() {
  // Clear the LCD and display gas level
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gas Level: ");
  lcd.print(gasLevel);

  // Determine air quality and update LEDs/buzzer
  lcd.setCursor(0, 1);
  if (gasLevel < GAS_GOOD) {
    lcd.print("Air: Good ");
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    noTone(BUZZER);  // Ensure buzzer is off

    // Print to Serial Monitor
    Serial.println("Air Quality: Good");
  } 
  else if (gasLevel >= GAS_GOOD && gasLevel < GAS_MEDIUM) {
    lcd.print("Air: Med  ");
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    noTone(BUZZER);  // Ensure buzzer is off

    // Print to Serial Monitor
    Serial.println("Air Quality: Medium");
  } 
  else if (gasLevel >= GAS_MEDIUM && gasLevel < GAS_BAD) {
    lcd.print("Air: Bad  ");
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    noTone(BUZZER);  // Ensure buzzer is off for bad air quality

    // Print to Serial Monitor
    Serial.println("Air Quality: Bad");
  } 
  else {
    lcd.print("Air: Harm ");
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    
    // Activate buzzer for harmful air quality
    tone(BUZZER, 1000);  // Emit a 1kHz tone for passive buzzers
    
    // Print to Serial Monitor
    Serial.println("Air Quality: Harmful - Buzzer activated!");
  }

  // Print gas level to Serial Monitor
  Serial.print("Gas Level: ");
  Serial.println(gasLevel);
}
