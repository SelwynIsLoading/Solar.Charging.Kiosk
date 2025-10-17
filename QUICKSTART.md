# Quick Start Guide - Solar Charging Station

## What's Been Built

A complete smart charging station kiosk system with:

### ✅ Core Features
- **Welcome Screen**: Beautiful gradient welcome page with "Continue" button
- **Slot Selection Screen**: Card-based layout showing all 13 charging slots
- **Slot Control Screen**: Individual charging control with coin insertion and fingerprint authentication
- **Admin Panel**: Revenue tracking and coin denomination management

### ✅ Three Slot Types

1. **Open Slots (1-3)**: Simple plug-and-charge
2. **Phone Slots (4-9)**: With fingerprint, UV sanitization (15s), and auto-lock
3. **Laptop Slots (10-13)**: With fingerprint and auto-lock (no UV)

### ✅ Technology Stack
- Blazor Server with .NET 9.0
- MudBlazor for modern UI
- Bootstrap 5 for responsive design
- SQLite database with EF Core
- Python API bridge for Arduino integration

## Running the Application

### 1. Build and Run

```bash
cd ChargingKiosk
dotnet restore
dotnet build
dotnet run
```

Open browser to: `https://localhost:7000` or `http://localhost:5000`

### 2. Start Python API (Optional for hardware)

```bash
cd python-api-sample
pip install -r requirements.txt
python app.py
```

This starts the Arduino bridge at `http://localhost:5000`

### 3. First Time Setup

1. The database will be created automatically on first run
2. Coin denominations will be seeded (1, 5, 10, 20 pesos)
3. Register an admin account via the login page

## User Flow

### Customer Journey

1. **Welcome Screen** → Click "Continue"
2. **Slot Selection** → Choose an available slot
3. **Coin Insertion** → Insert coins (simulated with button)
4. **Fingerprint Scan** (Phone/Laptop slots only) → Scan fingerprint
5. **Start Charging** → Click "Start Charging"
6. **UV Sanitization** (Phone slots only) → Wait 15 seconds
7. **Charging** → Device is now charging in locked compartment
8. **Stop Charging** → Click "Stop Charging" when done

### Admin Journey

1. **Login** → Use admin credentials
2. **Admin Panel** → View from navigation menu
3. **View Revenue** → See daily/monthly/yearly breakdowns
4. **Manage Coins** → Edit coin denominations and view counts
5. **Transaction History** → See all charging sessions

## Testing Without Hardware

The system works in simulation mode without actual hardware:

- **Coin Insertion**: Click "Insert Coin" button (adds ₱5)
- **Fingerprint**: Click "Scan Fingerprint" (always succeeds)
- **Relays/Solenoids**: Logged in console
- **UV Light**: 15-second countdown displayed

## Customization

### Change Slot Configuration

Edit `ChargingKiosk/Services/SlotService.cs`:

```csharp
private List<ChargingSlot> InitializeSlots()
{
    var slots = new List<ChargingSlot>();
    
    // Modify slot counts and types here
    for (int i = 1; i <= 5; i++)  // Change to 5 open slots
    {
        slots.Add(new ChargingSlot
        {
            SlotNumber = i,
            Type = SlotType.Open,
            Status = SlotStatus.Available
        });
    }
    
    return slots;
}
```

### Change Coin Denominations

1. Go to Admin Panel → Coin Denominations tab
2. Click Edit icon next to denomination
3. Modify values (future feature - currently in UI)

Or edit database directly:
```sql
UPDATE CoinDenominations SET Value = 25.00 WHERE Name = '20 Pesos';
```

### Update Python API URL

Edit `ChargingKiosk/appsettings.json`:

```json
"PythonApi": {
  "BaseUrl": "http://raspberrypi.local:5000"
}
```

## Raspberry Pi Deployment

### Quick Deploy

```bash
# On development machine
dotnet publish -c Release -r linux-arm64 --self-contained

# Copy to Raspberry Pi
scp -r bin/Release/net9.0/linux-arm64/publish/* pi@raspberrypi:/home/pi/charging-kiosk

# On Raspberry Pi
chmod +x /home/pi/charging-kiosk/ChargingKiosk
/home/pi/charging-kiosk/ChargingKiosk
```

### Set as System Service

See `README.md` for complete systemd service setup.

## Arduino Hardware Setup

### Pin Assignments

**Relays (Slots 1-13)**: Pins 22-34
**Solenoids (Slots 4-13)**: Pins 35-44
**UV Lights (Slots 4-9)**: Pins 45-50
**Coin Acceptor**: Pin 2 (interrupt)
**Fingerprint Sensor**: Pins 10-11 (software serial)

### Upload Sketch

1. Open Arduino IDE
2. Install libraries: ArduinoJson, Adafruit Fingerprint
3. Open `python-api-sample/arduino_sketch.ino`
4. Select Arduino Mega board
5. Upload

## Troubleshooting

### App Won't Start

```bash
# Check if ports are in use
netstat -ano | findstr :5000

# Reset database
rm ChargingKiosk/Data/app.db
dotnet ef database update
```

### Python API Connection Failed

- Check `appsettings.json` has correct URL
- Ensure Python API is running
- Test endpoint: `curl http://localhost:5000/health`

### Slots Not Updating

- Check browser console for errors
- Verify ISlotService is registered in DI
- Restart application

## Project Structure Reference

```
ChargingKiosk/
├── Components/Pages/       # UI pages
│   ├── Home.razor         # Welcome screen
│   ├── Slots.razor        # Slot selection
│   ├── SlotControl.razor  # Individual slot
│   └── Admin.razor        # Admin panel
├── Models/                # Data models
├── Services/              # Business logic
├── Data/                  # Database context
└── wwwroot/              # Static files

python-api-sample/
├── app.py                 # Flask API server
├── arduino_sketch.ino     # Arduino code
└── requirements.txt       # Python dependencies
```

## Next Steps

1. ✅ Application is ready to run
2. 🔧 Configure hardware (Arduino, sensors, locks)
3. 🎨 Customize UI colors/branding if needed
4. 🔐 Set up admin accounts
5. 📊 Test transaction flow
6. 🚀 Deploy to Raspberry Pi

## Support

- Check `README.md` for detailed documentation
- Review `python-api-sample/README.md` for Arduino setup
- Open issues for bugs or questions

---

**You're all set!** The application is fully functional and ready for testing. 🎉

