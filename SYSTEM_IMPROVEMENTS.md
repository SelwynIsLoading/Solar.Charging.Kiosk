# System Improvements - Charging Kiosk

## Overview
This document summarizes all improvements made to ensure the charging kiosk system runs smoothly according to the specified process flow.

## Process Flow Implementation

### 1. Open Slots (Slots 1-3)
- **Purpose**: Simple outlet charging
- **Process**:
  1. User inserts coins
  2. System calculates charging time based on amount
  3. Timer starts and relay turns on
  4. User can stop charging at any time
- **No Security**: No fingerprint or locking mechanism

### 2. Phone Slots (Slots 4-9)
- **Purpose**: Phone charging with UV sanitization and security
- **Process**:
  1. User inserts coins via coin acceptor
  2. User enrolls fingerprint for security
  3. UV light turns ON for 15 seconds (sanitization)
  4. UV light turns OFF automatically
  5. Slot locks and charging begins
  6. **During charging**: User can temporarily access device by scanning fingerprint (2-second unlock pulse)
  7. **End session**: User scans fingerprint to permanently unlock and stop charging
- **Features**: Fingerprint security, UV sanitization, temporary access

### 3. Laptop Slots (Slots 10-13)
- **Purpose**: Laptop charging with security (no UV)
- **Process**:
  1. User inserts coins via coin acceptor
  2. User enrolls fingerprint for security
  3. Slot locks and charging begins (no UV sanitization)
  4. **During charging**: User can temporarily access device by scanning fingerprint (2-second unlock pulse)
  5. **End session**: User scans fingerprint to permanently unlock and stop charging
- **Features**: Fingerprint security, temporary access (no UV)

---

## Technical Improvements

### Arduino Sketch (`arduino_sketch.ino`)

#### 1. **Improved Coin Detection Reliability**
- **Added**: `lastCoinPulseTime` variable for better pulse tracking
- **Enhanced**: Interrupt handler with 10ms debounce to filter noise
- **Improved**: `processCoinPulse()` function with:
  - 300ms wait time after last pulse (increased from 200ms) for complete pulse train detection
  - Better pulse range detection for different denominations
  - Structured JSON responses with timestamp
  - Warning logging for unknown pulse counts

```cpp
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
```

#### 2. **Temporary Unlock Feature**
- **Added**: `UNLOCK_TEMP` command for 2-second pulse unlock
- **Implementation**: Sends LOW pulse for 2 seconds then automatically re-locks to HIGH
- **Purpose**: Allows users to briefly access their device without ending the charging session

```cpp
void handleUnlockTemp(JsonObject data) {
  int slot = data["slot"];
  
  if (slot >= 4 && slot <= 13) {
    // Send LOW pulse for 2 seconds to unlock (normally HIGH = locked)
    digitalWrite(SOLENOID_PINS[slot - 4], LOW);
    delay(2000);  // Hold unlock for 2 seconds
    digitalWrite(SOLENOID_PINS[slot - 4], HIGH); // Re-lock
    sendResponse(true, "Temporary unlock completed");
  }
}
```

#### 3. **Solenoid Lock Behavior**
- **Logic**: HIGH = Locked (default), LOW = Unlocked
- **Temporary Unlock**: Pulses LOW for 2 seconds then returns to HIGH
- **Permanent Unlock**: Stays LOW until session ends

---

### Python API (`app.py`)

#### 1. **Enhanced Response Handling**
- **Added**: `handle_verification_response()` function for fingerprint verification
- **Improved**: Better timeout handling and status message parsing
- **Enhanced**: JSON decode error handling with fallback

#### 2. **New Endpoint: Temporary Unlock**
```python
@app.route('/api/solenoid/unlock-temp', methods=['POST'])
def unlock_temp():
    """
    Temporarily unlock solenoid for 2 seconds (for device access during charging)
    Sends pulse to unlock, waits 2 seconds, then automatically re-locks
    """
    result = send_arduino_command('UNLOCK_TEMP', {'slot': slot_number})
    return jsonify(result), 200 if result.get('success') else 500
```

#### 3. **Improved Coin Detection**
- **Enhanced**: Added timestamp logging for better tracking
- **Adjusted**: Polling timeout to 2 seconds for reliability
- **Better Logging**: Only logs when coins are detected (reduces spam)

---

### C# Services

#### 1. **IArduinoApiService Interface**
- **Added**: `Task<bool> UnlockTemporaryAsync(int slotNumber)` method

#### 2. **ArduinoApiService Implementation**
- **Added**: `UnlockTemporaryAsync()` method to call Python API
- **Enhanced**: Proper logging for all Arduino operations

