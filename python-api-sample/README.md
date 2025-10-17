# Python API Bridge for Arduino Integration

This Python API serves as a bridge between the Blazor application and Arduino Mega, handling hardware control for relays, solenoids, UV lights, fingerprint sensors, and coin acceptors.

## Installation

1. Install Python dependencies:
```bash
pip install -r requirements.txt
```

2. Update the Arduino serial port in `app.py`:
```python
# For Windows
ARDUINO_PORT = 'COM3'

# For Raspberry Pi
ARDUINO_PORT = '/dev/ttyACM0'
```

3. Run the API server:
```bash
python app.py
```

The API will be available at `http://localhost:5000`

## Arduino Setup

1. Install required Arduino libraries:
   - ArduinoJson
   - Adafruit Fingerprint Sensor Library

2. Upload `arduino_sketch.ino` to your Arduino Mega

3. Connect hardware:
   - Relays: Pins 22-34 (slots 1-13)
   - Solenoids: Pins 35-44 (slots 4-13)
   - UV Lights: Pins 45-50 (slots 4-9)
   - Coin Acceptor: Pin 2 (interrupt)
   - Fingerprint Sensor: Pins 10-11 (software serial)

## API Endpoints

### Control Relay
```
POST /api/relay
Body: {
  "slotNumber": 1,
  "state": true
}
```

### Control Solenoid
```
POST /api/solenoid
Body: {
  "slotNumber": 4,
  "lock": true
}
```

### Control UV Light
```
POST /api/uv-light
Body: {
  "slotNumber": 5,
  "state": true
}
```

### Verify Fingerprint
```
POST /api/fingerprint/verify
Body: {
  "fingerprintId": 1
}
Response: {
  "isValid": true
}
```

### Read Coin Value
```
GET /api/coin-slot
Response: {
  "value": 5.0
}
```

### Health Check
```
GET /health
Response: {
  "status": "healthy",
  "arduino_connected": true
}
```

## Running on Raspberry Pi

1. Install Python and dependencies
2. Set up as a systemd service:

Create `/etc/systemd/system/arduino-api.service`:
```ini
[Unit]
Description=Arduino API Bridge
After=network.target

[Service]
WorkingDirectory=/home/pi/python-api-sample
ExecStart=/usr/bin/python3 /home/pi/python-api-sample/app.py
Restart=always
User=pi

[Install]
WantedBy=multi-user.target
```

3. Enable and start:
```bash
sudo systemctl enable arduino-api
sudo systemctl start arduino-api
```

## Troubleshooting

### Serial Port Permission (Linux/Raspberry Pi)
```bash
sudo usermod -a -G dialout $USER
sudo chmod 666 /dev/ttyACM0
```

### Check Arduino Connection
```bash
ls /dev/tty*
```

### View Logs
```bash
journalctl -u arduino-api -f
```

## Testing Without Hardware

The API can run in simulation mode without an Arduino connected. It will log commands and return successful responses for testing the Blazor application.

