# Quick Reference Guide - Charging Kiosk Operation

## System Overview

The charging kiosk has **3 types of slots**:
- **Open Slots (1-3)**: Simple plug and charge
- **Phone Slots (4-9)**: Secure charging with UV sanitization
- **Laptop Slots (10-13)**: Secure charging without UV

---

## For Users

### Using Open Slots (1-3)

1. **Select Slot** → Click on available open slot
2. **Insert Coins** → Put coins in acceptor
3. **Start Charging** → Click "Start Charging" button
4. **Stop Anytime** → Click "Stop Charging" to end

**Features**: No security, no lock, just plug and charge!

---

### Using Phone Slots (4-9)

#### Step 1: Payment
1. **Select Slot** → Click on available phone slot
2. **Insert Coins** → Coin acceptor activates automatically
3. **Wait** → System calculates charging time

#### Step 2: Security Setup
1. **Enroll Fingerprint** → Click "Enroll Fingerprint"
2. **Place Finger** → Put finger on sensor
3. **Remove Finger** → Lift finger off sensor
4. **Place Again** → Put same finger on sensor
5. **Success!** → Fingerprint enrolled

#### Step 3: Charging Starts
1. **UV Light** → Automatically turns on for 15 seconds
2. **Phone Sanitized** → UV light turns off
3. **Door Locks** → Slot locks automatically
4. **Charging Begins** → Power relay turns on

#### Step 4: During Charging
**Need to Check Your Phone?**
1. Click **"Access Device (2s unlock)"**
2. Scan your fingerprint
3. Door unlocks for 2 seconds
4. Check your phone
5. Door automatically re-locks
6. Charging continues!

#### Step 5: Ending Session
**Ready to Leave?**
1. Click **"End Session & Unlock"**
2. Scan your fingerprint
3. Door unlocks permanently
4. Power turns off
5. Take your phone and go!

---

### Using Laptop Slots (10-13)

**Same as Phone Slots, but NO UV sanitization!**

1. Insert coins
2. Enroll fingerprint
3. Slot locks and charging starts (no UV)
4. Access device anytime with fingerprint (2s unlock)
5. End session with fingerprint

---

## For Operators

### Daily Startup

1. **Power On Arduino Mega**
   ```bash
   # Check Arduino connected to Raspberry Pi
   ls /dev/ttyACM*
   ```

2. **Start Python API**
   ```bash
   cd python-api-sample
   python app.py
   # Should see: "Connected to Arduino on /dev/ttyACM0"
   ```

3. **Start Blazor App**
   ```bash
   cd ChargingKiosk
   dotnet run
   # Should see: "Now listening on: http://localhost:5000"
   ```

4. **Verify System**
   - Open browser: `http://localhost:5000`
   - Check all slots show "Available"
   - Test one slot of each type

---

### System Check

**Health Check**:
```bash
curl http://localhost:5000/health
```

**Expected Response**:
```json
{
  "status": "healthy",
  "arduino_connected": true
}
```

---

### Common Issues

#### Issue: Coins Not Detected
**Quick Fix**:
1. Check coin acceptor power (12V)
2. Check connection to Arduino Pin 2
3. Check Arduino Serial Monitor for pulses
4. Test manual simulation in UI

#### Issue: Fingerprint Not Working
**Quick Fix**:
1. Check AS608 sensor wiring (TX→Pin11, RX→Pin10)
2. Restart Python API
3. Test enrollment with different finger
4. Check sensor is powered (5V)

#### Issue: UV Light Not Working
**Quick Fix**:
1. Check relay connections (Pins 45-50)
2. Check UV lamp power supply
3. Test manual control via API:
   ```bash
   curl -X POST http://localhost:5000/api/uv-light \
     -H "Content-Type: application/json" \
     -d '{"slotNumber": 4, "state": true}'
   ```

#### Issue: Door Won't Unlock
**Quick Fix**:
1. Check solenoid power (12V)
2. Check wiring (Pins 35-44)
3. Test manual unlock:
   ```bash
   curl -X POST http://localhost:5000/api/solenoid \
     -H "Content-Type: application/json" \
     -d '{"slotNumber": 4, "locked": false}'
   ```

---

### Slot Status Reference

| Status | Meaning | Color | Action |
|--------|---------|-------|--------|
| Available | Ready to use | Green | Users can select |
| In Use | Currently charging | Yellow | Show remaining time |
| Sanitizing | UV light active | Blue | Wait 15 seconds |
| Locked | Secured with fingerprint | Purple | Only owner can access |
| Out of Service | Maintenance mode | Red | Contact admin |

---

### Monitoring Tips

**Watch Arduino Serial Monitor**:
```
✓ Good: {"coinDetected":5.00,"pulses":5,"timestamp":12345}
✗ Bad:  {"warning":"Unknown pulse count","pulses":25}
```

**Watch Python Console**:
```
✓ Good: 💰 Coin detected: ₱5.00
✓ Good: ✓ Fingerprint verified!
✗ Bad:  ❌ Arduino communication error
```

**Watch Blazor Logs**:
```
✓ Good: Relay for slot 4 turned ON
✓ Good: UV sanitization completed for slot 4
✗ Bad:  Failed to control relay for slot 4
```

---

### Emergency Procedures

#### Emergency Stop
```bash
# Stop all relays
for i in {1..13}; do
  curl -X POST http://localhost:5000/api/relay \
    -H "Content-Type: application/json" \
    -d "{\"slotNumber\": $i, \"state\": false}"
done
```

