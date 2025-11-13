/*
 * Solar Charging Station - Arduino Mega Sketch
 * Handles hardware control for relays, solenoids, UV lights, and fingerprint sensor
 * 
 * Fingerprint Sensor: AS608 (using Adafruit Fingerprint library)
 * Compatible with R307, R305, ZFM-20, etc.
 */

#include <ArduinoJson.h>
#include <Adafruit_Fingerprint.h>

// Software Serial for AS608 Fingerprint Sensor
// AS608 Default: 57600 baud, 3.3V or 5V compatible
#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX (Pin 10), TX (Pin 11)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// ⚠️ CONFIGURATION: Clear fingerprint database on startup
// Set to true to clear all fingerprints when Arduino starts
// Set to false for normal operation (keeps enrolled fingerprints)
const bool CLEAR_DATABASE_ON_STARTUP = true;  // ⚠️ Change to true to clear database

constexpr int TOTAL_SLOTS = 16;
constexpr int UNUSED_PIN = -1;

// Pin definitions for relays (slots 1-16)
// Update these mappings if wiring changes for the new secure slots (4-6)
const int RELAY_PINS[TOTAL_SLOTS + 1] = {
  UNUSED_PIN, // index 0 unused
  22, // Slot 1
  23, // Slot 2
  24, // Slot 3
  54, // Slot 4 (Secure)
  55, // Slot 5 (Secure)
  56, // Slot 6 (Secure)
  25, // Slot 7 (Phone, original slot 4)
  28, // Slot 8 (Phone)
  31, // Slot 9 (Phone)
  34, // Slot 10 (Phone)
  37, // Slot 11 (Phone)
  40, // Slot 12 (Phone)
  43, // Slot 13 (Laptop)
  45, // Slot 14 (Laptop)
  47, // Slot 15 (Laptop)
  49  // Slot 16 (Laptop)
};

// Pin definitions for solenoids (secure, phone, laptop slots)
// Slots 4-6 are the new secure bays and use dedicated pins
const int SOLENOID_PINS[TOTAL_SLOTS + 1] = {
  UNUSED_PIN, // index 0 unused
  UNUSED_PIN, // Slot 1 - no solenoid
  UNUSED_PIN, // Slot 2 - no solenoid
  UNUSED_PIN, // Slot 3 - no solenoid
  57,         // Slot 4  (Secure)
  58,         // Slot 5  (Secure)
  59,         // Slot 6  (Secure)
  26,         // Slot 7  (Phone, original slot 4)
  29,         // Slot 8  (Phone)
  32,         // Slot 9  (Phone)
  35,         // Slot 10 (Phone)
  38,         // Slot 11 (Phone)
  40,         // Slot 12 (Phone)
  41,         // Slot 13 (Laptop)
  42,         // Slot 14 (Laptop)
  44,         // Slot 15 (Laptop)
  46          // Slot 16 (Laptop)
};

// Pin definitions for UV lights (phone slots 7-12)
const int UV_LIGHT_PINS[TOTAL_SLOTS + 1] = {
  UNUSED_PIN, // index 0 unused
  UNUSED_PIN, // Slot 1
  UNUSED_PIN, // Slot 2
  UNUSED_PIN, // Slot 3
  UNUSED_PIN, // Slot 4
  UNUSED_PIN, // Slot 5
  UNUSED_PIN, // Slot 6
  27,         // Slot 7  (Phone, original slot 4)
  30,         // Slot 8  (Phone)
  33,         // Slot 9  (Phone)
  36,         // Slot 10 (Phone)
  39,         // Slot 11 (Phone)
  42,         // Slot 12 (Phone)
  UNUSED_PIN, // Slot 13
  UNUSED_PIN, // Slot 14
  UNUSED_PIN, // Slot 15
  UNUSED_PIN  // Slot 16
};

int getRelayPin(int slot) {
  if (slot < 1 || slot > TOTAL_SLOTS) {
    return UNUSED_PIN;
  }
  return RELAY_PINS[slot];
}

int getSolenoidPin(int slot) {
  if (slot < 1 || slot > TOTAL_SLOTS) {
    return UNUSED_PIN;
  }
  return SOLENOID_PINS[slot];
}

int getUvLightPin(int slot) {
  if (slot < 1 || slot > TOTAL_SLOTS) {
    return UNUSED_PIN;
  }
  return UV_LIGHT_PINS[slot];
}

// Coin acceptor pin
const int COIN_PIN = 4;

