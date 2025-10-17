/*
 * Solar Charging Station - Arduino Mega Sketch
 * Handles hardware control for relays, solenoids, UV lights, and fingerprint sensor
 * 
 * Fingerprint Sensor: AS608 (using Adafruit Fingerprint library)
 * VERSION: Without LED control (for compatibility)
 * Compatible with R307, R305, ZFM-20, etc.
 */

#include <ArduinoJson.h>
#include <Adafruit_Fingerprint.h>

// Software Serial for AS608 Fingerprint Sensor
// AS608 Default: 57600 baud, 3.3V or 5V compatible
#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX (Pin 10), TX (Pin 11)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Pin definitions for relays (slots 1-13)
const int RELAY_PINS[] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34};

// Pin definitions for solenoids (slots 4-13)
const int SOLENOID_PINS[] = {35, 36, 37, 38, 39, 40, 41, 42, 43, 44};

// Pin definitions for UV lights (slots 4-9)
const int UV_LIGHT_PINS[] = {45, 46, 47, 48, 49, 50};

// Coin acceptor pin
const int COIN_PIN = 2;

volatile int coinPulseCount = 0;
float coinValue = 0.0;

void setup() {
  Serial.begin(9600);
  
  // Initialize relay pins
  // ACTIVE HIGH (HIGH = ON, LOW = OFF)
  for (int i = 0; i < 13; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], LOW);  // LOW = OFF (default state)
  }
  
  // Initialize solenoid pins
  // HIGH = locked, LOW = unlocked
  for (int i = 0; i < 10; i++) {
    pinMode(SOLENOID_PINS[i], OUTPUT);
    digitalWrite(SOLENOID_PINS[i], HIGH);  // HIGH = locked (default state)
  }
  
  // Initialize UV light pins
  // HIGH = ON, LOW = OFF
  for (int i = 0; i < 6; i++) {
    pinMode(UV_LIGHT_PINS[i], OUTPUT);
    digitalWrite(UV_LIGHT_PINS[i], LOW);  // LOW = OFF (default state)
  }
  
  // Initialize coin acceptor
  pinMode(COIN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(COIN_PIN), coinInterrupt, FALLING);
  
  // Initialize AS608 fingerprint sensor
  finger.begin(57600);
  
  if (finger.verifyPassword()) {
    Serial.println("{\"status\":\"AS608 Fingerprint sensor found and verified\"}");
    
    finger.getTemplateCount();
    if (finger.templateCount > 0) {
      Serial.print("{\"status\":\"AS608 has ");
      Serial.print(finger.templateCount);
      Serial.println(" fingerprints enrolled\"}");
    }
  } else {
    Serial.println("{\"status\":\"AS608 Fingerprint sensor not found or password incorrect\"}");
    Serial.println("{\"help\":\"Check connections: VCC->5V, GND->GND, TX->Pin11, RX->Pin10\"}");
  }
  
  Serial.println("{\"status\":\"Arduino Ready\"}");
}

void loop() {
  if (Serial.available() > 0) {
    String jsonString = Serial.readStringUntil('\n');
    processCommand(jsonString);
  }
  
  if (coinPulseCount > 0) {
    processCoinPulse();
  }
}

void processCommand(String jsonString) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, jsonString);
  
  if (error) {
    sendResponse(false, "Invalid JSON");
    return;
  }
  
  String command = doc["command"];
  JsonObject data = doc["data"];
  
  if (command == "RELAY") {
    handleRelay(data);
  } else if (command == "SOLENOID") {
    handleSolenoid(data);
  } else if (command == "UV_LIGHT") {
    handleUVLight(data);
  } else if (command == "FINGERPRINT_VERIFY") {
    handleFingerprintVerify(data);
  } else if (command == "FINGERPRINT_ENROLL") {
    handleFingerprintEnroll(data);
  } else if (command == "READ_COIN") {
    handleReadCoin();
  } else {
    sendResponse(false, "Unknown command");
  }
}

void handleRelay(JsonObject data) {
  int slot = data["slot"];
  bool state = data["state"];
  
  if (slot >= 1 && slot <= 13) {
    // ACTIVE HIGH: HIGH = ON, LOW = OFF
    digitalWrite(RELAY_PINS[slot - 1], state ? HIGH : LOW);
    sendResponse(true, "Relay controlled");
  } else {
    sendResponse(false, "Invalid slot number");
  }
}

void handleSolenoid(JsonObject data) {
  int slot = data["slot"];
  bool lock = data["lock"];
  
  if (slot >= 4 && slot <= 13) {
    // HIGH = locked, LOW = unlocked
    digitalWrite(SOLENOID_PINS[slot - 4], lock ? HIGH : LOW);
    sendResponse(true, "Solenoid controlled");
  } else {
    sendResponse(false, "Invalid slot number for solenoid");
  }
}

void handleUVLight(JsonObject data) {
  int slot = data["slot"];
  bool state = data["state"];
  
  if (slot >= 4 && slot <= 9) {
    digitalWrite(UV_LIGHT_PINS[slot - 4], state ? HIGH : LOW);
    sendResponse(true, "UV light controlled");
  } else {
    sendResponse(false, "Invalid slot number for UV light");
  }
}