#### Emergency Unlock All
```bash
# Unlock all solenoids
for i in {4..13}; do
  curl -X POST http://localhost:5000/api/solenoid \
    -H "Content-Type: application/json" \
    -d "{\"slotNumber\": $i, \"locked\": false}"
done
```

#### Restart System
```bash
# 1. Stop services
pkill -f "python app.py"
pkill -f "dotnet run"

# 2. Wait 5 seconds
sleep 5

# 3. Restart Arduino (power cycle)

# 4. Restart services
cd /path/to/python-api-sample && python app.py &
cd /path/to/ChargingKiosk && dotnet run &
```

---

### Maintenance Schedule

**Daily**:
- [ ] Check all slots functional
- [ ] Test coin acceptor with each denomination
- [ ] Verify fingerprint sensor working
- [ ] Clean UV lights (if dusty)

**Weekly**:
- [ ] Check all wiring connections
- [ ] Clean coin acceptor mechanism
- [ ] Test emergency procedures
- [ ] Backup transaction logs

**Monthly**:
- [ ] Deep clean all components
- [ ] Check power supplies
- [ ] Calibrate coin acceptor if needed
- [ ] Update software if available

---

### Configuration Files

**Python API Port** (`app.py`):
```python
ARDUINO_PORT = '/dev/ttyACM0'  # Raspberry Pi
# ARDUINO_PORT = 'COM3'  # Windows
```

**Blazor API URL** (`appsettings.json`):
```json
{
  "PythonApi": {
    "BaseUrl": "http://localhost:5000"
  }
}
```

**Coin Rates** (Database):
- Edit via Admin panel
- Default: ₱5 = 10 minutes

---

### Contact Information

**Technical Support**:
- Arduino Issues: Check ARDUINO_CHECKLIST.md
- Coin Detection: Check COIN_DETECTION_IMPROVEMENTS.md
- Fingerprint Setup: Check FINGERPRINT_SETUP.md
- Deployment: Check DEPLOYMENT.md

---

## Coin Denominations

| Coin | Pulses | Value | Default Minutes |
|------|--------|-------|-----------------|
| ₱1   | 1      | ₱1.00 | 2 min |
| ₱5   | 5      | ₱5.00 | 10 min |
| ₱10  | 10     | ₱10.00| 20 min |
| ₱20  | 20     | ₱20.00| 40 min |

*Rates can be configured in Admin panel*

---

## API Quick Reference

### Test Relay
```bash
curl -X POST http://localhost:5000/api/relay \
  -H "Content-Type: application/json" \
  -d '{"slotNumber": 1, "state": true}'
```

### Test Solenoid
```bash
curl -X POST http://localhost:5000/api/solenoid \
  -H "Content-Type: application/json" \
  -d '{"slotNumber": 4, "locked": true}'
```

### Test UV Light
```bash
curl -X POST http://localhost:5000/api/uv-light \
  -H "Content-Type: application/json" \
  -d '{"slotNumber": 4, "state": true}'
```

### Test Temporary Unlock
```bash
curl -X POST http://localhost:5000/api/solenoid/unlock-temp \
  -H "Content-Type: application/json" \
  -d '{"slotNumber": 4}'
```

### Check Coin Slot
```bash
curl http://localhost:5000/api/coin-slot
```

### Health Check
```bash
curl http://localhost:5000/health
```

---

## Troubleshooting Decision Tree

```
Problem?
│
├─ Coins not detected?
│  ├─ Check power to coin acceptor
│  ├─ Check Arduino Pin 2 connection
│  └─ Monitor Arduino Serial for pulses
│
├─ Fingerprint not working?
│  ├─ Check AS608 wiring (Pins 10, 11)
│  ├─ Restart Python API
│  └─ Try different finger
│
├─ Door won't unlock?
│  ├─ Check solenoid power
│  ├─ Test via API manually
│  └─ Check fingerprint verification
│
├─ UV light not working?
│  ├─ Check relay connection
│  ├─ Test via API manually
│  └─ Check UV lamp power
│
└─ System not responding?
   ├─ Check Python API running
   ├─ Check Arduino connection
   └─ Restart all services
```

---

## Safety Notes

⚠️ **UV Light Safety**:
- UV light is contained in closed compartment
- Only activates when door is closed
- Automatic 15-second shutoff
- Do not look directly at UV light

⚠️ **Electrical Safety**:
- Turn off power before maintenance
- Check for loose connections regularly
- Keep water away from electronics
- Use proper fuses/circuit breakers

⚠️ **Mechanical Safety**:
- Keep fingers away from solenoid locks
- Check for pinch points
- Regular inspection of moving parts

---

---

## 💰 Admin Dashboard - Sales Tracking

**NOW WORKING!** Transactions are automatically saved when users start charging.

### Accessing Admin Panel
1. Login with admin credentials
2. Click "Admin Panel" in navigation menu
3. View revenue and transactions

### What You'll See

**Revenue Cards**:
- 💵 Daily Revenue - Today's total
- 💵 Monthly Revenue - This month's total  
- 💵 Yearly Revenue - This year's total

**Transaction History**:
- All transactions with details
- Slot number and type
- Start/End times
- Amount paid
- "In Progress" for active sessions

### Quick Check
```
After user pays ₱20 and starts charging:
1. Go to /admin
2. Click "Transaction History" tab
3. Should see: Slot X - ₱20.00 - "In Progress"
4. After user stops:
5. Click "Refresh"
6. Should see: Slot X - ₱20.00 - With end time ✅
```

---

**Last Updated**: October 17, 2025  
**Version**: 2.3 (Sales Tracking Fixed)  
**For Support**: Check SYSTEM_IMPROVEMENTS.md and SALES_TRACKING_FEATURE.md