// ===== HARDWARE CONFIGURATION =====
// Change these values based on your specific hardware setup
// If your relays/solenoids/UV lights work backwards, just swap HIGH and LOW

// Relay configuration (slots 1-16)
const int RELAY_ON = LOW;      // Change to LOW if your relays are active-low
const int RELAY_OFF = HIGH;      // Change to HIGH if your relays are active-low

// Solenoid lock configuration (secure, phone, laptop slots)
const int SOLENOID_LOCKED = HIGH;    // Change to LOW if your solenoids lock with LOW signal
const int SOLENOID_UNLOCKED = LOW;   // Change to HIGH if your solenoids unlock with HIGH signal

// UV light configuration (phone slots 7-12)
const int UV_LIGHT_ON = LOW;   // Change to LOW if your UV lights turn on with LOW signal
const int UV_LIGHT_OFF = HIGH;   // Change to HIGH if your UV lights turn off with HIGH signal
// ===================================

volatile int coinPulseCount = 0;
float coinValue = 0.0;
unsigned long coinDetectedTime = 0;
bool coinProcessed = false;
unsigned long lastCoinPulseTime = 0;

void setup() {
  Serial.begin(9600);
  
  // Initialize relay pins
  // Use configuration constants for relay behavior
  for (int slot = 1; slot <= TOTAL_SLOTS; slot++) {
    int relayPin = getRelayPin(slot);
    if (relayPin != UNUSED_PIN) {
      pinMode(relayPin, OUTPUT);
      digitalWrite(relayPin, RELAY_OFF);  // Default state: OFF
    }
  }
  
  // Initialize solenoid pins
  // Use configuration constants for lock behavior
  for (int slot = 1; slot <= TOTAL_SLOTS; slot++) {
    int solenoidPin = getSolenoidPin(slot);
    if (solenoidPin != UNUSED_PIN) {
      pinMode(solenoidPin, OUTPUT);
      digitalWrite(solenoidPin, SOLENOID_LOCKED);  // Default state: locked
    }
  }
  
  // Initialize UV light pins
  // Use configuration constants for UV light behavior
  for (int slot = 1; slot <= TOTAL_SLOTS; slot++) {
    int uvPin = getUvLightPin(slot);
    if (uvPin != UNUSED_PIN) {
      pinMode(uvPin, OUTPUT);
      digitalWrite(uvPin, UV_LIGHT_OFF);  // Default state: OFF
    }
  }
  
  // Initialize coin acceptor
  pinMode(COIN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(COIN_PIN), coinInterrupt, FALLING);
  
  // Initialize AS608 fingerprint sensor
  // AS608 default baud rate: 57600
  // Can be changed to 9600, 19200, 38400, 57600, 115200 via AT commands
  finger.begin(57600);

  //finger.setPassword(0x00000000);  // Reset to default
  
  if (finger.verifyPassword()) {
    Serial.println("{\"status\":\"AS608 Fingerprint sensor found and verified\"}");
    
    // ⚠️ Clear database if configured to do so
    if (CLEAR_DATABASE_ON_STARTUP) {
      Serial.println("{\"status\":\"⚠️ CLEARING FINGERPRINT DATABASE...\"}");
      if (finger.emptyDatabase() == FINGERPRINT_OK) {
        Serial.println("{\"status\":\"✓ Fingerprint database cleared successfully\"}");
        Serial.println("{\"info\":\"All enrolled fingerprints have been deleted\"}");
      } else {
        Serial.println("{\"status\":\"✗ Failed to clear fingerprint database\"}");
      }
    }
    
    // Optional: Read sensor parameters
    finger.getTemplateCount();
    if (finger.templateCount > 0) {
      Serial.print("{\"status\":\"AS608 has ");
      Serial.print(finger.templateCount);
      Serial.println(" fingerprints enrolled\"}");
    } else {
      Serial.println("{\"status\":\"AS608 database is empty (no fingerprints enrolled)\"}");
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
  
  // Check for coin detection
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
  } else if (command == "UNLOCK_TEMP") {
    handleUnlockTemp(data);
  } else if (command == "FINGERPRINT_DELETE") {
    handleFingerprintDelete(data);
  } else {
    sendResponse(false, "Unknown command");
  }
}

void handleRelay(JsonObject data) {
  int slot = data["slot"];
  bool state = data["state"];
  
  int relayPin = getRelayPin(slot);
  if (relayPin == UNUSED_PIN) {
    sendResponse(false, "Invalid slot number for relay");
    return;
  }

  // Use configuration constants for relay behavior
  digitalWrite(relayPin, state ? RELAY_ON : RELAY_OFF);
  sendResponse(true, "Relay controlled");
}

void handleSolenoid(JsonObject data) {
  int slot = data["slot"];
  bool lock = data["lock"];
  int duration = data["duration"] | 0; // Get duration in seconds, default 0 (permanent)
  
  int solenoidPin = getSolenoidPin(slot);
  if (solenoidPin == UNUSED_PIN) {
    sendResponse(false, "Slot does not support solenoid control");
    return;
  }

  // Use configuration constants for lock/unlock behavior
  digitalWrite(solenoidPin, lock ? SOLENOID_LOCKED : SOLENOID_UNLOCKED);
  
  // If duration is specified, wait and then return to locked state
  if (duration > 0 && !lock) {
    delay(duration * 1000); // Convert seconds to milliseconds
    digitalWrite(solenoidPin, SOLENOID_LOCKED); // Return to locked state
    sendResponse(true, "Solenoid timed unlock completed");
  } else {
    sendResponse(true, "Solenoid controlled");
  }
}

void handleUVLight(JsonObject data) {
  int slot = data["slot"];
  bool state = data["state"];
  
  int uvPin = getUvLightPin(slot);
  if (uvPin == UNUSED_PIN) {
    sendResponse(false, "Slot does not support UV sanitization");
    return;
  }

  // Use configuration constants for UV light behavior
  digitalWrite(uvPin, state ? UV_LIGHT_ON : UV_LIGHT_OFF);
  sendResponse(true, "UV light controlled");
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
  
  // Step 0: Delete existing fingerprint if it exists (auto-cleanup)
  Serial.println("{\"status\":\"Checking for existing fingerprint...\"}");
  uint8_t deleteResult = finger.deleteModel(fingerprintId);
  if (deleteResult == FINGERPRINT_OK) {
    Serial.println("{\"status\":\"Deleted existing fingerprint - ready for re-enrollment\"}");
  } else {
    Serial.println("{\"status\":\"No existing fingerprint found - proceeding with enrollment\"}");
  }
  
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
    // Provide specific error message based on error code
    if (p == FINGERPRINT_IMAGEMESS) {
      sendResponse(false, "Image too messy - clean sensor and finger, press firmly");
    } else if (p == FINGERPRINT_FEATUREFAIL) {
      sendResponse(false, "No fingerprint features found - press harder, cover entire sensor");
    } else if (p == FINGERPRINT_INVALIDIMAGE) {
      sendResponse(false, "Invalid image - clean sensor and try again");
    } else {
      sendResponse(false, "Image conversion failed - ensure finger covers sensor completely");
    }
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
    // Provide specific error message based on error code
    if (p == FINGERPRINT_IMAGEMESS) {
      sendResponse(false, "Second image too messy - clean sensor and finger, press firmly");
    } else if (p == FINGERPRINT_FEATUREFAIL) {
      sendResponse(false, "No features in second scan - press harder, cover entire sensor");
    } else if (p == FINGERPRINT_INVALIDIMAGE) {
      sendResponse(false, "Second image invalid - clean sensor and try again");
    } else {
      sendResponse(false, "Second image conversion failed - ensure finger covers sensor completely");
    }
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
    // Success!
    StaticJsonDocument<100> doc;
    doc["success"] = true;
    doc["message"] = "Fingerprint enrolled successfully";
    doc["fingerprintId"] = fingerprintId;
    
    String response;
    serializeJson(doc, response);
    Serial.println(response);
  } else {
    // Enhanced error messages with troubleshooting hints
    StaticJsonDocument<150> doc;
    doc["success"] = false;
    doc["fingerprintId"] = fingerprintId;
    
    if (p == FINGERPRINT_BADLOCATION) {
      doc["error"] = "Invalid fingerprint ID location";
      doc["message"] = "Use fingerprint ID between 1-127";
    } else if (p == FINGERPRINT_FLASHERR) {
      doc["error"] = "Fingerprint ID already exists or flash memory error";
      doc["message"] = "This slot was already taken. Try deleting it first or use a different ID.";
      doc["hint"] = "Run enrollment again - auto-delete should fix this";
    } else {
      doc["error"] = "Failed to store fingerprint";
      doc["message"] = "Unknown error during storage";
      doc["errorCode"] = p;
    }
    
    String response;
    serializeJson(doc, response);
    Serial.println(response);
  }
}

