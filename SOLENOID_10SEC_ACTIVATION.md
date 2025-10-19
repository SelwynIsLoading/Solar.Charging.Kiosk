# Solenoid 10-Second Activation Update

## Overview
Modified the solenoid lock system to automatically unlock for 10 seconds and then return to locked state during both the start and end of charging sessions.

## Changes Made

### 1. Arduino Sketch (Solar4/Solar4.ino)
**Modified:** `handleSolenoid()` function
- Added `duration` parameter to support timed unlock operations
- When `duration > 0` and unlocking (`!lock`), the solenoid:
  1. Unlocks immediately
  2. Waits for specified duration (in seconds)
  3. Automatically locks back

```cpp
void handleSolenoid(JsonObject data) {
  int slot = data["slot"];
  bool lock = data["lock"];
  int duration = data["duration"] | 0; // Get duration in seconds, default 0 (permanent)
  
  if (slot >= 4 && slot <= 13) {
    digitalWrite(SOLENOID_PINS[slot - 4], lock ? SOLENOID_LOCKED : SOLENOID_UNLOCKED);
    
    // If duration is specified, wait and then return to locked state
    if (duration > 0 && !lock) {
      delay(duration * 1000); // Convert seconds to milliseconds
      digitalWrite(SOLENOID_PINS[slot - 4], SOLENOID_LOCKED); // Return to locked state
      sendResponse(true, "Solenoid timed unlock completed");
    } else {
      sendResponse(true, "Solenoid controlled");
    }
  }
}
```

### 2. Python API (python-api-sample/app.py)
**Modified:** `/api/solenoid` endpoint
- Added `duration` parameter support
- Passes duration to Arduino for timed operations
- Enhanced logging to show duration when specified

```python
@app.route('/api/solenoid', methods=['POST'])
def control_solenoid():
    data = request.json
    slot_number = data.get('slotNumber')
    lock_state = data.get('locked')
    duration = data.get('duration', 0)  # Duration in seconds, default 0 (permanent)
    
    result = send_arduino_command('SOLENOID', {
        'slot': slot_number,
        'lock': lock_state,
        'duration': duration
    })
```

### 3. C# Services

#### IArduinoApiService.cs
- Updated interface signature to include optional `durationSeconds` parameter

```csharp
Task<bool> ControlSolenoidAsync(int slotNumber, bool locked, int durationSeconds = 0);
```

#### ArduinoApiService.cs
- Modified `ControlSolenoidAsync()` to accept and pass duration parameter
- Sends duration to Python API

```csharp
public async Task<bool> ControlSolenoidAsync(int slotNumber, bool lockState, int durationSeconds = 0)
{
    var response = await _httpClient.PostAsJsonAsync("/api/solenoid", new
    {
        slotNumber,
        locked = lockState,
        duration = durationSeconds
    });
}
```

#### ISlotService.cs
- Updated interface to include duration parameter

```csharp
Task<bool> LockSlotAsync(int slotNumber, bool lockState, int durationSeconds = 0);
```

#### SlotService.cs
**Modified:** `StartChargingAsync()` method
- Changed from permanent lock to **10-second unlock**
- When starting a charging session, solenoid unlocks for 10 seconds to allow user to place device
- After 10 seconds, automatically locks
- **Important**: UV sanitization now starts AFTER the 10-second solenoid period completes (for Phone slots)

```csharp
// Turn on relay
await ControlRelayAsync(slotNumber, true);

// Unlock the slot for 10 seconds, then auto-lock if it's Phone or Laptop type
if (slot.Type == SlotType.Phone || slot.Type == SlotType.Laptop)
{
    // Unlock for 10 seconds to allow user to place device, then auto-lock
    await LockSlotAsync(slotNumber, false, 10);
    _logger.LogInformation($"Slot {slotNumber} locked after 10-second device placement window");
}

// For Phone slots, start UV sanitization AFTER solenoid is locked
if (slot.Type == SlotType.Phone)
{
    _logger.LogInformation($"Starting UV sanitization for slot {slotNumber} now that device is secured");
    await StartUVSanitizationAsync(slotNumber);
}
```

**Modified:** `StopChargingAsync()` method
- Changed from permanent unlock to **10-second unlock**
- When ending a charging session, solenoid unlocks for 10 seconds to allow user to retrieve device
- After 10 seconds, automatically locks back for security

```csharp
// Unlock the slot for 10 seconds to allow user to retrieve device, then auto-lock
if (slot.Type == SlotType.Phone || slot.Type == SlotType.Laptop)
{
    await LockSlotAsync(slotNumber, false, 10);
}
```

**Modified:** `LockSlotAsync()` method
- Added `durationSeconds` parameter with default value of 0 (permanent lock/unlock)
- Enhanced logging to show when timed unlock is being used

```csharp
public async Task<bool> LockSlotAsync(int slotNumber, bool lockState, int durationSeconds = 0)
{
    await _arduinoService.ControlSolenoidAsync(slotNumber, lockState, durationSeconds);
    if (durationSeconds > 0 && !lockState)
    {
        _logger.LogInformation($"Slot {slotNumber} unlocked for {durationSeconds} seconds, then auto-locking");
    }
}
```

