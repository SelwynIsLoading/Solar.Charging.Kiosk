/*
 * Test Coin Slot Detection - Simplified Test Sketch
 * Use this to debug coin acceptor issues
 */

const int COIN_PIN = 2;

volatile int coinPulseCount = 0;
unsigned long lastPulseTime = 0;
float coinValue = 0.0;

void setup() {
  Serial.begin(9600);
  
  pinMode(COIN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(COIN_PIN), coinInterrupt, FALLING);
  
  Serial.println("=== Coin Slot Test Started ===");
  Serial.println("Insert coins and watch the output...");
  Serial.println("");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Show pulse count in real-time
  static int lastPulseCount = 0;
  if (coinPulseCount != lastPulseCount) {
    Serial.print("Pulse count: ");
    Serial.println(coinPulseCount);
    lastPulseCount = coinPulseCount;
  }
  
  // Process coin after 200ms of no new pulses
  if (coinPulseCount > 0 && (currentTime - lastPulseTime > 200)) {
    processCoin();
  }
  
  delay(10);
}

void coinInterrupt() {
  coinPulseCount++;
  lastPulseTime = millis();
  
  // Debug: Blink built-in LED on each pulse
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void processCoin() {
  int pulses = coinPulseCount;
  
  Serial.println("");
  Serial.println("==================");
  Serial.print("Total Pulses: ");
  Serial.println(pulses);
  
  // Determine coin value
  if (pulses == 1) {
    coinValue = 1.0;
  } else if (pulses >= 2 && pulses <= 6) {
    coinValue = 5.0;
  } else if (pulses >= 7 && pulses <= 12) {
    coinValue = 10.0;
  } else if (pulses >= 13 && pulses <= 22) {
    coinValue = 20.0;
  } else {
    Serial.println("⚠ Unknown pulse count!");
    coinValue = 0.0;
  }
  
  if (coinValue > 0) {
    Serial.print("Coin Detected: ₱");
    Serial.println(coinValue, 2);
  }
  
  Serial.println("==================");
  Serial.println("");
  
  // Reset
  coinPulseCount = 0;
}

