# AS608 Fingerprint Sensor Setup Guide

Complete guide for integrating the AS608 fingerprint sensor with the Solar Charging Station.

## Table of Contents
1. [AS608 Specifications](#as608-specifications)
2. [Wiring Connections](#wiring-connections)
3. [Library Installation](#library-installation)
4. [Testing the Sensor](#testing-the-sensor)
5. [Enrolling Fingerprints](#enrolling-fingerprints)
6. [Troubleshooting](#troubleshooting)

---

## AS608 Specifications

### Technical Details

| Parameter | Specification |
|-----------|--------------|
| Supply Voltage | DC 3.3V - 5V |
| Supply Current | Working: <120mA, Peak: <140mA |
| Interface | UART (TTL Serial) |
| Baud Rate | 9600 - 115200 (default: 57600) |
| Image Size | 256 x 288 pixels |
| Template Size | 512 bytes |
| Storage Capacity | 300 fingerprints (AS608) / 1000 (AS608-1) |
| False Accept Rate | <0.001% |
| False Reject Rate | <1.0% |
| Matching Time | <1 second |
| Dimensions | 56mm x 20mm x 21.5mm |

### Features

- ✅ 360-degree finger rotation recognition
- ✅ Built-in template storage (non-volatile memory)
- ✅ High-speed DSP processor
- ✅ ESD protection
- ✅ Compatible with R305, R307, ZFM-20 series

---

## Wiring Connections

### AS608 Pin Configuration

The AS608 has 6 pins:

```
AS608 Module          Arduino Mega
┌─────────────┐
│ 1. VCC (Red)    ──► 5V or 3.3V
│ 2. GND (Black)  ──► GND
│ 3. TX  (White)  ──► Pin 11 (Arduino RX)
│ 4. RX  (Green)  ──► Pin 10 (Arduino TX)
│ 5. WAKE (Blue)  ──► Not connected (optional)
│ 6. 3V3  (NC)    ──► Not connected
└─────────────┘
```

### Important Notes

⚠️ **Power Considerations:**
- The AS608 can work with both 3.3V and 5V
- If using 5V: Connect VCC directly to Arduino 5V
- If using 3.3V: Connect VCC to Arduino 3.3V pin
- **Peak current can reach 140mA** - ensure stable power supply

⚠️ **Serial Communication:**
- TX (sensor) → RX (Arduino Pin 11)
- RX (sensor) → TX (Arduino Pin 10)
- Note the crossover: TX goes to RX, RX goes to TX

### Physical Wiring Diagram

```
Arduino Mega 2560          AS608 Sensor
   ┌─────────┐            ┌──────────┐
   │         │            │          │
   │ 5V   ───┼────────────┤ VCC (Red)│
   │ GND  ───┼────────────┤ GND (Blk)│
   │ Pin10───┼────────────┤ RX (Grn) │
   │ Pin11───┼────────────┤ TX (Wht) │
   │         │            │          │
   └─────────┘            └──────────┘
```

---

## Library Installation

### 1. Install Arduino IDE

Download from: https://www.arduino.cc/en/software

### 2. Install Required Libraries

**Method 1: Arduino Library Manager (Recommended)**

1. Open Arduino IDE
2. Go to **Sketch → Include Library → Manage Libraries**
3. Search for "Adafruit Fingerprint"
4. Install **Adafruit Fingerprint Sensor Library**
5. Search for "ArduinoJson"
6. Install **ArduinoJson by Benoit Blanchon**

**Method 2: Manual Installation**

```bash
# Download libraries
git clone https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library.git
git clone https://github.com/bblanchon/ArduinoJson.git

# Copy to Arduino libraries folder
# Windows: Documents\Arduino\libraries\
# Mac: ~/Documents/Arduino/libraries/
# Linux: ~/Arduino/libraries/
```

### 3. Library Dependencies

The Adafruit Fingerprint library automatically includes:
- Adafruit BusIO
- SoftwareSerial (built-in)

---

## Testing the Sensor

### Basic Test Sketch

Upload this simple test to verify your AS608 is working:

```cpp
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(11, 10); // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  Serial.begin(9600);
  finger.begin(57600);
  
  Serial.println("AS608 Fingerprint Test");
  
  if (finger.verifyPassword()) {
    Serial.println("✓ Sensor found!");
    
    finger.getTemplateCount();
    Serial.print("Sensor contains ");
    Serial.print(finger.templateCount);
    Serial.println(" templates");
  } else {
    Serial.println("✗ Sensor not found :(");
    Serial.println("Check wiring!");
  }
}

void loop() {
  // Test sensor responsiveness
  int p = finger.getImage();
  if (p == FINGERPRINT_OK) {
    Serial.println("Finger detected!");
    delay(2000);
  }
  delay(50);
}
```

### Expected Output

```
AS608 Fingerprint Test
✓ Sensor found!
Sensor contains 0 templates
```

### Troubleshooting Connection Issues

If sensor is not found:

1. **Check Power:**
   ```cpp
   // Add debug output
   Serial.println("Attempting connection at 57600 baud...");
   ```

2. **Try Different Baud Rates:**
   ```cpp
   // Some AS608 modules come with different default baud rates
   int baudRates[] = {9600, 19200, 38400, 57600, 115200};
   
   for (int i = 0; i < 5; i++) {
     finger.begin(baudRates[i]);
     if (finger.verifyPassword()) {
       Serial.print("Found at ");
       Serial.print(baudRates[i]);
       Serial.println(" baud");
       break;
     }
   }
   ```

3. **Check Wiring:**
   - Verify TX/RX crossover
   - Ensure solid connections
   - Check for damaged wires

4. **Check Power Supply:**
   - Measure voltage at VCC pin (should be 4.8-5.2V for 5V)
   - Ensure GND is connected
   - Try external power supply if Arduino can't provide enough current

---

## Enrolling Fingerprints

### Method 1: Using Arduino IDE (Manual)

Use the Adafruit example sketch:

```
File → Examples → Adafruit Fingerprint Sensor Library → enroll
```

Steps:
1. Upload the enroll sketch
2. Open Serial Monitor (9600 baud)
3. Enter fingerprint ID (1-300)
4. Follow prompts to place finger twice
5. Fingerprint is stored in sensor's memory

### Method 2: Via Python API (Automated)

```python
import requests

# Enroll new fingerprint
response = requests.post('http://localhost:5000/api/fingerprint/enroll', 
    json={'userId': 1})

print(response.json())
```

### Method 3: Via Arduino Serial Commands

Send JSON commands via Serial:

```json
{
  "command": "FINGERPRINT_ENROLL",
  "data": {
    "userId": 1
  }
}
```

### Enrollment Process

The AS608 requires **two scans** of the same finger:

```
Step 1: Place finger     → Capture image #1
Step 2: Remove finger    → Wait
Step 3: Place again      → Capture image #2
Step 4: Process          → Create template
Step 5: Store            → Save to memory (ID 1-300)
```

### Best Practices for Enrollment

1. **Clean Fingers:** Ensure finger is clean and dry
2. **Full Contact:** Press firmly but not too hard
3. **Center Finger:** Place finger in center of sensor
4. **Consistent Pressure:** Use same pressure for both scans
5. **Try Different Angles:** Enroll at slight rotation for better recognition

### Managing Enrolled Fingerprints

```cpp
// Get template count
finger.getTemplateCount();
int count = finger.templateCount;

// Delete specific fingerprint
finger.deleteModel(fingerprintId);

// Delete all fingerprints
finger.emptyDatabase();

// Load template for backup
finger.loadModel(fingerprintId);

// Download template
uint8_t buffer[512];
finger.getModel();
```

---

## Integration with Charging Station

### Enrollment Workflow for Users

1. **User Registration:**
   - User selects slot
   - System prompts for fingerprint enrollment
   - User scans finger twice
   - System stores with unique ID

2. **Verification During Charging:**
   - User inserts coins
   - System prompts for fingerprint
   - User places finger on sensor
   - AS608 matches against database
   - If match: Start charging
   - If no match: Reject

### Code Integration

The charging station uses the fingerprint as follows:

```csharp
// SlotControl.razor
private async Task VerifyFingerprint()
{
    Snackbar.Add("Scanning fingerprint...", Severity.Info);
    
    // Calls Python API → Arduino → AS608
    var verified = await SlotService.VerifyFingerprintAsync(1);
    
    if (verified)
    {
        _fingerprintVerified = true;
        Snackbar.Add("Fingerprint verified!", Severity.Success);
    }
    else
    {
        Snackbar.Add("Fingerprint not recognized", Severity.Error);
    }
}
```

### API Flow

```
UI (Blazor) → SlotService → ArduinoApiService → Python API → Arduino → AS608
    ↓
Verify Request
    ↓
AS608 Scan → Match → Response
    ↓
Update UI with result
```

---

## Troubleshooting

### Problem: Sensor Not Detected

**Symptoms:** `Sensor not found` error

**Solutions:**
1. Check wiring (TX/RX crossover)
2. Try different baud rate (9600, 57600, 115200)
3. Verify power supply (5V stable)
4. Test with example sketch first

### Problem: Can't Enroll Fingerprint

**Symptoms:** Enrollment fails or times out

**Solutions:**
1. Clean sensor window with soft cloth
2. Ensure finger is dry
3. Press firmly on sensor
4. Try different finger
5. Increase timeout in code

### Problem: Low Recognition Rate

**Symptoms:** Fingerprint not recognized even though enrolled

**Solutions:**
1. Re-enroll fingerprint
2. Enroll multiple templates of same finger
3. Adjust confidence threshold in code
4. Clean sensor regularly
5. Ensure consistent finger placement

### Problem: Intermittent Connection

**Symptoms:** Works sometimes, not others

**Solutions:**
1. Check power supply stability
2. Add decoupling capacitor (100µF) near sensor
3. Use shorter wires (<30cm recommended)
4. Check for loose connections
5. Add 1kΩ pull-up resistors on TX/RX lines

### Problem: Slow Response Time

**Symptoms:** Takes too long to verify

**Solutions:**
1. Use `fingerFastSearch()` instead of `fingerSearch()`
2. Reduce template database size
3. Increase baud rate to 115200
4. Optimize scanning timeout

---

## Advanced Configuration

### Changing Baud Rate

```cpp
// Change to 115200 baud for faster communication
finger.begin(57600);  // Current baud
finger.setPassword(0x00000000);  // Verify password
finger.setBaudrate(115200);  // Set new baud rate
// Now reconnect at new baud rate
finger.begin(115200);
```

### Adjusting Security Level

```cpp
// Set security level (1-5)
// 1 = Low security, fast matching
// 5 = High security, slower matching
finger.setSecurityLevel(3);
```

### LED Control

The AS608 has built-in LED control:

```cpp
// Turn LED on (various colors available)
finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_BLUE, 10);

// Breathing effect
finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 100, FINGERPRINT_LED_PURPLE, 0);

// Flash
finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_RED, 10);

// Turn off
finger.LEDcontrol(FINGERPRINT_LED_OFF, 0, FINGERPRINT_LED_RED, 0);
```

---

## Maintenance

### Regular Cleaning

- Clean sensor window weekly with soft, lint-free cloth
- Use isopropyl alcohol (70%) for stubborn residue
- Avoid scratching the optical window
- Keep sensor dry

### Database Management

- Back up fingerprint templates periodically
- Delete old/unused templates
- Monitor template count (max 300)
- Test recognition rates monthly

### Hardware Inspection

- Check wire connections quarterly
- Inspect for physical damage
- Verify power supply stability
- Test with known good fingerprint

---

## Resources

### Documentation
- [Adafruit Fingerprint Library](https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library)
- [AS608 Datasheet](https://cdn-shop.adafruit.com/datasheets/AS608_User_Manual.pdf)
- [Arduino Serial Communication](https://www.arduino.cc/reference/en/language/functions/communication/serial/)

### Example Projects
- [Adafruit Learning Guide](https://learn.adafruit.com/adafruit-optical-fingerprint-sensor)
- [Arduino Project Hub](https://create.arduino.cc/projecthub/projects/tags/fingerprint)

### Support
- Arduino Forum: https://forum.arduino.cc/
- Adafruit Support: https://forums.adafruit.com/
- This Project Issues: [GitHub Repository]

---

## Quick Reference

### Common Commands

| Function | Code |
|----------|------|
| Initialize | `finger.begin(57600);` |
| Verify Password | `finger.verifyPassword()` |
| Get Template Count | `finger.getTemplateCount()` |
| Capture Image | `finger.getImage()` |
| Convert Image | `finger.image2Tz(slot)` |
| Search | `finger.fingerFastSearch()` |
| Create Model | `finger.createModel()` |
| Store Model | `finger.storeModel(id)` |
| Delete Model | `finger.deleteModel(id)` |
| Empty Database | `finger.emptyDatabase()` |

### Return Codes

| Code | Meaning |
|------|---------|
| `FINGERPRINT_OK` | Success |
| `FINGERPRINT_NOFINGER` | No finger detected |
| `FINGERPRINT_IMAGEFAIL` | Image capture failed |
| `FINGERPRINT_NOTFOUND` | No match found |
| `FINGERPRINT_PACKETRECIEVEERR` | Communication error |

---

**Last Updated:** January 2025  
**Version:** 1.0  
**Compatible with:** AS608, R305, R307, ZFM-20 series sensors

