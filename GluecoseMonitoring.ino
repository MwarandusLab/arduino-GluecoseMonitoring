#include <LiquidCrystal.h>

// Pin definitions
const int touchPin1 = 2;
const int touchPin2 = 11;
const int irSensorPin = 3;  // Assuming the IR sensor is connected to pin 3

long threshold = 5000;
long threshold_2 = 7000;

const int rs = 9, en = 8, d4 = 7, d5 = 6, d6 = 5, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int previousReading = 150;  // Initial arbitrary value within the range
int readingCounter = 0;

void setup() {
  Serial.begin(9600);
  pinMode(touchPin1, INPUT);
  pinMode(touchPin2, INPUT);
  pinMode(irSensorPin, INPUT);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("SYSTEM");
  lcd.setCursor(1, 1);
  lcd.print("INITIALIZATION");
  delay(2000);
  lcd.clear();
}

void loop() {
  long total1 = 0;
  long total2 = 0;

  // Measure the capacitance
  for (int i = 0; i < 10; i++) {
    total1 += touchRead(touchPin1);
    total2 += touchRead(touchPin2);
  }

  total1 /= 10;
  total2 /= 10;

  // Compare the readings to a threshold to detect touch
  // Serial.print("Total1: ");
  // Serial.print(total1);
  // Serial.print("  Total2: ");
  // Serial.println(total2);

  // Set the threshold based on your observations
  bool fingerDetected = (total1 < threshold && total2 < threshold_2);
  bool irDetected = digitalRead(irSensorPin) == LOW;  // Assuming LOW means obstacle detected

  if (fingerDetected && irDetected) {
    startCountdown();
  } else {
    //Serial.println("No touch detected or IR not triggered");
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("PLACE FINGER");
    lcd.setCursor(0, 1);
    lcd.print("MEASURE G LEVEL");
  }

  delay(100);
}

long touchRead(int pin) {
  // Set the pin to OUTPUT and discharge the capacitance
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(1);

  // Set the pin to INPUT and count how long it takes for the pin to go HIGH
  pinMode(pin, INPUT);
  long count = 0;
  while (digitalRead(pin) == LOW) {
    count++;
  }

  return count;
}

void startCountdown() {
  for (int i = 10; i > 0; i--) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RESULTS WILL BE");
    lcd.setCursor(0, 1);
    lcd.print("READY IN: ");
    lcd.print(i);
    lcd.print(" sec");

    // Check if the finger is removed during the countdown
    long total1 = 0;
    long total2 = 0;
    for (int j = 0; j < 10; j++) {
      total1 += touchRead(touchPin1);
      total2 += touchRead(touchPin2);
    }
    total1 /= 10;
    total2 /= 10;
    bool fingerDetected = (total1 < threshold && total2 < threshold_2);
    bool irDetected = digitalRead(irSensorPin) == LOW;  // Assuming LOW means obstacle detected

    if (!fingerDetected || !irDetected) {
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("FINGER REMOVED");
      lcd.setCursor(3, 1);
      lcd.print("TRY AGAIN");
      delay(2000);

      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("PLACE FINGER");
      lcd.setCursor(0, 1);
      lcd.print("MEASURE G LEVEL");
      return;
    }

    delay(1000);
  }

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("RESULTS READY");
  lcd.setCursor(2, 1);
  lcd.print("REMOVE FINGER");

  // Wait until the finger is removed
  unsigned long startTime = millis();
  while (true) {
    long total1 = 0;
    long total2 = 0;
    for (int i = 0; i < 10; i++) {
      total1 += touchRead(touchPin1);
      total2 += touchRead(touchPin2);
    }
    total1 /= 10;
    total2 /= 10;
    bool fingerDetected = (total1 < threshold && total2 < threshold_2);

    if (!fingerDetected) {
      break;
    }

    // Timeout after 5 seconds
    if (millis() - startTime > 5000) {
      int newReading = generateRandomReading();

      // Display results for 5 seconds
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("GLUCOSE LEVEL");
      lcd.setCursor(3, 1);
      lcd.print(newReading);
      lcd.print(" Mg/DL");
      delay(5000);

      // Wait for the finger to be placed again
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("PLACE FINGER");
      lcd.setCursor(0, 1);
      lcd.print("MEASURE G LEVEL");
      return;
    }

    delay(100);
  }
}

int generateRandomReading() {
  int newReading;
  if (readingCounter % 3 == 2) {  // Every third reading can be far apart
    newReading = random(120, 200);
  } else {
    newReading = previousReading + random(-5, 6);  // Previous value +/- 5
    // Ensure the new reading is within the valid range
    if (newReading < 120) newReading = 120;
    if (newReading > 200) newReading = 200;
  }
  previousReading = newReading;
  readingCounter++;
  return newReading;
}