## Behavior

### Starting Charging Session (Phone/Laptop slots):
1. User initiates charging
2. Power relay turns **ON**
3. Solenoid **unlocks** for 10 seconds
4. User has 10 seconds to place their device
5. Solenoid **automatically locks** after 10 seconds
6. Device is secured
7. **For Phone slots only**: UV sanitization starts for 15 seconds (AFTER the solenoid locks)

### Ending Charging Session (Phone/Laptop slots):
1. User initiates stop charging (or session expires)
2. Relay turns off (stops power)
3. Solenoid **unlocks** for 10 seconds
4. User has 10 seconds to retrieve their device
5. Solenoid **automatically locks** after 10 seconds
6. Slot is secured for next user

### Benefits:
- ✅ **User-friendly**: Gives users time to place/retrieve devices
- ✅ **Secure**: Automatically locks after timeout
- ✅ **No manual locking needed**: System handles lock state automatically
- ✅ **Backward compatible**: Duration parameter is optional (defaults to 0 for permanent state changes)
- ✅ **Safe UV operation**: UV light only starts AFTER the solenoid is locked and device is secured
- ✅ **Sequential timing**: Device must be placed → locked → then sanitized (for Phone slots)

## Timing Sequence for Phone Slots

### Total Start-up Time: ~25 seconds

```
┌──────────────────────────────────────────────────────────────┐
│ CHARGING SESSION START - Phone Slot                         │
└──────────────────────────────────────────────────────────────┘

[0s] ──────────────────────────────────────────────────────────
      ⚡ Power Relay: ON
      🔓 Solenoid: UNLOCKED
      👤 User Action: Place your phone in the slot
      
[0-10s] ═══════════════════════════════════════════════════════
        ⏱️  10-second window to place device
        
[10s] ─────────────────────────────────────────────────────────
      🔒 Solenoid: LOCKED (auto-lock)
      📱 Device: SECURED
      💡 UV Light: ON (starts immediately after lock)
      
[10-25s] ══════════════════════════════════════════════════════
         ⏱️  15-second UV sanitization
         🧼 Sanitizing your device
         
[25s] ─────────────────────────────────────────────────────────
      💡 UV Light: OFF
      🔋 Status: CHARGING (secured, sanitized, ready)
      ✅ Session Active
```

### Key Points:
1. **0 seconds**: Power relay ON
2. **0-10 seconds**: Solenoid UNLOCKED (user places phone)
3. **10 seconds**: Solenoid LOCKED (phone secured)
4. **10-25 seconds**: UV sanitization (15 seconds)
5. **25 seconds**: Charging begins with secured and sanitized phone

This sequence ensures:
- ✅ User has adequate time to place the device (10 seconds)
- ✅ Device is locked before UV light starts (safety)
- ✅ UV sanitization happens with device properly positioned
- ✅ No UV exposure while door/compartment is open
- ✅ Device is both secured AND sanitized before charging

## Deployment Notes

1. **Upload new Arduino sketch** (`Solar4.ino`) to Arduino Mega
2. **Restart Python API** to use updated `app.py`
3. **Rebuild and restart** .NET application

## Testing

### Test Sequence for Phone Slots:
1. **Start charging session** on a Phone slot (e.g., Slot 4)
2. **Verify power relay** turns ON immediately
3. **Observe solenoid** unlocking for 10 seconds (you can place phone)
4. **Wait and verify** solenoid automatically locks after 10 seconds
5. **Observe UV light** starts AFTER solenoid locks (runs for 15 seconds)
6. **Wait for UV** to complete (total ~25 seconds from start)
7. **End the charging session**
8. **Observe solenoid** unlocking for 10 seconds (retrieve phone)
9. **Verify** solenoid locks after 10 seconds

### Test Sequence for Laptop Slots:
1. **Start charging session** on a Laptop slot (e.g., Slot 10)
2. **Verify power relay** turns ON immediately
3. **Observe solenoid** unlocking for 10 seconds (you can place laptop)
4. **Wait and verify** solenoid automatically locks after 10 seconds
5. **Note**: No UV light (Laptop slots don't have UV sanitization)
6. **End the charging session**
7. **Observe solenoid** unlocking for 10 seconds (retrieve laptop)
8. **Verify** solenoid locks after 10 seconds

### Expected Timing for Phone Slots:
- **t=0s**: Relay ON, Solenoid UNLOCKED
- **t=10s**: Solenoid LOCKED
- **t=10s**: UV light ON
- **t=25s**: UV light OFF, charging ready

## Configuration

To change the duration from 10 seconds to a different value:
- Modify the duration parameter in `SlotService.cs`:
  ```csharp
  await LockSlotAsync(slotNumber, false, 10);  // Change 10 to desired seconds
  ```

The system still supports permanent lock/unlock operations by passing `0` for duration (or omitting the parameter).

