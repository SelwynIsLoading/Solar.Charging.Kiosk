# Hardware Configuration Guide

## Overview
The Arduino sketch now includes **easy-to-configure constants** for all hardware components. This allows you to adapt the system to different relay modules, solenoid types, and UV light configurations without editing code throughout the file.

---

## Configuration Section

### Location
Lines 30-45 in `arduino_sketch.ino`:

```cpp
// ===== HARDWARE CONFIGURATION =====
// Change these values based on your specific hardware setup
// If your relays/solenoids/UV lights work backwards, just swap HIGH and LOW

// Relay configuration (slots 1-13)
const int RELAY_ON = HIGH;      // Change to LOW if your relays are active-low
const int RELAY_OFF = LOW;      // Change to HIGH if your relays are active-low

// Solenoid lock configuration (slots 4-13)
const int SOLENOID_LOCKED = HIGH;    // Change to LOW if your solenoids lock with LOW signal
const int SOLENOID_UNLOCKED = LOW;   // Change to HIGH if your solenoids unlock with HIGH signal

// UV light configuration (slots 4-9)
const int UV_LIGHT_ON = HIGH;   // Change to LOW if your UV lights turn on with LOW signal
const int UV_LIGHT_OFF = LOW;   // Change to HIGH if your UV lights turn off with HIGH signal
// ===================================
```

---

## How to Configure

### Step 1: Identify Your Hardware Type

**Check your relay/solenoid/UV module documentation or test manually:**

1. **Upload the default sketch** to Arduino
2. **Test each component** individually
3. **If it works backwards**, swap HIGH and LOW in the configuration

---

## Relay Configuration

### Default Configuration (Active-HIGH)
```cpp
const int RELAY_ON = HIGH;      // Signal HIGH turns relay ON
const int RELAY_OFF = LOW;      // Signal LOW turns relay OFF
```

**Use this if:**
- ✅ Relay turns ON when you send HIGH signal
- ✅ Relay turns OFF when you send LOW signal
- ✅ Common with most relay modules

---

### Reversed Configuration (Active-LOW)
```cpp
const int RELAY_ON = LOW;       // Signal LOW turns relay ON
const int RELAY_OFF = HIGH;     // Signal HIGH turns relay OFF
```

**Use this if:**
- ✅ Relay turns ON when you send LOW signal
- ✅ Relay turns OFF when you send HIGH signal
- ✅ Common with some solid-state relays

---

### How to Test Relays

**Method 1: Arduino Serial Monitor**
1. Upload sketch
2. Send command: `{"command":"RELAY","data":{"slot":1,"state":true}}`
3. Check if relay clicks ON
4. Send command: `{"command":"RELAY","data":{"slot":1,"state":false}}`
5. Check if relay clicks OFF

**Method 2: Python API**
```bash
# Turn ON relay
curl -X POST http://localhost:5000/api/relay \
  -H "Content-Type: application/json" \
  -d '{"slotNumber": 1, "state": true}'

# Turn OFF relay
curl -X POST http://localhost:5000/api/relay \
  -H "Content-Type: application/json" \
  -d '{"slotNumber": 1, "state": false}'
```

**If relay works backwards**: Swap HIGH and LOW in relay configuration.

---

## Solenoid Configuration

### Default Configuration (Active-HIGH Lock)
```cpp
const int SOLENOID_LOCKED = HIGH;    // Signal HIGH locks the solenoid
const int SOLENOID_UNLOCKED = LOW;   // Signal LOW unlocks the solenoid
```

**Use this if:**
- ✅ Door/compartment LOCKS when you send HIGH signal
- ✅ Door/compartment UNLOCKS when you send LOW signal
- ✅ Common with electric bolt locks

---

### Reversed Configuration (Active-LOW Lock)
```cpp
const int SOLENOID_LOCKED = LOW;     // Signal LOW locks the solenoid
const int SOLENOID_UNLOCKED = HIGH;  // Signal HIGH unlocks the solenoid
```

**Use this if:**
- ✅ Door/compartment LOCKS when you send LOW signal
- ✅ Door/compartment UNLOCKS when you send HIGH signal
- ✅ Common with some electromagnetic locks

---

### How to Test Solenoids

