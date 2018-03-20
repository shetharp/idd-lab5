// Define pins
#define ledPin 9
#define recordBtn 7
#define playBtn 6
#define speakerPin 8

// Include libraries
#include "Adafruit_VL53L0X.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>

// Program variables
int addr = 0;
int data = 0;
unsigned long start = 0;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Initialize the sensor object
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  // Setup pins
  pinMode(ledPin, OUTPUT);
  pinMode(recordBtn, INPUT);
  pinMode(playBtn, INPUT);
  pinMode(speakerPin, OUTPUT);

  // Start LCD and Serial monitor
  lcd.begin(16, 2);
  Serial.begin(115200);
  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }
  
  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  // power 
  Serial.println(F("VL53L0X API Simple Ranging example\n\n")); 
  
  // Initialize UI
  lcd.clear();
  lcd.print("Press left");
  lcd.setCursor(0,1);
  lcd.print("button to record");
}

void loop() {

  // Turn LED and speaker off if user is not recording or playing song
  analogWrite(ledPin, 0);
  noTone(speakerPin);  

  // If the user presses the record button
  if (digitalRead(recordBtn) == LOW) {
    // First clear the EEPROM
    lcd.clear();
    lcd.print("Recording in...");
    start = millis();
    addr = 0;
    for (int i = 0 ; i < EEPROM.length() ; i++) {
      lcd.setCursor(0,1);
      lcd.print(String(3-((millis()-start)/1000)));
      EEPROM.write(i, 0);
    }
    // Begin recording
    lcd.clear();
    lcd.print("Recording!");
    lcd.setCursor(0,1);
    lcd.print("Value: ");
    start = millis();
    // Record for 10 seconds
    while(millis()-start < 10000) {
      // Sensor measure object
      VL53L0X_RangingMeasurementData_t measure;

      // Get the measurement
      Serial.print("Reading a measurement... ");
      lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

      // Check for a valid measurement
      if (measure.RangeStatus != 4) {  // phase failures have incorrect data
        Serial.print("Distance (mm): ");
        Serial.println(String(measure.RangeMilliMeter));

        // Convert the distance in mm to a byte-size (0-255) data point
        data = constrain(map(measure.RangeMilliMeter, 30, 400, 0, 255), 0, 255);
        // Write out the data to the LED
        analogWrite(ledPin, data);
        // Convert the data to a tone to play on the speaker
        tone(speakerPin, constrain(map(data, 0, 255, 30, 4800), 30, 4800));
        // Write the data to EEPROM
        EEPROM.write(addr, data);
        if (addr >= EEPROM.length()) {
          addr = 0;
        } else {
          addr++;
        }
        // Print the data to the LCD screen as well as the time remaining
        lcd.setCursor(11,0);
        lcd.print("   ");
        lcd.setCursor(11,0);
        lcd.print(String(10-((millis()-start)/1000)));
        lcd.setCursor(7,1);
        lcd.print("   ");
        lcd.setCursor(7,1);
        lcd.print(String(data));
      }
      // Delay between sensor readings
      delay(10);
    }
    // Update the UI when recording is finished
    lcd.clear();
    lcd.print("Left: Record new");
    lcd.setCursor(0,1);
    lcd.print("Right: Play back");
  }

  // User presses the play button
  if (digitalRead(playBtn) == LOW) {
    // Begin playing the song
    lcd.clear();
    lcd.print("Playing!");
    lcd.setCursor(0,1);
    lcd.print("Value: ");
    start = millis();
    // Read the EEPROM
    for (int i = 0; i < EEPROM.length(); i++) {
      analogWrite(ledPin, EEPROM.read(i));
      tone(speakerPin, constrain(map(EEPROM.read(i), 0, 255, 30, 4800), 30, 4800));
      lcd.setCursor(9,0);
      lcd.print("   ");
      lcd.setCursor(9,0);
      lcd.print(String(10-((millis()-start)/1000)));
      lcd.setCursor(7,1);
      lcd.print("   ");
      lcd.setCursor(7,1);
      lcd.print(String(EEPROM.read(i)));
      if ((10-((millis()-start)/1000) <= 0 || (10-((millis()-start)/1000) > 10))) {
        break;
      }
      delay(50);
    }
    // Update the UI when song is done playing
    lcd.clear();
    lcd.print("Left: Record new");
    lcd.setCursor(0,1);
    lcd.print("Right: Play back");
  } 
}
