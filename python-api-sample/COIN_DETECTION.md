# Real-Time Coin Detection Guide

Complete guide for setting up real-time coin detection with the coin acceptor.

## How It Works

### System Flow

```
Physical Coin Inserted
    ↓
Coin Acceptor Sends Pulses
    ↓
Arduino Pin 2 (Interrupt)
    ↓
Count Pulses (1, 5, 10, or 20)
    ↓
Determine Coin Value
    ↓
Store in coinValue variable
    ↓
UI Polls Every 500ms
    ↓
Python API Requests Coin Value
    ↓
Arduino Returns Value (if detected)
    ↓
UI Updates Instantly!
    ↓
Show Notification + Update Display
```

---

## Arduino Coin Detection (Enhanced)

### Key Improvements

**1. Prevents Missed Coins**
```cpp
// Old: Cleared immediately after reading
coinValue = 0.0;  // ❌ Lost if UI didn't poll at exact moment

// New: Keeps value for 2 seconds, allows one read
if (coinValue > 0 && !coinProcessed && (currentTime - coinDetectedTime < 2000)) {
    doc["value"] = coinValue;
    coinProcessed = true; // ✅ Prevents duplicate reads
}
```

**2. Debouncing**
```cpp
// Wait 200ms after last pulse before processing
if (coinPulseCount > 0 && (currentTime - coinDetectedTime > 200)) {
    // Process coin value
}
```

**3. Pulse Range Tolerance**
```cpp
// Old: Exact match required
if (coinPulseCount == 5) coinValue = 5.0;

// New: Range-based (handles noise)
if (coinPulseCount >= 2 && coinPulseCount <= 6) {
    coinValue = 5.0;  // ✅ More reliable
}
```

**4. Auto-Clear**
```cpp
// Clear after 3 seconds to be ready for next coin
if (coinValue > 0 && (currentTime - coinDetectedTime > 3000)) {
    coinValue = 0.0;
    coinProcessed = false;
}
```

---

## Python API Enhancements

### Coin Slot Endpoint

```python
@app.route('/api/coin-slot', methods=['GET'])
def get_coin_value():
    """
    Called by UI every 500ms for real-time detection
    Returns: { "value": coin_amount, "timestamp": detection_time }
    """
    result = send_arduino_command('READ_COIN', {})
    coin_value = result.get('value', 0)
    
    # Log only when coin detected (avoid spam in console)
    if coin_value > 0:
        print(f"💰 Coin detected: ₱{coin_value:.2f}")
    
    return jsonify({
        'value': coin_value,
        'timestamp': result.get('timestamp', 0)
    }), 200
```

**Benefits:**
- ✅ No console spam (only logs when coin detected)
- ✅ Returns timestamp for tracking
- ✅ Handles simulation mode gracefully

---

## Coin Acceptor Configuration

### Pulse-to-Value Mapping

Most coin acceptors use pulse output:

| Coin | Pulses | Arduino Range | Value |
|------|--------|---------------|-------|
| ₱1 | 1 | 1 | 1.00 |
| ₱5 | 5 | 2-6 | 5.00 |
| ₱10 | 10 | 7-12 | 10.00 |
| ₱20 | 20 | 18-22 | 20.00 |

**Range tolerance helps with:**
- Electrical noise
- Coin acceptor variations
- Worn coins
- Fast insertion

### Adjusting for Your Coin Acceptor

Edit in Arduino code if your acceptor uses different pulses:

```cpp
void processCoinPulse() {
    if (coinPulseCount == 1) {
        coinValue = 1.0;  // Adjust value
    } else if (coinPulseCount >= 4 && coinPulseCount <= 6) {
        coinValue = 5.0;  // Adjust range
    }
    // ... more denominations
}
```

---

## Testing Real-Time Detection

### Test 1: Arduino Serial Monitor

```cpp
// Open Serial Monitor (9600 baud)
// Insert coin physically
// Should see:
{"coinDetected":"₱5.00"}
```

### Test 2: Python API Console

```bash
python app.py

# When coin inserted:
💰 Coin detected: ₱5.00
```

### Test 3: UI Real-Time

```
1. Open slot in browser
2. Coin listener auto-starts
3. Insert physical coin
4. Within 500ms, UI shows:
   - Notification: "💰 ₱5.00 inserted! +30 minutes"
   - Amount updates: ₱0 → ₱5
   - Time updates: 0 min → 30 min
```

### Test 4: Multiple Coins

```
Insert ₱5 → Wait 1s → Insert ₱10 → Wait 1s → Insert ₱5

Expected UI:
- ₱5.00 inserted! +30 minutes (Total: ₱5, 30 min)
- ₱10.00 inserted! +60 minutes (Total: ₱15, 90 min)
- ₱5.00 inserted! +30 minutes (Total: ₱20, 120 min)
```

---

## Troubleshooting

### Coin Not Detected

**Check:**
1. Coin acceptor power (12V connected?)
2. Signal wire connected to Pin 2
3. Ground is common between Arduino and acceptor
4. Coin acceptor is enabled (some have inhibit pin)

**Test:**
```cpp
// Add to Arduino setup()
pinMode(COIN_PIN, INPUT_PULLUP);
Serial.print("Coin pin state: ");
Serial.println(digitalRead(COIN_PIN));
```

### Pulses Not Counted

**Check:**
```cpp
// In loop(), monitor interrupt
void loop() {
    if (coinPulseCount > 0) {
        Serial.print("Pulses: ");
        Serial.println(coinPulseCount);
    }
}
```