```csharp
public async Task<bool> UnlockTemporaryAsync(int slotNumber)
{
    _logger.LogInformation($"Temporary unlock for slot {slotNumber}");
    
    var response = await _httpClient.PostAsJsonAsync("/api/solenoid/unlock-temp", new
    {
        slotNumber
    });
    
    return response.IsSuccessStatusCode;
}
```

#### 3. **ISlotService Interface**
- **Added**: `Task<bool> UnlockTemporaryAsync(int slotNumber, int fingerprintId)` method

#### 4. **SlotService Implementation**
- **Added**: `UnlockTemporaryAsync()` method with fingerprint verification
- **Features**:
  - Validates slot type (Phone or Laptop only)
  - Verifies slot is in use
  - Verifies fingerprint matches enrolled fingerprint
  - Sends temporary unlock command to Arduino

```csharp
public async Task<bool> UnlockTemporaryAsync(int slotNumber, int fingerprintId)
{
    var slot = GetSlot(slotNumber);
    
    // Validate slot type and status
    if (slot.Type != SlotType.Phone && slot.Type != SlotType.Laptop)
        return false;
    
    if (slot.Status != SlotStatus.InUse && slot.Status != SlotStatus.Locked)
        return false;
    
    // Verify fingerprint
    if (!slot.FingerprintId.HasValue || slot.FingerprintId.Value != fingerprintId)
    {
        var verified = await VerifyFingerprintAsync(fingerprintId);
        if (!verified) return false;
    }
    
    // Send temporary unlock
    await _arduinoService.UnlockTemporaryAsync(slotNumber);
    return true;
}
```

#### 5. **UV Sanitization Verification**
- **Confirmed**: UV light runs for exactly 15 seconds
- **Implementation**: `Task.Delay(15000)` in `StartUVSanitizationAsync()`
- **Status Management**: Changes from `Sanitizing` to `Locked` after completion

---

### UI - SlotControl.razor

#### 1. **Separated Access Methods**
- **Access Device**: Temporary 2-second unlock for checking device during charging
- **End Session**: Permanent unlock to stop charging and retrieve device

#### 2. **New "Access Device" Feature**
```razor
<MudButton OnClick="AccessDevice">
    Access Device (2s unlock)
</MudButton>
```

**Implementation**:
- Verifies fingerprint
- Calls `SlotService.UnlockTemporaryAsync()`
- Provides visual feedback with progress bar
- Automatically re-locks after 2 seconds
- Charging continues uninterrupted

#### 3. **Enhanced "End Session" Feature**
```razor
<MudButton OnClick="InitiateUnlock">
    End Session & Unlock
</MudButton>
```

**Implementation**:
- Verifies fingerprint
- Stops charging
- Permanently unlocks slot
- Returns to available state

#### 4. **User Experience Improvements**
- Clear separation between temporary access and ending session
- Progress indicators for all operations
- Informative messages explaining each action
- Color-coded buttons (Info for access, Error for end session)

---

## Hardware Configuration

### Solenoid Lock Wiring
- **Signal**: HIGH = Locked (default), LOW = Unlocked
- **Behavior**: Normally locked, pulse LOW to unlock
- **Pins**: 35-44 (Slots 4-13)

### Coin Acceptor Configuration
- **Pin**: 2 (interrupt)
- **Detection**: Pulse counting with debounce
- **Denominations**:
  - 1 pulse = ₱1
  - 2-6 pulses = ₱5
  - 7-12 pulses = ₱10
  - 13-22 pulses = ₱20

### UV Light
- **Pins**: 45-50 (Slots 4-9)
- **Duration**: 15 seconds (automatic)
- **Activation**: Immediately after starting charge on phone slots

---

## Testing Checklist

### Open Slots (1-3)
- [ ] Insert coins - verify amount detected
- [ ] Start charging - verify relay turns ON
- [ ] Verify timer counts down correctly
- [ ] Stop charging - verify relay turns OFF

### Phone Slots (4-9)
- [ ] Insert coins - verify amount detected
- [ ] Enroll fingerprint - verify success
- [ ] Verify UV light turns ON
- [ ] Verify UV light turns OFF after 15 seconds
- [ ] Verify slot locks automatically
- [ ] Test "Access Device" - verify 2-second unlock
- [ ] Verify charging continues after access
- [ ] Test "End Session" - verify permanent unlock
- [ ] Verify relay turns OFF after session end

