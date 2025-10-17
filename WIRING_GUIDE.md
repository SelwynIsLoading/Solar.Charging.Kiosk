# Hardware Wiring Guide - Solar Charging Station

This document provides detailed wiring instructions for all hardware components in the Solar Charging Station Kiosk.

## Table of Contents
1. [Components List](#components-list)
2. [Power Supply Requirements](#power-supply-requirements)
3. [Arduino Mega Pin Assignments](#arduino-mega-pin-assignments)
4. [Relay Module Wiring](#relay-module-wiring)
5. [Solenoid Lock Wiring](#solenoid-lock-wiring)
6. [UV Light Wiring](#uv-light-wiring)
7. [Fingerprint Sensor Wiring](#fingerprint-sensor-wiring)
8. [Coin Acceptor Wiring](#coin-acceptor-wiring)
9. [Complete Wiring Diagram](#complete-wiring-diagram)
10. [Safety Considerations](#safety-considerations)

---

## Components List

### Required Components

| Component | Quantity | Specifications |
|-----------|----------|----------------|
| Arduino Mega 2560 | 1 | Main controller |
| Relay Module (16-channel) | 1 | 5V trigger, 10A contacts minimum |
| Solenoid Lock 12V | 10 | For slots 4-13 |
| UV Light LED Strip | 6 | 12V, for phone slots |
| Fingerprint Sensor | 1 | AS608 or R307 compatible |
| Coin Acceptor | 1 | Multi-coin with pulse output |
| Power Supply 5V 5A | 1 | For Arduino and relays |
| Power Supply 12V 10A | 1 | For solenoids and UV lights |
| Power Supply 220V→5V | 13 | For charging outlets |
| Jumper Wires | Various | Male-Female, Male-Male |
| Screw Terminals | Various | For secure connections |
| Diodes (1N4007) | 10 | Flyback protection for solenoids |
| Resistors 10kΩ | 20 | Pull-up/down as needed |

---

## Power Supply Requirements

### Power Distribution Overview

```
Main Power Input (220V AC)
├── 5V 5A PSU ──────────► Arduino Mega + Relay Logic
├── 12V 10A PSU ────────► Solenoids + UV Lights
└── 220V→5V Adapters ──► Charging Outlets (13x)
```

### Power Consumption Calculation

| System | Voltage | Current | Total Power |
|--------|---------|---------|-------------|
| Arduino Mega | 5V | 500mA | 2.5W |
| Relay Module | 5V | 800mA | 4W |
| Solenoids (10x active) | 12V | 5A | 60W |
| UV Lights (6x active) | 12V | 3A | 36W |
| **Total System** | - | - | **~100W** |

> **Note**: Charging outlets power is separate and depends on devices being charged.

---

## Arduino Mega Pin Assignments

### Complete Pin Mapping

```
Arduino Mega 2560 (54 Digital Pins, 16 Analog Pins)

DIGITAL PINS:
├── Pin 0-1:    Reserved (USB Serial)
├── Pin 2:      Coin Acceptor (Interrupt 0)
├── Pin 3:      Reserved (Interrupt 1)
├── Pin 4-9:    Reserved for expansion
├── Pin 10-11:  Fingerprint Sensor (Software Serial TX/RX)
├── Pin 12-21:  Reserved for expansion
├── Pin 22-34:  Relay Outputs (13 channels)
├── Pin 35-44:  Solenoid Outputs (10 channels)
└── Pin 45-50:  UV Light Outputs (6 channels)

POWER PINS:
├── VIN:        Not used (USB powered or barrel jack)
├── 5V:         +5V output (max 500mA if USB powered)
├── 3.3V:       +3.3V output (max 50mA)
└── GND:        Multiple ground pins (use for all components)
```

### Pin Assignment Table

| Pin # | Function | Connected To | Notes |
|-------|----------|--------------|-------|
| 2 | INPUT | Coin Acceptor Signal | Interrupt-capable |
| 10 | TX | Fingerprint RX | Software Serial |
| 11 | RX | Fingerprint TX | Software Serial |
| 22 | OUTPUT | Relay 1 (Slot 1) | Open slot |
| 23 | OUTPUT | Relay 2 (Slot 2) | Open slot |
| 24 | OUTPUT | Relay 3 (Slot 3) | Open slot |
| 25 | OUTPUT | Relay 4 (Slot 4) | Phone slot |
| 26 | OUTPUT | Relay 5 (Slot 5) | Phone slot |
| 27 | OUTPUT | Relay 6 (Slot 6) | Phone slot |
| 28 | OUTPUT | Relay 7 (Slot 7) | Phone slot |
| 29 | OUTPUT | Relay 8 (Slot 8) | Phone slot |
| 30 | OUTPUT | Relay 9 (Slot 9) | Phone slot |
| 31 | OUTPUT | Relay 10 (Slot 10) | Laptop slot |
| 32 | OUTPUT | Relay 11 (Slot 11) | Laptop slot |
| 33 | OUTPUT | Relay 12 (Slot 12) | Laptop slot |
| 34 | OUTPUT | Relay 13 (Slot 13) | Laptop slot |
| 35 | OUTPUT | Solenoid 1 (Slot 4) | Via transistor/relay |
| 36 | OUTPUT | Solenoid 2 (Slot 5) | Via transistor/relay |
| 37 | OUTPUT | Solenoid 3 (Slot 6) | Via transistor/relay |
| 38 | OUTPUT | Solenoid 4 (Slot 7) | Via transistor/relay |
| 39 | OUTPUT | Solenoid 5 (Slot 8) | Via transistor/relay |
| 40 | OUTPUT | Solenoid 6 (Slot 9) | Via transistor/relay |
| 41 | OUTPUT | Solenoid 7 (Slot 10) | Via transistor/relay |
| 42 | OUTPUT | Solenoid 8 (Slot 11) | Via transistor/relay |
| 43 | OUTPUT | Solenoid 9 (Slot 12) | Via transistor/relay |
| 44 | OUTPUT | Solenoid 10 (Slot 13) | Via transistor/relay |
| 45 | OUTPUT | UV Light 1 (Slot 4) | Via transistor/relay |
| 46 | OUTPUT | UV Light 2 (Slot 5) | Via transistor/relay |
| 47 | OUTPUT | UV Light 3 (Slot 6) | Via transistor/relay |
| 48 | OUTPUT | UV Light 4 (Slot 7) | Via transistor/relay |
| 49 | OUTPUT | UV Light 5 (Slot 8) | Via transistor/relay |
| 50 | OUTPUT | UV Light 6 (Slot 9) | Via transistor/relay |

---

## Relay Module Wiring

### 16-Channel Relay Module Connection

```
Arduino Mega          16-Channel Relay Module
Pin 22 ──────────────► IN1  (Slot 1 Power)
Pin 23 ──────────────► IN2  (Slot 2 Power)
Pin 24 ──────────────► IN3  (Slot 3 Power)
Pin 25 ──────────────► IN4  (Slot 4 Power)
Pin 26 ──────────────► IN5  (Slot 5 Power)
Pin 27 ──────────────► IN6  (Slot 6 Power)
Pin 28 ──────────────► IN7  (Slot 7 Power)
Pin 29 ──────────────► IN8  (Slot 8 Power)
Pin 30 ──────────────► IN9  (Slot 9 Power)
Pin 31 ──────────────► IN10 (Slot 10 Power)
Pin 32 ──────────────► IN11 (Slot 11 Power)
Pin 33 ──────────────► IN12 (Slot 12 Power)
Pin 34 ──────────────► IN13 (Slot 13 Power)

5V ───────────────────► VCC
GND ──────────────────► GND
```

### Relay High-Power Side (AC Power)

For each relay (example for Slot 1):

```
220V AC Hot (Live) ──┬─► Common (COM)
                     │
                   [Relay 1]
                     │
Normally Open (NO) ──┴─► Slot 1 Outlet Hot
                         
220V AC Neutral ────────► Slot 1 Outlet Neutral (Direct)
220V AC Ground ─────────► Slot 1 Outlet Ground (Direct)
```

> **⚠️ WARNING**: Only qualified electricians should wire high-voltage AC connections!

### Relay Wiring Diagram per Slot

```
┌─────────────┐
│ Relay Module│
├─────────────┤
│ COM  NO  NC │
│  │   │   │ │
└──┼───┼───┼─┘
   │   │   └── Not connected
   │   └────── To outlet hot wire
   └────────── From 220V hot (always powered)
```

---

## Solenoid Lock Wiring

### Solenoid Control Circuit

Each solenoid requires a driver circuit (transistor or relay) because Arduino cannot directly drive 12V solenoids.

#### Option 1: Using TIP120 Transistor (Recommended)

```
Arduino Pin 35-44 ────┬──► 1kΩ Resistor ──► TIP120 Base
                      │
                    ──┴──► Flyback Diode (1N4007)
                      │    Cathode to +12V
                      │    Anode to Collector
                      │
GND ──────────────────┴──► TIP120 Emitter

+12V ─────────────────────► Solenoid +
TIP120 Collector ─────────► Solenoid -
```

#### Circuit for Each Solenoid

```
        +12V
         │
         ├──────┤◄├──── Flyback Diode (1N4007)
         │       │
    [Solenoid]  │
         │       │
         └───────┴──── TIP120 Collector
                │
Arduino Pin ─┤1kΩ├─ TIP120 Base
                │
GND ────────────┴──── TIP120 Emitter
```

### Solenoid Connections Table

| Arduino Pin | Solenoid # | Slot # | Type |
|-------------|------------|--------|------|
| 35 | 1 | 4 | Phone |
| 36 | 2 | 5 | Phone |
| 37 | 3 | 6 | Phone |
| 38 | 4 | 7 | Phone |
| 39 | 5 | 8 | Phone |
| 40 | 6 | 9 | Phone |
| 41 | 7 | 10 | Laptop |
| 42 | 8 | 11 | Laptop |
| 43 | 9 | 12 | Laptop |
| 44 | 10 | 13 | Laptop |

### Flyback Diode Placement

```
Always place a 1N4007 diode across each solenoid:
- Cathode (marked end) to +12V
- Anode to Arduino control side
This protects against voltage spikes when solenoid turns off.
```

---

## UV Light Wiring

### UV LED Strip Connection

Similar to solenoids, UV lights need a driver circuit.

```
Arduino Pin 45-50 ────► 1kΩ Resistor ──► TIP120 Base
                                    │
+12V ─────────────────────────────► UV LED Strip +
TIP120 Collector ─────────────────► UV LED Strip -
TIP120 Emitter ───────────────────► GND
```

### UV Light Connections Table

| Arduino Pin | UV Light # | Slot # | Duration |
|-------------|------------|--------|----------|
| 45 | 1 | 4 | 15 seconds |
| 46 | 2 | 5 | 15 seconds |
| 47 | 3 | 6 | 15 seconds |
| 48 | 4 | 7 | 15 seconds |
| 49 | 5 | 8 | 15 seconds |
| 50 | 6 | 9 | 15 seconds |

### UV Light Specifications

- **Voltage**: 12V DC
- **Wavelength**: 254nm (UVC) or 365nm (UVA)
- **Power**: ~5W per strip
- **Mounting**: Inside phone compartment
- **Safety**: Must have interlocks to prevent exposure when door open

---

## Fingerprint Sensor Wiring

### AS608 / R307 Fingerprint Sensor

```
Fingerprint Sensor    Arduino Mega
┌──────────────────┐
│  VCC (Red)   ────┼──► 5V (or 3.3V, check sensor spec)
│  GND (Black) ────┼──► GND
│  TX  (White) ────┼──► Pin 11 (Arduino RX)
│  RX  (Green) ────┼──► Pin 10 (Arduino TX)
│  WAKE (Yellow)   │    (Optional, leave disconnected)
└──────────────────┘
```

### Software Serial Configuration

```cpp
#include <SoftwareSerial.h>

SoftwareSerial mySerial(11, 10); // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  mySerial.begin(57600);  // Fingerprint sensor baud rate
  finger.begin(57600);
}
```

### Fingerprint Sensor Mounting

- Mount near slot access point
- Ensure clean surface for scanning
- Protect from moisture
- LED indicator should be visible to user

---

## Coin Acceptor Wiring

### Multi-Coin Acceptor with Pulse Output

```
Coin Acceptor        Arduino Mega
┌──────────────┐
│  +12V (Red)  ├──► +12V Power Supply
│  GND (Black) ├──► GND (Common Ground)
│  COIN (Blue) ├──► Pin 2 (Interrupt 0)
│  INHIBIT    │    (Optional control)
└──────────────┘
```

### Pulse Output Logic

The coin acceptor sends pulses based on coin value:
- 1 Peso = 1 pulse
- 5 Pesos = 5 pulses
- 10 Pesos = 10 pulses
- 20 Pesos = 20 pulses

### Coin Signal Connection

```
Coin Acceptor COIN ──┬──► 10kΩ Pull-up to +5V
                     │
                     └──► Arduino Pin 2
```

### Arduino Interrupt Code

```cpp
const int COIN_PIN = 2;
volatile int coinPulseCount = 0;

void setup() {
  pinMode(COIN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(COIN_PIN), coinInterrupt, FALLING);
}

void coinInterrupt() {
  coinPulseCount++;
}
```

---

## Complete Wiring Diagram

### System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                    RASPBERRY PI 4B                               │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │        Blazor App (Port 5000)                            │  │
│  │        Python API (Port 5000)                            │  │
│  └───────────────────────┬──────────────────────────────────┘  │
└──────────────────────────┼─────────────────────────────────────┘
                           │ USB Serial
                           ▼
┌─────────────────────────────────────────────────────────────────┐
│                    ARDUINO MEGA 2560                             │
│  Pins:                                                           │
│  • 2      → Coin Acceptor                                       │
│  • 10-11  → Fingerprint Sensor                                  │
│  • 22-34  → Relay Module (13 channels)                          │
│  • 35-44  → Solenoid Drivers (10 channels)                      │
│  • 45-50  → UV Light Drivers (6 channels)                       │
└───┬───────┬──────┬──────┬──────┬──────────────────────────────┘
    │       │      │      │      │
    │       │      │      │      └──► 12V PSU (Solenoids + UV)
    │       │      │      └─────────► Fingerprint Sensor
    │       │      └────────────────► Coin Acceptor (12V)
    │       └───────────────────────► GND (Common)
    └───────────────────────────────► 5V PSU (Arduino + Relays)
```

### Physical Layout Recommendation

```
┌─────────────────────────────────────────────────────────────┐
│                     KIOSK ENCLOSURE                          │
│                                                              │
│  Top Section:                                               │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  Raspberry Pi 4B + 7" Touchscreen                    │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                              │
│  Middle Section (Slots):                                    │
│  ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐         │
│  │ 1  │ │ 2  │ │ 3  │ │ 4  │ │ 5  │ │ 6  │ │ 7  │  ...    │
│  └────┘ └────┘ └────┘ └────┘ └────┘ └────┘ └────┘         │
│                                                              │
│  Bottom Section (Control Box):                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  • Arduino Mega                                       │  │
│  │  • Relay Module (16-channel)                          │  │
│  │  • Power Supplies (5V, 12V)                           │  │
│  │  • TIP120 Driver Circuits                             │  │
│  │  • Terminal Blocks                                     │  │
│  │  • Circuit Breakers & Fuses                           │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                              │
│  Front Panel:                                               │
│  • Coin Slot → Coin Acceptor                               │
│  • Fingerprint Scanner                                      │
└─────────────────────────────────────────────────────────────┘
```

---

## Safety Considerations

### Electrical Safety

#### ⚠️ HIGH VOLTAGE WARNING
- All 220V AC wiring MUST be done by a licensed electrician
- Use proper gauge wire for AC connections (minimum 14 AWG for outlets)
- Install circuit breakers for each power supply
- Use GFCI outlets if near water/moisture
- Ensure proper grounding of all metal enclosures

#### Low Voltage Safety
- Use fuses on 12V lines (10A fast-blow recommended)
- Ensure all ground connections are secure
- Use heat sinks on TIP120 transistors if running continuously
- Provide ventilation in control box

### Component Protection

```
Protection Checklist:
☑ Flyback diodes on all solenoids
☑ Fuses on all power supply outputs
☑ Overcurrent protection on relay module
☑ Heatsinks on driver transistors
☑ Strain relief on all wire connections
☑ Cable management to prevent shorts
☑ Proper wire gauge for current requirements
```

### Recommended Wire Gauges

| Circuit | Current | Wire Gauge (AWG) |
|---------|---------|------------------|
| 220V AC Power | 10A | 14 AWG minimum |
| 12V Solenoid | 0.5A | 20-22 AWG |
| 12V UV Lights | 0.5A | 20-22 AWG |
| 5V Logic | 0.1A | 22-24 AWG |
| Arduino I/O | <20mA | 24-26 AWG |

### Fire Safety

- Use flame-retardant enclosures
- Install smoke detector near electronics
- Ensure adequate ventilation
- Keep flammable materials away
- Have fire extinguisher nearby (Type C for electrical)

### User Safety

1. **UV Light Protection**
   - UV lights must only activate when door is closed
   - Install door switch interlock
   - Use UVC-blocking material on door window

2. **Solenoid Lock Safety**
   - Emergency manual override on all locks
   - Power-off should unlock (fail-safe)
   - Visual indicator when locked

3. **Electrical Protection**
   - All AC outlets must have GFCI protection
   - Ground fault protection on main supply
   - Surge protection on sensitive electronics

---

## Testing Procedure

### Pre-Power Testing

```
1. Visual Inspection
   ☐ All connections secure
   ☐ No exposed wires
   ☐ Proper wire routing
   ☐ All grounds connected

2. Continuity Testing (Power OFF)
   ☐ Test all ground connections
   ☐ Check for shorts between power and ground
   ☐ Verify relay contacts (NO vs NC)

3. Resistance Testing
   ☐ Solenoid resistance (should be 8-20Ω typically)
   ☐ Check for shorts in driver circuits
```

### Power-On Testing (Sequential)

```
Step 1: Power Arduino Only (5V)
   ☐ Check power LED
   ☐ Upload test sketch
   ☐ Serial communication working

Step 2: Add 12V Supply
   ☐ Test one solenoid at a time
   ☐ Test one UV light at a time
   ☐ Check for overheating

Step 3: Test Sensors
   ☐ Fingerprint sensor enrollment
   ☐ Coin acceptor pulse detection

Step 4: Test Relays (NO AC POWER YET)
   ☐ Listen for relay clicks
   ☐ Measure continuity with multimeter

Step 5: AC Power (ELECTRICIAN ONLY)
   ☐ Test with load (lamp) first
   ☐ Verify GFCI operation
   ☐ Check voltage at outlets
```

---

## Troubleshooting

### Common Issues

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| Arduino won't power on | Power supply issue | Check 5V PSU, USB cable |
| Relay not clicking | Wrong pin or bad connection | Verify pin number, check wiring |
| Solenoid not locking | Insufficient voltage | Check 12V supply, transistor |
| UV light dim | Undervoltage | Verify 12V supply, connections |
| Fingerprint not responding | Serial connection | Check TX/RX pins, baud rate |
| Coin acceptor no pulse | Ground issue | Ensure common ground |
| Outlet not working | Relay wiring | Check COM/NO connections |

### Multimeter Measurements

**Normal Values:**
- Arduino 5V pin: 4.8-5.2V
- Arduino 3.3V pin: 3.2-3.4V
- 12V supply: 11.8-12.3V
- Relay coil: ~70Ω resistance
- Solenoid: 8-20Ω resistance
- TIP120 Collector-Emitter (ON): <0.5V
- TIP120 Collector-Emitter (OFF): >11V

---

## Bill of Materials (BOM)

### Electronics

| Item | Quantity | Unit Price (est.) | Total |
|------|----------|-------------------|-------|
| Arduino Mega 2560 | 1 | ₱800 | ₱800 |
| 16-Channel Relay Module | 1 | ₱600 | ₱600 |
| Solenoid Lock 12V | 10 | ₱150 | ₱1,500 |
| UV LED Strip 12V | 6 | ₱200 | ₱1,200 |
| Fingerprint Sensor | 1 | ₱500 | ₱500 |
| Coin Acceptor | 1 | ₱1,500 | ₱1,500 |
| Power Supply 5V 5A | 1 | ₱300 | ₱300 |
| Power Supply 12V 10A | 1 | ₱800 | ₱800 |
| TIP120 Transistor | 20 | ₱15 | ₱300 |
| 1N4007 Diode | 50 | ₱2 | ₱100 |
| Resistors (kit) | 1 | ₱150 | ₱150 |
| Jumper Wires (kit) | 1 | ₱200 | ₱200 |
| Screw Terminals | 20 | ₱10 | ₱200 |
| **Subtotal** | | | **₱8,150** |

### Enclosure & Mounting

| Item | Quantity | Est. Cost |
|------|----------|-----------|
| Metal Enclosure | 1 | ₱3,000 |
| DIN Rail | 2m | ₱200 |
| Cable Glands | 20 | ₱500 |
| Cooling Fan | 2 | ₱300 |
| **Subtotal** | | **₱4,000** |

### **Total Estimated Cost: ₱12,150**

> Note: Prices are estimates in Philippine Pesos and may vary by supplier.

---

## Maintenance Schedule

### Daily
- Visual inspection of display and user interface
- Test one charging slot
- Check for unusual sounds or smells

### Weekly
- Clean fingerprint sensor
- Test coin acceptor with all denominations
- Verify all slots operational
- Check for loose connections

### Monthly
- Clean UV lights
- Test all solenoid locks
- Inspect wiring for wear
- Check ventilation fans
- Update revenue logs

### Quarterly
- Deep clean all components
- Tighten all terminals
- Test emergency shutoffs
- Calibrate coin acceptor
- Replace any worn components

---

## Support and Resources

### Datasheets
- Arduino Mega 2560: https://store.arduino.cc/products/arduino-mega-2560
- TIP120 Transistor: https://www.onsemi.com/pdf/datasheet/tip120-d.pdf
- 1N4007 Diode: Standard rectifier diode datasheet

### Learning Resources
- Arduino Relay Control: https://www.arduino.cc/en/Tutorial/Relay
- Transistor as Switch: https://www.electronics-tutorials.ws/transistor/tran_4.html
- Solenoid Control: https://learn.adafruit.com/adafruit-arduino-lesson-13-dc-motors

### Where to Buy (Philippines)
- Makerlab Electronics
- e-Gizmo
- Lazada / Shopee (search for Arduino components)
- CircuitRocks

---

## Revision History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2025-01-16 | Initial wiring guide |

---

**⚠️ DISCLAIMER**: This wiring guide is provided for educational purposes. Always consult with licensed electricians for high-voltage AC wiring. The authors are not responsible for any damage or injury resulting from improper wiring or installation.

---

For questions or corrections, please refer to the main project README.md or contact the development team.

