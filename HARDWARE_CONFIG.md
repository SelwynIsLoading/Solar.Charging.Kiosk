# Hardware Configuration Guide

## Relay Configuration

### Active-High Relay Configuration

Your relay module uses **ACTIVE HIGH** logic:

| Signal | Relay State | Power Output |
|--------|-------------|--------------|
| **HIGH** | ON | Power to outlet |
| **LOW** | OFF | No power to outlet |

**In Code:**
```cpp
// Turn ON relay (provide power)
digitalWrite(RELAY_PIN, HIGH);  // HIGH = ON

// Turn OFF relay (cut power)
digitalWrite(RELAY_PIN, LOW);   // LOW = OFF
```

**Configuration:**
- Direct logic (HIGH = ON, LOW = OFF)
- Simple to understand
- Matches boolean state directly

### Default State: OFF (Safe)

```cpp
// Initialize all relays to OFF
for (int i = 0; i < 13; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], LOW);  // LOW = OFF
}
```

**On power-up or reset:**
- All relays → LOW → OFF
- All outlets → No power
- Safe state!

---

## Solenoid Configuration

### Solenoid Lock Configuration

Your solenoid locks:

| Signal | Solenoid State | Door State |
|--------|----------------|------------|
| **HIGH** | Energized | Locked (secure) |
| **LOW** | Not energized | Unlocked |

**In Code:**
```cpp
// LOCK the door (secure state)
digitalWrite(SOLENOID_PIN, HIGH);  // HIGH = locked

// UNLOCK the door (release)
digitalWrite(SOLENOID_PIN, LOW);   // LOW = unlocked
```

**Configuration:**
- HIGH signal locks the door
- LOW signal unlocks the door
- Default to locked for security

### Default State: Locked (Secure)

```cpp
// Initialize all solenoids to locked
for (int i = 0; i < 10; i++) {
    pinMode(SOLENOID_PINS[i], OUTPUT);
    digitalWrite(SOLENOID_PINS[i], HIGH);  // HIGH = locked
}
```

**On power-up or reset:**
- All solenoids → HIGH
- All doors → Locked
- Secure state!

---

## Logic Summary

### Quick Reference Table

| Component | Action | Arduino Code | Physical Result |
|-----------|--------|--------------|-----------------|
| **Relay** | Turn ON power | `LOW` | Outlet has 220V |
| **Relay** | Turn OFF power | `HIGH` | Outlet no power |
| **Solenoid** | Lock door | `LOW` | Door locked |
| **Solenoid** | Unlock door | `HIGH` | Door opens |

### Code Implementation

```cpp
// Relay Control
void handleRelay(JsonObject data) {
  bool state = data["state"];  // true = turn ON, false = turn OFF
  
  // ACTIVE LOW: Invert logic
  digitalWrite(RELAY_PIN, state ? LOW : HIGH);
  //                       true → LOW  (ON)
  //                       false → HIGH (OFF)
}

// Solenoid Control
void handleSolenoid(JsonObject data) {
  bool lock = data["lock"];  // true = lock, false = unlock
  
  // Normally Open: Invert logic
  digitalWrite(SOLENOID_PIN, lock ? LOW : HIGH);
  //                         true → LOW  (locked)
  //                         false → HIGH (unlocked)
}
```

---

## Safety Features

### Power-On State

When Arduino starts/resets:

```
1. All relays → HIGH → Outlets OFF → ✅ Safe
2. All solenoids → LOW → Doors locked → ✅ Secure
```

### Power-Off Behavior

If Arduino loses power:

```
Relays (with pull-up):
- Pins float HIGH
- Relays turn OFF
- No power to outlets → ✅ Safe

Solenoids:
- Pins float LOW (or add pull-down)
- Solenoids de-energize
- Doors stay locked → ✅ Secure
```

### Emergency Stop

Can be implemented:

```cpp
void emergencyStop() {
  // Cut all power
  for (int i = 0; i < 13; i++) {
    digitalWrite(RELAY_PINS[i], HIGH);  // OFF
  }
  
  // Unlock all doors (safety!)
  for (int i = 0; i < 10; i++) {
    digitalWrite(SOLENOID_PINS[i], HIGH);  // Unlocked
  }
}
```

---

## Hardware Compatibility

### Relay Module Types

**Active-Low (Your Type):**
```
VCC, GND, IN1-IN16
- IN pins: LOW = ON, HIGH = OFF
- Most common type
- Usually has optocouplers
```

**Active-High (Different Type):**
```
If you had this type:
digitalWrite(RELAY_PIN, state ? HIGH : LOW);  // Opposite!
```

**How to Identify:**
- LED on module lights when pin is LOW → Active-Low ✓
- LED on module lights when pin is HIGH → Active-High

### Solenoid Lock Types

**Normally Open (Your Type):**
```
Default: De-energized = Locked
Energized: HIGH = Unlocked/Released
- Fail-secure (power loss = locked)
- Requires power to unlock
```