void handleReadCoin() {
  StaticJsonDocument<100> doc;
  doc["success"] = true;
  
  // Return coin value if detected within last 2 seconds
  unsigned long currentTime = millis();
  
  if (coinValue > 0 && !coinProcessed && (currentTime - coinDetectedTime < 2000)) {
    doc["value"] = coinValue;
    doc["timestamp"] = coinDetectedTime;
    coinProcessed = true; // Mark as read to prevent duplicate
  } else {
    doc["value"] = 0.0;
  }
  
  String response;
  serializeJson(doc, response);
  Serial.println(response);
  
  // Clear coin value after 3 seconds to be ready for next coin
  if (coinValue > 0 && (currentTime - coinDetectedTime > 3000)) {
    coinValue = 0.0;
    coinPulseCount = 0;
    coinProcessed = false;
  }
}

void coinInterrupt() {
  unsigned long currentTime = millis();
  
  // Debounce: ignore pulses within 10ms of each other (noise)
  if (currentTime - lastCoinPulseTime < 10) {
    return;
  }
  
  coinPulseCount++;
  lastCoinPulseTime = currentTime;
  coinDetectedTime = currentTime;
}

void processCoinPulse() {
  unsigned long currentTime = millis();
  
  // Only process if we have pulses AND enough time passed since last pulse
  // Wait 300ms after last pulse to ensure pulse train is complete
  if (coinPulseCount > 0 && (currentTime - coinDetectedTime > 300)) {
    
    int pulses = coinPulseCount;
    float detectedValue = 0.0;
    
    // Different pulse counts for different denominations
    // Adjust based on your coin acceptor configuration
    // Using ranges to handle slight variations in pulse counting
    if (pulses == 1) {
      detectedValue = 1.0;  // 1 Peso
    } else if (pulses >= 2 && pulses <= 6) {
      detectedValue = 5.0;  // 5 Pesos (typically 5 pulses)
    } else if (pulses >= 7 && pulses <= 12) {
      detectedValue = 10.0; // 10 Pesos (typically 10 pulses)
    } else if (pulses >= 13 && pulses <= 22) {
      detectedValue = 20.0; // 20 Pesos (typically 20 pulses)
    }
    
    if (detectedValue > 0) {
      coinValue = detectedValue;
      
      // Send notification to serial with timestamp
      StaticJsonDocument<128> doc;
      doc["coinDetected"] = coinValue;
      doc["pulses"] = pulses;
      doc["timestamp"] = currentTime;
      
      String response;
      serializeJson(doc, response);
      Serial.println(response);
      
      coinDetectedTime = currentTime;
      coinProcessed = false; // Ready to be read by API
    } else {
      // Unknown pulse count - log for debugging
      StaticJsonDocument<128> doc;
      doc["warning"] = "Unknown pulse count";
      doc["pulses"] = pulses;
      doc["timestamp"] = currentTime;
      
      String response;
      serializeJson(doc, response);
      Serial.println(response);
    }
    
    // Reset counter
    coinPulseCount = 0;
  }
}