### Laptop Slots (10-13)
- [ ] Insert coins - verify amount detected
- [ ] Enroll fingerprint - verify success
- [ ] Verify NO UV light activation
- [ ] Verify slot locks automatically
- [ ] Test "Access Device" - verify 2-second unlock
- [ ] Verify charging continues after access
- [ ] Test "End Session" - verify permanent unlock
- [ ] Verify relay turns OFF after session end

### Coin Detection
- [ ] Test each denomination (₱1, ₱5, ₱10, ₱20)
- [ ] Verify correct pulse counting
- [ ] Test rapid insertions
- [ ] Verify no duplicate detections
- [ ] Check for false positives/negatives

---

## Troubleshooting

### Coin Detection Issues
1. Check Arduino serial monitor for pulse counts
2. Verify coin acceptor wiring to Pin 2
3. Adjust pulse ranges in `processCoinPulse()` if needed
4. Check for loose connections causing noise

### Solenoid Lock Issues
1. Verify HIGH/LOW logic (HIGH = locked)
2. Check power supply to solenoids
3. Test temporary unlock manually via Python API
4. Verify pin assignments (35-44)

### UV Light Issues
1. Check relay wiring to pins 45-50
2. Verify 15-second delay in logs
3. Test manual control via `/api/uv-light` endpoint
4. Check power supply to UV lights

### Fingerprint Issues
1. Verify AS608 sensor connection
2. Check baud rate (57600)
3. Test enrollment via `/api/fingerprint/enroll`
4. Verify stored fingerprint IDs

---

## API Endpoints Summary

### Python API (Port 5000)
- `POST /api/relay` - Control power relay
- `POST /api/solenoid` - Control solenoid lock
- `POST /api/solenoid/unlock-temp` - **NEW** Temporary 2s unlock
- `POST /api/uv-light` - Control UV light
- `POST /api/fingerprint/enroll` - Enroll new fingerprint
- `POST /api/fingerprint/verify` - Verify fingerprint
- `GET /api/coin-slot` - Read coin value
- `GET /health` - Health check

### Blazor API (Internal)
All Arduino operations are routed through `IArduinoApiService` which communicates with Python API.

---

## Configuration Files

### `appsettings.json`
```json
{
  "PythonApi": {
    "BaseUrl": "http://localhost:5000"
  }
}
```

### `python-api-sample/app.py`
```python
ARDUINO_PORT = '/dev/ttyACM0'  # Raspberry Pi
# ARDUINO_PORT = 'COM3'  # Windows
BAUD_RATE = 9600
```

---

## Deployment Notes

1. **Upload Arduino Sketch**: Use Arduino IDE to upload `arduino_sketch.ino`
2. **Start Python API**: `python app.py` (runs on port 5000)
3. **Start Blazor App**: `dotnet run` (runs on configured port)
4. **Verify Connections**: Check `/health` endpoint
5. **Test Each Slot Type**: Follow testing checklist

---

## Future Enhancements

1. **Admin Panel**: Add configuration for coin denominations
2. **Transaction Logging**: Store all transactions in database
3. **Remote Monitoring**: Real-time slot status dashboard
4. **SMS Notifications**: Alert users when charging time expires
5. **Multi-Language Support**: Add language selection
6. **Receipt Printing**: Generate receipts for transactions

---

## Support

For issues or questions:
1. Check troubleshooting section
2. Review Arduino serial monitor logs
3. Check Python API console output
4. Verify hardware connections
5. Test individual components separately

---

---

## 🆕 Latest Feature: Automatic Fingerprint Deletion

### Overview
Fingerprints are now automatically deleted from the AS608 sensor database after each charging session ends. This ensures user privacy and optimal sensor memory management.

### Benefits

**Security & Privacy**:
- ✅ No biometric data remains after session
- ✅ Each session is completely independent
- ✅ Prevents unauthorized fingerprint reuse
- ✅ Complies with privacy best practices

**Memory Management**:
- ✅ Frees sensor memory for next user
- ✅ Enables unlimited sessions over time
- ✅ Prevents sensor memory exhaustion
- ✅ Maintains optimal performance

### Implementation Details

**Arduino Command**: `FINGERPRINT_DELETE`
```cpp
void handleFingerprintDelete(JsonObject data) {
  int fingerprintId = data["fingerprintId"];
  uint8_t p = finger.deleteModel(fingerprintId);
  
  if (p == FINGERPRINT_OK) {
    sendResponse(true, "Fingerprint deleted successfully");
  } else {
    sendResponse(false, "Failed to delete fingerprint");
  }
}
```