**Method 1: Arduino Serial Monitor**
1. Upload sketch
2. Send command: `{"command":"SOLENOID","data":{"slot":4,"lock":true}}`
3. Check if door locks (can't open)
4. Send command: `{"command":"SOLENOID","data":{"slot":4,"lock":false}}`
5. Check if door unlocks (can open)

**Method 2: Python API**
```bash
# LOCK solenoid
curl -X POST http://localhost:5000/api/solenoid \
  -H "Content-Type: application/json" \
  -d '{"slotNumber": 4, "locked": true}'

# UNLOCK solenoid
curl -X POST http://localhost:5000/api/solenoid \
  -H "Content-Type: application/json" \
  -d '{"slotNumber": 4, "locked": false}'
```

**If solenoid works backwards**: Swap HIGH and LOW in solenoid configuration.

---

## UV Light Configuration

### Default Configuration (Active-HIGH)
```cpp
const int UV_LIGHT_ON = HIGH;   // Signal HIGH turns UV light ON
const int UV_LIGHT_OFF = LOW;   // Signal LOW turns UV light OFF
```

**Use this if:**
- ✅ UV light turns ON when you send HIGH signal
- ✅ UV light turns OFF when you send LOW signal
- ✅ Most common configuration

---

### Reversed Configuration (Active-LOW)
```cpp
const int UV_LIGHT_ON = LOW;    // Signal LOW turns UV light ON
const int UV_LIGHT_OFF = HIGH;  // Signal HIGH turns UV light OFF
```

**Use this if:**
- ✅ UV light turns ON when you send LOW signal
- ✅ UV light turns OFF when you send HIGH signal
- ✅ Some specialty UV modules

---

### How to Test UV Lights

**Method 1: Arduino Serial Monitor**
1. Upload sketch
2. Send command: `{"command":"UV_LIGHT","data":{"slot":4,"state":true}}`
3. Check if UV light turns ON (visible glow)
4. Send command: `{"command":"UV_LIGHT","data":{"slot":4,"state":false}}`
5. Check if UV light turns OFF

**Method 2: Python API**
```bash
# Turn ON UV light
curl -X POST http://localhost:5000/api/uv-light \
  -H "Content-Type: application/json" \
  -d '{"slotNumber": 4, "state": true}'

# Turn OFF UV light
curl -X POST http://localhost:5000/api/uv-light \
  -H "Content-Type: application/json" \
  -d '{"slotNumber": 4, "state": false}'
```

**If UV light works backwards**: Swap HIGH and LOW in UV light configuration.

---

## Common Hardware Scenarios

### Scenario 1: All Components Active-HIGH
**Configuration** (DEFAULT):
```cpp
const int RELAY_ON = HIGH;
const int RELAY_OFF = LOW;
const int SOLENOID_LOCKED = HIGH;
const int SOLENOID_UNLOCKED = LOW;
const int UV_LIGHT_ON = HIGH;
const int UV_LIGHT_OFF = LOW;
```

**No changes needed!**

---

### Scenario 2: All Components Active-LOW
**Configuration**:
```cpp
const int RELAY_ON = LOW;
const int RELAY_OFF = HIGH;
const int SOLENOID_LOCKED = LOW;
const int SOLENOID_UNLOCKED = HIGH;
const int UV_LIGHT_ON = LOW;
const int UV_LIGHT_OFF = HIGH;
```

**Just swap all HIGH/LOW values.**

---

### Scenario 3: Mixed Configuration
**Example**: Relays are active-HIGH, but solenoids are active-LOW

**Configuration**:
```cpp
const int RELAY_ON = HIGH;           // Active-HIGH relays
const int RELAY_OFF = LOW;
const int SOLENOID_LOCKED = LOW;     // Active-LOW solenoids  
const int SOLENOID_UNLOCKED = HIGH;
const int UV_LIGHT_ON = HIGH;        // Active-HIGH UV lights
const int UV_LIGHT_OFF = LOW;
```

**Configure each component independently!**

---

### Scenario 4: Inverted Solenoid Wiring
**Problem**: Door locks when you want it unlocked

**Solution**:
```cpp
// Just swap these two lines:
const int SOLENOID_LOCKED = LOW;     // Was HIGH
const int SOLENOID_UNLOCKED = HIGH;  // Was LOW
```

**Upload and test again!**

---

## Quick Configuration Table

| Hardware Type | Signal HIGH | Signal LOW | Config Values |
|--------------|-------------|------------|---------------|
| **Active-HIGH** | Component ON/LOCKED | Component OFF/UNLOCKED | HIGH = ON, LOW = OFF |
| **Active-LOW** | Component OFF/UNLOCKED | Component ON/LOCKED | LOW = ON, HIGH = OFF |

---

## Troubleshooting

### Issue: Relays Work Backwards

**Symptom**: Relay turns OFF when you want it ON

**Solution**:
```cpp
// Change from:
const int RELAY_ON = HIGH;
const int RELAY_OFF = LOW;

// To:
const int RELAY_ON = LOW;
const int RELAY_OFF = HIGH;
```

---

### Issue: Solenoids Work Backwards

**Symptom**: Door unlocks when you want it locked

**Solution**:
```cpp
// Change from:
const int SOLENOID_LOCKED = HIGH;
const int SOLENOID_UNLOCKED = LOW;

// To:
const int SOLENOID_LOCKED = LOW;
const int SOLENOID_UNLOCKED = HIGH;
```

---

### Issue: UV Lights Work Backwards

**Symptom**: UV light turns OFF when you want it ON

**Solution**:
```cpp
// Change from:
const int UV_LIGHT_ON = HIGH;
const int UV_LIGHT_OFF = LOW;

// To:
const int UV_LIGHT_ON = LOW;
const int UV_LIGHT_OFF = HIGH;
```

---

### Issue: Only Some Components Work Backwards

**Solution**: Configure each component type independently!

**Example**: Relays work fine, but solenoids backwards
```cpp
// Leave relays as default:
const int RELAY_ON = HIGH;
const int RELAY_OFF = LOW;

// Reverse only solenoids:
const int SOLENOID_LOCKED = LOW;
const int SOLENOID_UNLOCKED = HIGH;

// Leave UV lights as default:
const int UV_LIGHT_ON = HIGH;
const int UV_LIGHT_OFF = LOW;
```

---

## Testing Procedure

### Complete System Test

1. **Configure Hardware**
   ```cpp
   // Edit configuration constants in arduino_sketch.ino
   ```

2. **Upload to Arduino**
   ```bash
   # Use Arduino IDE to upload sketch
   ```

3. **Start Python API**
   ```bash
   cd python-api-sample
   python app.py
   ```

4. **Test Relays** (all 13 slots)
   ```bash
   # Test slot 1
   curl -X POST http://localhost:5000/api/relay \
     -H "Content-Type: application/json" \
     -d '{"slotNumber": 1, "state": true}'
   # Should turn ON
   
   curl -X POST http://localhost:5000/api/relay \
     -H "Content-Type: application/json" \
     -d '{"slotNumber": 1, "state": false}'
   # Should turn OFF
   ```

5. **Test Solenoids** (slots 4-13)
   ```bash
   # Test slot 4
   curl -X POST http://localhost:5000/api/solenoid \
     -H "Content-Type: application/json" \
     -d '{"slotNumber": 4, "locked": true}'
   # Should LOCK
   
   curl -X POST http://localhost:5000/api/solenoid \
     -H "Content-Type: application/json" \
     -d '{"slotNumber": 4, "locked": false}'
   # Should UNLOCK
   ```

6. **Test UV Lights** (slots 4-9)
   ```bash
   # Test slot 4
   curl -X POST http://localhost:5000/api/uv-light \
     -H "Content-Type: application/json" \
     -d '{"slotNumber": 4, "state": true}'
   # Should turn ON
   
   curl -X POST http://localhost:5000/api/uv-light \
     -H "Content-Type: application/json" \
     -d '{"slotNumber": 4, "state": false}'
   # Should turn OFF
   ```

7. **If Any Component Works Backwards**:
   - Stop Python API
   - Edit configuration in Arduino sketch
   - Re-upload sketch to Arduino
   - Restart Python API
   - Test again

---

## Benefits of Configuration Constants

✅ **Flexibility**: Works with any hardware type  
✅ **Easy to Change**: Edit 2 lines instead of searching entire code  
✅ **No Code Knowledge Needed**: Just swap HIGH/LOW  
✅ **Quick Testing**: Test and adjust in minutes  
✅ **Hardware Independence**: Same code for different setups  
✅ **Clear Documentation**: Constants are self-documenting  

---

## Pin Reference

### Relays (Slots 1-13)
- Pins: 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34
- Configuration: `RELAY_ON` / `RELAY_OFF`

### Solenoids (Slots 4-13)
- Pins: 35, 36, 37, 38, 39, 40, 41, 42, 43, 44
- Configuration: `SOLENOID_LOCKED` / `SOLENOID_UNLOCKED`

### UV Lights (Slots 4-9)
- Pins: 45, 46, 47, 48, 49, 50
- Configuration: `UV_LIGHT_ON` / `UV_LIGHT_OFF`

---

## Example Configurations

### Example 1: Standard Setup
```cpp
// Most common configuration - no changes needed
const int RELAY_ON = HIGH;
const int RELAY_OFF = LOW;
const int SOLENOID_LOCKED = HIGH;
const int SOLENOID_UNLOCKED = LOW;
const int UV_LIGHT_ON = HIGH;
const int UV_LIGHT_OFF = LOW;
```

---

### Example 2: All Reversed
```cpp
// If ALL your components are active-low
const int RELAY_ON = LOW;
const int RELAY_OFF = HIGH;
const int SOLENOID_LOCKED = LOW;
const int SOLENOID_UNLOCKED = HIGH;
const int UV_LIGHT_ON = LOW;
const int UV_LIGHT_OFF = HIGH;
```

---

### Example 3: Mixed Configuration
```cpp
// Common with certain relay modules
const int RELAY_ON = LOW;            // Relay module is active-low
const int RELAY_OFF = HIGH;
const int SOLENOID_LOCKED = HIGH;    // Solenoids are active-high
const int SOLENOID_UNLOCKED = LOW;
const int UV_LIGHT_ON = HIGH;        // UV lights are active-high
const int UV_LIGHT_OFF = LOW;
```

---

## Summary

**Before** this feature:
- ❌ Had to edit code in multiple places
- ❌ Easy to miss a spot
- ❌ Confusing for non-programmers
- ❌ Time-consuming to change

**After** this feature:
- ✅ Edit only 6 lines (3 pairs of constants)
- ✅ All components update automatically
- ✅ Simple swap: HIGH ↔ LOW
- ✅ Takes 30 seconds to reconfigure

---

**Last Updated**: October 17, 2025  
**Version**: 2.2  
**Status**: ✅ Ready to Configure

