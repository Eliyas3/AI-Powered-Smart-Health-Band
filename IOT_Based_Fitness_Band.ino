//Eliyas Science Info www.youtube.com/c/Eliyasscienceinfo
//IOT Fitness Band

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30100_PulseOximeter.h"


#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 32  
#define OLED_RESET -1     
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define REPORTING_PERIOD_MS 1000  
PulseOximeter pox;
uint32_t lastReportTime = 0;

#define LM35_PIN A0 

unsigned long activityReminderTime = 0;
#define ACTIVITY_REMINDER_INTERVAL 1800000 

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing...");

    // Initialize the OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Stop execution if OLED initialization fails
    }
    display.clearDisplay();
    display.setTextSize(1); // Use smaller text for displaying multiple lines
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    // Show welcome message before displaying sensor data
    display.println("    Welcome To  ");
    display.setCursor(0,12);
    display.println("   Fitness Band  ");
    display.display();
    delay(2000); // Show welcome message for 2 seconds
    display.clearDisplay(); // Clear screen after showing the welcome message

    // Initialize the MAX30100 PulseOximeter library
    if (!pox.begin()) {
        Serial.println("FAILED to initialize the MAX30100. Please check the wiring!");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("MAX30100 Error!");
        display.display();
        while (true); // Stop execution if MAX30100 initialization fails
    } else {
        Serial.println("MAX30100 initialized successfully.");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("MAX30100 Ready!");
        display.display();
    }

    // Set the callback for beat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
    // Update the MAX30100 sensor readings
    pox.update();

    // Read temperature from LM35
    float tempC = analogRead(LM35_PIN) * (3.3 / 1024.0) * 100.0; // LM35 gives 10mV/°C

    // Report heart rate, SpO2, and temperature every REPORTING_PERIOD_MS
    if (millis() - lastReportTime > REPORTING_PERIOD_MS) {
        lastReportTime = millis();

        // Read heart rate and SpO2
        float heartRate = pox.getHeartRate();
        float spO2 = pox.getSpO2();

        // Print to Serial Monitor
        Serial.print("Heart rate: ");
        Serial.print(heartRate);
        Serial.print(" bpm | SpO2: ");
        Serial.print(spO2);
        Serial.print(" % | Temp: ");
        Serial.print(tempC);
        Serial.println(" C");

        // Display on OLED
        display.clearDisplay();
        
        // First line: Heart Rate
        display.setCursor(0, 0); 
        display.print("Heart Rate: ");
        display.print(heartRate);
        display.println(" bpm");

        // Second line: SpO2
        display.setCursor(0, 12); // Position the cursor for the second line
        display.print("SpO2: ");
        display.print(spO2);
        display.println(" %");

        // Third line: Temperature (if there's space)
        display.setCursor(0, 24); // Position the cursor for the third line
        display.print("Temp: ");
        display.print(tempC);
        display.println(" C");

        display.display();
    }

    // Activity reminder logic
    if (millis() - activityReminderTime > ACTIVITY_REMINDER_INTERVAL) {
        activityReminderTime = millis(); // Reset the reminder timer

        // Display the activity reminder
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Activity Reminder!");
        display.setCursor(0, 12);
        display.println("Time to move!");
        display.display();

        // Wait for 5 seconds to show the activity reminder
        delay(5000); // 5 seconds delay before the next sensor reading
    }
}

// Callback function for beat detection
void onBeatDetected() {
    Serial.println("Beat detected!");
}