void handleUnlockTemp(JsonObject data) {
  int slot = data["slot"];
  
  int solenoidPin = getSolenoidPin(slot);
  if (solenoidPin == UNUSED_PIN) {
    sendResponse(false, "Slot does not support temporary unlock");
    return;
  }

  // Send unlock signal for 2 seconds using configuration constants
  digitalWrite(solenoidPin, SOLENOID_UNLOCKED);
  delay(2000);  // Hold unlock for 2 seconds
  digitalWrite(solenoidPin, SOLENOID_LOCKED); // Re-lock
  sendResponse(true, "Temporary unlock completed");
}

void handleFingerprintDelete(JsonObject data) {
  int fingerprintId = data["fingerprintId"];
  
  Serial.println("{\"status\":\"Deleting fingerprint from AS608 sensor...\"}");
  
  // Delete fingerprint from AS608 sensor memory
  uint8_t p = finger.deleteModel(fingerprintId);
  
  if (p == FINGERPRINT_OK) {
    // Success!
    StaticJsonDocument<100> doc;
    doc["success"] = true;
    doc["message"] = "Fingerprint deleted successfully";
    doc["fingerprintId"] = fingerprintId;
    
    String response;
    serializeJson(doc, response);
    Serial.println(response);
  } else if (p == FINGERPRINT_DELETEFAIL) {
    sendResponse(false, "Failed to delete fingerprint");
  } else {
    sendResponse(false, "Error deleting fingerprint from sensor");
  }
}

void sendResponse(bool success, const char* message) {
  StaticJsonDocument<100> doc;
  doc["success"] = success;
  doc["message"] = message;
  
  String response;
  serializeJson(doc, response);
  Serial.println(response);
}