
#define BLYNK_TEMPLATE_ID "TMPL3gcfp4U-l"
#define BLYNK_TEMPLATE_NAME "Smart Plant Monitoring"
#define BLYNK_AUTH_TOKEN "SCvwiI7Ymnn_K-F01dKgpDpRMoLgJtHQ"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// Initialize the LCD display
LiquidCrystal_I2C lcd(0x27, 16, 2);

char auth[] = "SCvwiI7Ymnn_K-F01dKgpDpRMoLgJtHQ";  // Enter your Blynk Auth token
char ssid[] = "Eliyas";  // Enter your WIFI SSID
char pass[] = "0123456789";  // Enter your WIFI Password

DHT dht(4, DHT11); // GPIO4 for DHT11 sensor
BlynkTimer timer;

// Define component pins
#define SOIL_PIN 32      // GPIO34 (ADC1) Soil Moisture Sensor
#define PIR_PIN 27       // GPIO27 PIR Motion Sensor
#define RELAY_PIN 25     // GPIO25 Relay
#define PUSH_BUTTON_PIN 26 // GPIO26 Button

int relayState = LOW;
int buttonState = HIGH;
int PIR_ToggleValue;

// Virtual Pins
#define VPIN_BUTTON V12

void checkPhysicalButton();

// Variables for sensor readings
void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);
  dht.begin();

  Blynk.begin(auth, ssid, pass);

  lcd.setCursor(0, 0);
  lcd.print("  Initializing  ");
  for (int a = 5; a <= 10; a++) {
    lcd.setCursor(a, 1);
    lcd.print(".");
    delay(500);
  }
  lcd.clear();
  lcd.setCursor(11, 1);
  lcd.print("W:OFF");

  // Timers for sensors
  timer.setInterval(1000L, readSoilMoisture);
  timer.setInterval(1000L, readDHTSensor);
  timer.setInterval(500L, checkPhysicalButton);
}

// Read DHT11 sensor values
void readDHTSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(t, 1);
  lcd.print("C");

  lcd.setCursor(8, 0);
  lcd.print("H:");
  lcd.print(h, 1);
  lcd.print("%");
}

// Read soil moisture sensor values
void readSoilMoisture() {
  int value = analogRead(SOIL_PIN);
  value = map(value, 0, 4095, 0, 100);
  value = (value - 100) * -1;

  Blynk.virtualWrite(V3, value);
  lcd.setCursor(0, 1);
  lcd.print("S:");
  lcd.print(value);
  lcd.print("%");
}

// Read PIR sensor values
void readPIRSensor() {
  bool value = digitalRead(PIR_PIN);
  if (value) {
    Blynk.logEvent("pirmotion", "WARNING! Motion Detected!");
    WidgetLED LED(V5);
    LED.on();
  } else {
    WidgetLED LED(V5);
    LED.off();
  }
}

BLYNK_WRITE(V6) {
  PIR_ToggleValue = param.asInt();
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(VPIN_BUTTON);
}

BLYNK_WRITE(VPIN_BUTTON) {
  relayState = param.asInt();
  digitalWrite(RELAY_PIN, relayState);
}

void checkPhysicalButton() {
  if (digitalRead(PUSH_BUTTON_PIN) == LOW) {
    if (buttonState != LOW) {
      relayState = !relayState;
      digitalWrite(RELAY_PIN, relayState);
      Blynk.virtualWrite(VPIN_BUTTON, relayState);
    }
    buttonState = LOW;
  } else {
    buttonState = HIGH;
  }
}

void loop() {
  if (PIR_ToggleValue == 1) {
    lcd.setCursor(5, 1);
    lcd.print("M:ON ");
    readPIRSensor();
  } else {
    lcd.setCursor(5, 1);
    lcd.print("M:OFF");
    WidgetLED LED(V5);
    LED.off();
  }

  if (relayState == HIGH) {
    lcd.setCursor(11, 1);
    lcd.print("W:ON ");
  } else {
    lcd.setCursor(11, 1);
    lcd.print("W:OFF");
  }

  Blynk.run();
  timer.run();
}