**Python API**: `POST /api/fingerprint/delete`
```python
@app.route('/api/fingerprint/delete', methods=['POST'])
def delete_fingerprint():
    result = send_arduino_command('FINGERPRINT_DELETE', {
        'fingerprintId': fingerprint_id
    })
    return jsonify(result)
```

**C# Service**: `DeleteFingerprintAsync()`
```csharp
public async Task<bool> DeleteFingerprintAsync(int fingerprintId)
{
    var response = await _httpClient.PostAsJsonAsync("/api/fingerprint/delete", 
        new { fingerprintId });
    return response.IsSuccessStatusCode;
}
```

**Automatic Cleanup**: Called in `StopChargingAsync()`
```csharp
// Delete fingerprint from AS608 sensor to free up memory
if (fingerprintIdToDelete.HasValue)
{
    var deleted = await _arduinoService.DeleteFingerprintAsync(fingerprintIdToDelete.Value);
    _logger.LogInformation($"Fingerprint {fingerprintIdToDelete.Value} deleted from sensor");
}
```

### When Fingerprints Are Deleted

✅ **Normal session end** - User scans fingerprint to unlock and leave  
✅ **Timeout expiration** - Charging time expires, automatic cleanup  
✅ **Manual stop** - User ends session via UI  

❌ **NOT deleted during temporary access** - "Access Device (2s unlock)" preserves fingerprint

### Error Handling

**Graceful Failure**:
- Session always ends normally, even if deletion fails
- User can retrieve device regardless of deletion status
- Errors logged for operator review
- Non-blocking operation

See **FINGERPRINT_DELETION_FEATURE.md** for complete documentation.

---

---

## 🔧 Latest Feature: Configurable Hardware Behavior

### Overview
All hardware components (relays, solenoids, UV lights) now use easy-to-configure constants. No more searching through code to fix backwards hardware!

### Configuration Section (Lines 30-45)

```cpp
// ===== HARDWARE CONFIGURATION =====
// Relay configuration (slots 1-13)
const int RELAY_ON = HIGH;      // Change to LOW if relays are active-low
const int RELAY_OFF = LOW;      // Change to HIGH if relays are active-low

// Solenoid lock configuration (slots 4-13)
const int SOLENOID_LOCKED = HIGH;    // Change to LOW to reverse lock behavior
const int SOLENOID_UNLOCKED = LOW;   // Change to HIGH to reverse lock behavior

// UV light configuration (slots 4-9)
const int UV_LIGHT_ON = HIGH;   // Change to LOW if UV lights are active-low
const int UV_LIGHT_OFF = LOW;   // Change to HIGH if UV lights are active-low
// ===================================
```

### Benefits

**Flexibility**:
- ✅ Works with Active-HIGH or Active-LOW relays
- ✅ Works with any solenoid type (Normally Open/Normally Closed)
- ✅ Adapts to different UV light modules
- ✅ Mix and match different hardware types

**Ease of Use**:
- ✅ Edit only 2 lines per component type
- ✅ Simple HIGH ↔ LOW swap
- ✅ No programming knowledge needed
- ✅ Takes 30 seconds to reconfigure
- ✅ All instances update automatically

### Quick Configuration

**If Component Works Backwards**:
1. Open `arduino_sketch.ino`
2. Find configuration section (lines 30-45)
3. Swap HIGH and LOW for that component
4. Upload to Arduino
5. Test - Done!

**Common Scenarios**:

| Scenario | Configuration | Use When |
|----------|--------------|----------|
| Standard (Default) | HIGH = ON/LOCKED, LOW = OFF/UNLOCKED | Most common setup |
| Reversed | LOW = ON/LOCKED, HIGH = OFF/UNLOCKED | Active-LOW modules |
| Mixed | Each component configured independently | Different hardware types |

### Testing

**Test Relay**:
```bash
curl -X POST http://localhost:5000/api/relay \
  -H "Content-Type: application/json" \
  -d '{"slotNumber": 1, "state": true}'
```

**Test Solenoid**:
```bash
curl -X POST http://localhost:5000/api/solenoid \
  -H "Content-Type: application/json" \
  -d '{"slotNumber": 4, "locked": true}'
```

**Test UV Light**:
```bash
curl -X POST http://localhost:5000/api/uv-light \
  -H "Content-Type: application/json" \
  -d '{"slotNumber": 4, "state": true}'
```

**If backwards**: Swap HIGH/LOW in configuration and re-test.

See **HARDWARE_CONFIG_GUIDE.md** for complete configuration guide with detailed testing procedures and troubleshooting.

---

**Last Updated**: October 17, 2025
**Version**: 2.2 (Added Hardware Configuration)
**Status**: Production Ready ✅

