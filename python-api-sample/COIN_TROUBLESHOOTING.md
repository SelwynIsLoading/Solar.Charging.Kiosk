# Coin Slot Troubleshooting Guide

## Quick Diagnosis

### Test 1: Check Physical Connections

**Coin Acceptor Wiring:**
```
Coin Acceptor    →    Arduino Mega
───────────────────────────────────
+12V (Red)       →    12V Power Supply
GND (Black)      →    GND (Common Ground)
COIN (Blue)      →    Pin 2
```

**Verify:**
```bash
# Check connections are secure
# Verify 12V power is on
# Measure voltage at coin acceptor (should be ~12V)
```

### Test 2: Upload Simple Test Sketch

Upload `test_coin_slot.ino` (I just created it):

```cpp
// This sketch:
// - Shows real-time pulse counts
// - Blinks LED on each pulse
// - Prints coin value after 200ms
// - Helps diagnose issues
```

**What you should see:**

```
=== Coin Slot Test Started ===
Insert coins and watch the output...

[Insert ₱5 coin]

Pulse count: 1
Pulse count: 2
Pulse count: 3
Pulse count: 4
Pulse count: 5

==================
Total Pulses: 5
Coin Detected: ₱5.00
==================
```

### Test 3: Check Interrupt

Add this to your main sketch temporarily:

```cpp
void setup() {
  // ... existing code ...
  
  // Test interrupt
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.println("Testing coin interrupt...");
  Serial.print("Pin 2 state: ");
  Serial.println(digitalRead(COIN_PIN));
}

void coinInterrupt() {
  coinPulseCount++;
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Blink LED
  
  // Debug output
  Serial.print("!");  // Print ! for each pulse
}
```

**Expected:** LED blinks and `!` appears each pulse

---

## Common Issues and Fixes

### Issue 1: No Pulses Detected

**Symptoms:**
- Insert coin, nothing happens
- `coinPulseCount` stays at 0

**Possible Causes:**

#### A. Coin Acceptor Not Powered
```bash
# Check:
- 12V connected to coin acceptor?
- Measure voltage with multimeter
- Coin acceptor LED should be on
```

#### B. Signal Wire Not Connected
```bash
# Check:
- Blue wire (COIN/SIGNAL) connected to Pin 2?
- Connection is secure?
- No broken wires?
```

#### C. Common Ground Missing
```bash
# CRITICAL: Arduino GND must connect to coin acceptor GND
# Without common ground, signal won't work!

Solution:
- Connect Arduino GND to coin acceptor GND
- Connect to 12V PSU GND as well
```

#### D. Wrong Interrupt Pin
```cpp
// Pin 2 is correct for interrupt 0
// Verify in code:
attachInterrupt(digitalPinToInterrupt(COIN_PIN), coinInterrupt, FALLING);
//                                    ↑ Should be 2
```

#### E. Coin Acceptor Disabled/Inhibited
```bash
# Some coin acceptors have INHIBIT pin
# Must be pulled LOW to enable
# Check coin acceptor datasheet

If your acceptor has INHIBIT pin:
const int INHIBIT_PIN = 3;
pinMode(INHIBIT_PIN, OUTPUT);
digitalWrite(INHIBIT_PIN, LOW);  // Enable coin acceptor
```

### Issue 2: Wrong Pulse Count

**Symptoms:**
- Insert ₱5, get ₱10
- Inconsistent readings

**Solutions:**

#### A. Adjust Pulse Ranges
```cpp
// Your coin acceptor might use different pulse counts
// Test and adjust:

if (pulses == 1) coinValue = 1.0;
else if (pulses >= 3 && pulses <= 7) coinValue = 5.0;   // Adjust range
else if (pulses >= 8 && pulses <= 15) coinValue = 10.0; // Adjust range
```

#### B. Increase Debounce Time
```cpp
// If getting extra pulses from noise:
if (coinPulseCount > 0 && (currentTime - coinDetectedTime > 300)) { // Increase to 300ms
    processCoinPulse();
}
```

#### C. Check Coin Acceptor Settings
```bash
# Some acceptors have DIP switches for pulse mode
# Check manual for your specific model
```

### Issue 3: Multiple Readings of Same Coin

**Symptoms:**
- Insert ₱5, UI shows ₱10 or ₱15
- Same coin counted multiple times

**This is FIXED in the updated code:**

```cpp
// Prevents duplicate reads
if (coinValue > 0 && !coinProcessed && (currentTime - coinDetectedTime < 2000)) {
    doc["value"] = coinValue;
    coinProcessed = true; // ✅ Won't return again
}
```

### Issue 4: Coin Not Detected by UI

**Symptoms:**
- Arduino detects coin (Serial shows it)
- UI doesn't update

**Check:**

#### A. Python API Running?
```bash
python app.py

# Should show:
Starting Solar Charging Station Python API...
```

#### B. Arduino Connected to Python?
```python
# In app.py, check:
try:
    arduino = serial.Serial(ARDUINO_PORT, BAUD_RATE, timeout=1)
    print(f"Connected to Arduino on {ARDUINO_PORT}")
except:
    print("Arduino not connected - simulation mode")
    arduino = None
```

#### C. Correct COM Port?
```python
# Windows
ARDUINO_PORT = 'COM3'  # Check Device Manager

# Raspberry Pi
ARDUINO_PORT = '/dev/ttyACM0'  # or /dev/ttyUSB0

# Find port on Raspberry Pi:
ls /dev/tty*
```

#### D. Coin Listener Active in UI?
```
- Should see green "Listening" badge
- If not, click "Activate Coin Acceptor"
```

---

## Diagnostic Steps

### Step 1: Test Arduino Alone