**Normally Closed (Different Type):**
```
If you had this type:
digitalWrite(SOLENOID_PIN, lock ? HIGH : LOW);  // Opposite!
Default: Energized = Locked
```

**How to Identify:**
- With no power: Door locked → Normally Open ✓
- With no power: Door unlocked → Normally Closed

---

## Testing

### Test Relay Logic

```cpp
// Upload sketch
// Open Serial Monitor

// Test turn ON
{"command":"RELAY","data":{"slot":1,"state":true}}
// Should see: Relay clicks, LED on module turns ON

// Test turn OFF  
{"command":"RELAY","data":{"slot":1,"state":false}}
// Should see: Relay clicks, LED on module turns OFF
```

**Verify with multimeter:**
```
Test relay COM and NO pins:
- Command ON (LOW signal) → Continuity between COM and NO
- Command OFF (HIGH signal) → No continuity
```

### Test Solenoid Logic

```cpp
// Test LOCK (secure state)
{"command":"SOLENOID","data":{"slot":4,"lock":true}}
// Solenoid should: De-energize (or stay de-energized)
// Door should: Be locked

// Test UNLOCK (release)
{"command":"SOLENOID","data":{"slot":4,"lock":false}}
// Solenoid should: Energize (you might hear a click)
// Door should: Be unlocked/released
```

---

## Wiring Recommendations

### Add Pull-up/Pull-down Resistors

**For Relays (Active-Low):**
```
Arduino Pin ──┬──► 10kΩ Pull-up to +5V
              │
              └──► Relay IN pin

Ensures HIGH (OFF) if Arduino not connected
```

**For Solenoids:**
```
Arduino Pin ──┬──► 10kΩ Pull-down to GND
              │
              └──► Transistor Base (via 1kΩ)

Ensures LOW (locked) if Arduino not connected
```

### Power Indicators

Add LEDs to monitor states:

```cpp
const int POWER_LED = 51;   // Shows if relay is ON
const int LOCK_LED = 52;    // Shows if solenoid is locked

void handleRelay(JsonObject data) {
  bool state = data["state"];
  digitalWrite(RELAY_PINS[slot - 1], state ? LOW : HIGH);
  digitalWrite(POWER_LED, state ? HIGH : LOW);  // LED on when power ON
}

void handleSolenoid(JsonObject data) {
  bool lock = data["lock"];
  digitalWrite(SOLENOID_PINS[slot - 4], lock ? LOW : HIGH);
  digitalWrite(LOCK_LED, lock ? HIGH : LOW);  // LED on when locked
}
```

---

## Configuration Summary

### Current Configuration (UPDATED)

```cpp
// RELAYS (Active-High)
// Command: state = true (turn ON)
digitalWrite(RELAY_PIN, HIGH);  // HIGH = ON ✓

// Command: state = false (turn OFF)
digitalWrite(RELAY_PIN, LOW);   // LOW = OFF ✓

// SOLENOIDS
// Command: lock = true (lock door)
digitalWrite(SOLENOID_PIN, HIGH);  // HIGH = locked ✓

// Command: lock = false (unlock door)
digitalWrite(SOLENOID_PIN, LOW);   // LOW = unlocked ✓

// UV LIGHTS
// Command: state = true (turn ON)
digitalWrite(UV_PIN, HIGH);  // HIGH = ON ✓

// Command: state = false (turn OFF)
digitalWrite(UV_PIN, LOW);   // LOW = OFF ✓
```

### If You Have Different Hardware

**Active-High Relays:**
```cpp
digitalWrite(RELAY_PIN, state ? HIGH : LOW);
```

**Normally Closed Solenoids:**
```cpp
digitalWrite(SOLENOID_PIN, lock ? HIGH : LOW);
```

**To check your hardware type:**
1. Measure relay module with multimeter
2. Send LOW signal, check if relay clicks ON
3. Test solenoid with 12V directly
4. See if LOW or HIGH locks the door

---

## Updated Arduino Code

The main `arduino_sketch.ino` has been updated with:

✅ **Relays:** Active-Low (inverted)
✅ **Solenoids:** Normally Open (inverted for lock logic)
✅ **Safe defaults:** Relays OFF, Solenoids locked
✅ **Clear comments:** Explains the logic

**Ready to upload!**

---

## Testing Checklist

After uploading updated sketch:

- [ ] Test Relay ON → Should hear click, LED on module lights up
- [ ] Test Relay OFF → Should hear click, LED turns off
- [ ] Verify with multimeter: LOW = relay ON
- [ ] Test Solenoid LOCK → Should hear click (or stay silent if already locked)
- [ ] Test Solenoid UNLOCK → Should hear click, door releases
- [ ] Verify: LOW = locked, HIGH = unlocked
- [ ] Test from UI → Relays respond correctly
- [ ] Test from UI → Solenoids lock/unlock correctly

The hardware logic is now correctly configured for your setup! 🎉