### Wrong Coin Value

**Adjust pulse ranges:**
```cpp
// If ₱5 coins trigger as ₱10:
if (coinPulseCount >= 4 && coinPulseCount <= 6) {  // Narrow range
    coinValue = 5.0;
}
```

### UI Not Updating

**Check:**
1. Python API is running
2. Blazor app is running
3. Coin listener is active (green "Listening" badge)
4. Check browser console for errors
5. Check Python console for coin detection logs

**Debug in Python:**
```python
# Add to get_coin_value()
print(f"Polling coin slot... Value: {coin_value}")
```

### Duplicate Detections

**Arduino has protection:**
```cpp
coinProcessed = true; // First read marks as processed
// Second poll within 2 seconds returns 0
```

---

## Timing Configuration

### Current Settings

| Setting | Value | Adjustable? |
|---------|-------|-------------|
| UI Poll Rate | 500ms | Yes (SlotControl.razor) |
| Coin Hold Time | 2 seconds | Yes (Arduino) |
| Auto-Clear Time | 3 seconds | Yes (Arduino) |
| Pulse Debounce | 200ms | Yes (Arduino) |

### Adjusting Poll Rate

**For faster response (100ms):**
```csharp
// In SlotControl.razor
}, null, 0, 100); // Check every 100ms (10 times per second)
```

**For less API calls (1000ms):**
```csharp
}, null, 0, 1000); // Check every 1 second
```

**Recommended: 500ms** (good balance)

### Adjusting Coin Hold Time

```cpp
// In Arduino handleReadCoin()
if (coinValue > 0 && !coinProcessed && (currentTime - coinDetectedTime < 2000)) {
    //                                                                    ↑
    //                                                        Change this (milliseconds)
```

---

## Advanced Features

### Add Coin History in Python

```python
# Store recent coin insertions
coin_history = []

@app.route('/api/coin-slot', methods=['GET'])
def get_coin_value():
    global coin_history
    
    result = send_arduino_command('READ_COIN', {})
    coin_value = result.get('value', 0)
    
    if coin_value > 0:
        coin_history.append({
            'value': coin_value,
            'time': time.time()
        })
        print(f"💰 Coin detected: ₱{coin_value:.2f}")
    
    return jsonify({
        'value': coin_value,
        'history': coin_history[-10:]  # Last 10 coins
    }), 200
```

### Add Sound Notification in UI

```csharp
// In SlotControl.razor
@inject IJSRuntime JS

private async Task PlayCoinSound()
{
    await JS.InvokeVoidAsync("playSound", "coin-drop.mp3");
}

// Call when coin detected:
await PlayCoinSound();
```

### Add Vibration Feedback (Mobile)

```javascript
// In wwwroot/app.js
function vibrateOnCoin() {
    if (navigator.vibrate) {
        navigator.vibrate(200); // Vibrate for 200ms
    }
}
```

---

## Production Recommendations

### For Kiosk Deployment

1. **Enable coin listener on page load** ✅ (Already implemented)
2. **Auto-stop after 60 seconds of no activity**
3. **Add coin insertion timeout (e.g., 2 minutes max)**
4. **Log all coin insertions to database**
5. **Add "Cancel" button to refund if user leaves**

### Security

1. **Validate coin values** (don't trust client)
2. **Rate limit API** (prevent spam)
3. **Log suspicious activity** (rapid polling, etc.)
4. **Add coin acceptor tamper detection**

### User Experience

1. **Visual**: Amount display animates on coin insert ✅
2. **Audio**: Play sound on coin detection
3. **Haptic**: Vibrate on mobile devices
4. **Status**: Show "Listening" indicator ✅
5. **Help**: Show coin denominations and rates ✅

---

## Quick Test Commands

### Test Arduino Coin Detection

```cpp
// Upload sketch
// Open Serial Monitor
// Insert coin
// Should see:
{"coinDetected":"₱5.00"}

// Then send:
{"command":"READ_COIN","data":{}}

// Should receive:
{"success":true,"value":5.0,"timestamp":12345}
```

### Test Python API

```bash
# Terminal 1: Start API
python app.py

# Terminal 2: Poll coin slot
while true; do curl http://localhost:5000/api/coin-slot; echo ""; sleep 0.5; done

# Insert coin - should see value appear in responses
```

### Test Complete Flow

```bash
# 1. Start Python API
python app.py

# 2. Start Blazor (another terminal)
dotnet run

# 3. Open http://localhost:5000/slot/4
# 4. Click "Activate Coin Acceptor"
# 5. Insert physical coin
# 6. Watch UI update in real-time!
```

---

## Summary of Changes

### Arduino (`arduino_sketch.ino`)

✅ **Added:**
- `coinDetectedTime` - Tracks when coin was detected
- `coinProcessed` - Prevents duplicate reads
- Pulse range tolerance (2-6 for ₱5 instead of exact 5)
- 200ms debouncing
- 2-second read window
- 3-second auto-clear

### Python API (`app.py`)

✅ **Enhanced:**
- Logs only when coin detected (no spam)
- Returns timestamp with value
- Better simulation handling

### UI (`SlotControl.razor`)

✅ **Implemented:**
- Real-time polling (500ms)
- Auto-start coin listener
- Start/Stop buttons
- Visual "Listening" indicator
- Instant UI updates
- Animated amount display
- Toast notifications for each coin

---

**The system now supports true real-time coin detection!** 🎉

Every coin inserted is detected within 500ms and displayed to the user immediately.