**Upload test_coin_slot.ino:**
```cpp
1. Upload sketch
2. Open Serial Monitor (9600 baud)
3. Insert coin
4. Should see pulse counts and coin value
```

**Expected:**
```
Pulse count: 1
Pulse count: 2
...
Pulse count: 5
Total Pulses: 5
Coin Detected: ₱5.00
```

### Step 2: Test with Python API

**Check Arduino output in Python console:**

```bash
python app.py

# When coin inserted, should see:
→ Sending to Arduino: {"command": "READ_COIN", "data": {}}
← Received from Arduino: {"success":true,"value":5.0}
💰 Coin detected: ₱5.00
```

### Step 3: Test UI Polling

**Check browser console (F12):**

```javascript
// Should see network requests every 500ms:
GET http://localhost:5000/api/coin-slot
Response: {"value": 5.0, "timestamp": 12345}
```

---

## Quick Fixes

### Fix 1: Add Debug Output to Arduino

```cpp
void coinInterrupt() {
  coinPulseCount++;
  coinDetectedTime = millis();
  
  // Debug: Print each pulse
  Serial.print("P");  // P = Pulse
}

void processCoinPulse() {
  if (coinPulseCount > 0) {
    unsigned long currentTime = millis();
    unsigned long timeSinceLastPulse = currentTime - coinDetectedTime;
    
    // Debug output
    Serial.println();
    Serial.print("Pulses: ");
    Serial.print(coinPulseCount);
    Serial.print(" | Time since last: ");
    Serial.print(timeSinceLastPulse);
    Serial.println("ms");
    
    // Process only after debounce time
    if (timeSinceLastPulse > 200) {
      // ... rest of processing
    }
  }
}
```

### Fix 2: Test Pin Directly

```cpp
void loop() {
  // Read pin state directly
  static int lastState = HIGH;
  int currentState = digitalRead(COIN_PIN);
  
  if (currentState != lastState) {
    Serial.print("Pin 2 changed to: ");
    Serial.println(currentState == HIGH ? "HIGH" : "LOW");
    lastState = currentState;
  }
  
  // ... rest of loop
}
```

### Fix 3: Enable Internal Pull-up

```cpp
void setup() {
  // Make sure pull-up is enabled
  pinMode(COIN_PIN, INPUT_PULLUP);
  digitalWrite(COIN_PIN, HIGH);  // Ensure pull-up is active
  
  // ... rest of setup
}
```

---

## Hardware Checklist

**Before debugging software:**

- [ ] Coin acceptor has 12V power
- [ ] Coin acceptor GND connected to Arduino GND
- [ ] COIN signal wire connected to Arduino Pin 2
- [ ] Pin 2 configured as INPUT_PULLUP
- [ ] Interrupt attached to Pin 2
- [ ] Common ground between all components
- [ ] Coin acceptor is enabled (check INHIBIT pin if exists)
- [ ] Test with known-good coin

---

## Coin Acceptor Types

### Type 1: Pulse Output (Most Common)

Your current code is for this type:
- Sends pulses on signal line
- Number of pulses = coin value
- Example: 5 pulses = ₱5

**Configuration:**
```cpp
attachInterrupt(digitalPinToInterrupt(COIN_PIN), coinInterrupt, FALLING);
```

### Type 2: Serial Output

Some acceptors send serial data:
- Requires serial communication
- Different code needed

### Type 3: Parallel Output

Binary coded output on multiple pins:
- Uses multiple pins
- Different code needed

**Make sure you have Type 1 (pulse-based)!**

---

## Manual Testing Commands

### Test via Serial Monitor

```json
// Send this command:
{"command":"READ_COIN","data":{}}

// Should receive:
{"success":true,"value":5.0,"timestamp":12345}
```

### Test via curl (if Python API running)

```bash
# Poll manually
curl http://localhost:5000/api/coin-slot

# Response:
{"value": 0, "timestamp": 0}  # No coin

# After inserting coin:
{"value": 5.0, "timestamp": 12345}
```

---

## Recommended Test Procedure

**1. Hardware Test:**
```
- Connect coin acceptor
- Power on (12V)
- Insert coin
- Listen for mechanical acceptance sound
- Check if coin is accepted or rejected
```

**2. Signal Test:**
```
- Upload test_coin_slot.ino
- Open Serial Monitor
- Insert coin
- Watch for pulse counts
- Verify coin value calculation
```

**3. Integration Test:**
```
- Upload main arduino_sketch.ino
- Start Python API
- Check Python console for coin detection
- Insert coin
- Verify "💰 Coin detected" message appears
```

**4. UI Test:**
```
- Start Blazor app
- Open slot page
- Activate coin listener
- Insert coin
- Verify UI updates within 500ms
```

---

## Alternative: Simplified Coin Detection

If complex version doesn't work, try simpler approach:

```cpp
volatile bool coinDetected = false;
volatile unsigned long coinTime = 0;

void coinInterrupt() {
  // Simple: Just detect ANY coin as ₱5
  coinDetected = true;
  coinTime = millis();
}

void loop() {
  if (coinDetected && (millis() - coinTime > 100)) {
    Serial.println("{\"coinDetected\":5.0}");
    coinValue = 5.0;
    coinDetected = false;
  }
}
```

This treats any coin insertion as ₱5 (good for testing).

---

## Contact Support

If still not working after these steps, provide:

1. **Coin acceptor model/brand**
2. **Serial Monitor output** when coin inserted
3. **Photo of wiring**
4. **Multimeter readings** (12V, GND, signal)
5. **Python API console output**

---

**Most common issue: Missing common ground between Arduino and coin acceptor!** ⚡