void handleFingerprintVerify(JsonObject data) {
  int expectedId = data["id"];
  
  Serial.println("{\"status\":\"Waiting for finger on AS608 sensor...\"}");
  
  // Wait for finger with timeout
  unsigned long timeout = millis() + 5000; // 5 second timeout
  int p = -1;
  
  while (p != FINGERPRINT_OK && millis() < timeout) {
    p = finger.getImage();
    delay(50);
  }
  
  if (p != FINGERPRINT_OK) {
    StaticJsonDocument<100> doc;
    doc["success"] = true;
    doc["isValid"] = false;
    doc["error"] = "No finger detected or timeout";
    
    String response;
    serializeJson(doc, response);
    Serial.println(response);
    return;
  }
  
  // Convert image to template
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    StaticJsonDocument<100> doc;
    doc["success"] = true;
    doc["isValid"] = false;
    doc["error"] = "Image conversion failed";
    
    String response;
    serializeJson(doc, response);
    Serial.println(response);
    return;
  }
  
  // Search for match in database
  p = finger.fingerFastSearch();
  
  if (p == FINGERPRINT_OK) {
    // Match found - now verify it's the correct fingerprint
    bool isCorrectFingerprint = (finger.fingerID == expectedId);
    
    if (isCorrectFingerprint) {
      // Correct fingerprint!
      StaticJsonDocument<150> doc;
      doc["success"] = true;
      doc["isValid"] = true;
      doc["fingerprintId"] = finger.fingerID;
      doc["confidence"] = finger.confidence;
      doc["matchScore"] = finger.confidence;
      
      String response;
      serializeJson(doc, response);
      Serial.println(response);
    } else {
      // Wrong fingerprint matched!
      StaticJsonDocument<150> doc;
      doc["success"] = true;
      doc["isValid"] = false;
      doc["error"] = "Wrong fingerprint detected";
      doc["matchedId"] = finger.fingerID;
      doc["expectedId"] = expectedId;
      doc["confidence"] = finger.confidence;
      
      String response;
      serializeJson(doc, response);
      Serial.println(response);
    }
  } else {
    // No match in database
    StaticJsonDocument<100> doc;
    doc["success"] = true;
    doc["isValid"] = false;
    
    if (p == FINGERPRINT_NOTFOUND) {
      doc["error"] = "No match found in database";
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      doc["error"] = "Communication error";
    } else {
      doc["error"] = "Verification failed";
    }
    
    String response;
    serializeJson(doc, response);
    Serial.println(response);
  }
}

void handleFingerprintEnroll(JsonObject data) {
  int fingerprintId = data["userId"];
  int p = -1;
  
  Serial.println("{\"status\":\"Starting AS608 fingerprint enrollment...\"}");
  
  // Step 1: Get first image
  Serial.println("{\"status\":\"Place finger on sensor\"}");
  
  unsigned long timeout = millis() + 10000; // 10 second timeout
  p = -1;
  while (p != FINGERPRINT_OK && millis() < timeout) {
    p = finger.getImage();
    delay(50);
  }
  
  if (p != FINGERPRINT_OK) {
    sendResponse(false, "Timeout waiting for finger");
    return;
  }
  
  // Convert image to template in slot 1
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    sendResponse(false, "Image conversion failed");
    return;
  }
  
  // Step 2: Remove finger
  Serial.println("{\"status\":\"Remove finger\"}");
  delay(2000);
  
  // Wait for finger removal (with timeout)
  timeout = millis() + 5000;
  while (finger.getImage() != FINGERPRINT_NOFINGER && millis() < timeout) {
    delay(50);
  }
  
  // Step 3: Get second image
  Serial.println("{\"status\":\"Place same finger again\"}");
  
  timeout = millis() + 10000;
  p = -1;
  while (p != FINGERPRINT_OK && millis() < timeout) {
    p = finger.getImage();
    delay(50);
  }
  
  if (p != FINGERPRINT_OK) {
    sendResponse(false, "Timeout waiting for second scan");
    return;
  }
  
  // Convert image to template in slot 2
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    sendResponse(false, "Second image conversion failed");
    return;
  }
  
  // Step 4: Create model from both templates
  Serial.println("{\"status\":\"Creating fingerprint template...\"}");
  
  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    if (p == FINGERPRINT_ENROLLMISMATCH) {
      sendResponse(false, "Fingerprints did not match. Please try again.");
    } else {
      sendResponse(false, "Failed to create fingerprint model");
    }
    return;
  }
  
  // Step 5: Store model in AS608 memory
  Serial.println("{\"status\":\"Storing fingerprint...\"}");
  
  p = finger.storeModel(fingerprintId);
  if (p == FINGERPRINT_OK) {
    StaticJsonDocument<100> doc;
    doc["success"] = true;
    doc["message"] = "Fingerprint enrolled successfully";
    doc["fingerprintId"] = fingerprintId;
    
    String response;
    serializeJson(doc, response);
    Serial.println(response);
  } else {
    if (p == FINGERPRINT_BADLOCATION) {
      sendResponse(false, "Could not store in that location");
    } else if (p == FINGERPRINT_FLASHERR) {
      sendResponse(false, "Error writing to flash memory");
    } else {
      sendResponse(false, "Failed to store fingerprint");
    }
  }
}

void handleReadCoin() {
  StaticJsonDocument<100> doc;
  doc["success"] = true;
  doc["value"] = coinValue;
  
  String response;
  serializeJson(doc, response);
  Serial.println(response);
  
  coinValue = 0.0;
  coinPulseCount = 0;
}

void coinInterrupt() {
  coinPulseCount++;
}

void processCoinPulse() {
  // Different pulse counts for different denominations
  if (coinPulseCount == 1) {
    coinValue = 1.0;
  } else if (coinPulseCount == 5) {
    coinValue = 5.0;
  } else if (coinPulseCount == 10) {
    coinValue = 10.0;
  } else if (coinPulseCount == 20) {
    coinValue = 20.0;
  }
  
  coinPulseCount = 0;
}

void sendResponse(bool success, const char* message) {
  StaticJsonDocument<100> doc;
  doc["success"] = success;
  doc["message"] = message;
  
  String response;
  serializeJson(doc, response);
  Serial.println(response);
}

