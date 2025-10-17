# Solar Charging Station Kiosk

A modern, smart charging station application built with Blazor and MudBlazor, designed to run on Raspberry Pi 4B. This system manages charging slots with integrated security features including fingerprint authentication, UV sanitization, and automatic door locking.

## Features

### 🔌 Three Types of Charging Slots

1. **Open Slots (1-3)**
   - Simple plug-and-charge functionality
   - No security features
   - Relay-controlled power outlets

2. **Phone Slots (4-9)**
   - Fingerprint authentication
   - UV light sanitization (15 seconds)
   - Automatic solenoid door lock
   - Coin-operated charging

3. **Laptop Slots (10-13)**
   - Fingerprint authentication
   - Automatic solenoid door lock
   - Coin-operated charging
   - No UV sanitization

### 📊 Admin Panel

- **Revenue Tracking**
  - Daily, Monthly, and Yearly reports
  - Revenue breakdown by slot type
  - Transaction history

- **Coin Management**
  - Configurable coin denominations
  - Daily/Monthly/Yearly coin counting
  - Active/Inactive denomination management

### 🎨 Modern UI

- Built with MudBlazor for a clean, modern interface
- Bootstrap 5 for responsive design
- Gradient backgrounds and smooth animations
- Mobile-friendly design

## Technology Stack

- **Frontend**: Blazor Server (.NET 9.0)
- **UI Framework**: MudBlazor + Bootstrap 5
- **Database**: SQLite with Entity Framework Core
- **Authentication**: ASP.NET Core Identity
- **Hardware Integration**: Python API + Arduino Mega

## Project Structure

```
ChargingKiosk/
├── Components/
│   ├── Pages/
│   │   ├── Home.razor          # Welcome screen
│   │   ├── Slots.razor         # Slot selection screen
│   │   ├── SlotControl.razor   # Individual slot control
│   │   └── Admin.razor         # Admin panel
│   └── Layout/
│       ├── MainLayout.razor    # Main layout
│       └── NavMenu.razor       # Navigation menu
├── Models/
│   ├── ChargingSlot.cs         # Slot model
│   ├── Transaction.cs          # Transaction model
│   ├── CoinDenomination.cs     # Coin denomination model
│   ├── SlotType.cs             # Slot type enum
│   └── SlotStatus.cs           # Slot status enum
├── Services/
│   ├── ISlotService.cs         # Slot management interface
│   ├── SlotService.cs          # Slot management implementation
│   ├── IArduinoApiService.cs   # Arduino API interface
│   ├── ArduinoApiService.cs    # Arduino API implementation
│   ├── IInventoryService.cs    # Inventory interface
│   └── InventoryService.cs     # Inventory implementation
└── Data/
    ├── ApplicationDbContext.cs # Database context
    └── Migrations/             # EF Core migrations
```

## Hardware Requirements

- Raspberry Pi 4B (or compatible)
- Arduino Mega
- Fingerprint sensor (compatible with Arduino)
- UV light modules (for phone slots)
- Solenoid locks (for secure slots)
- Relays for power control
- Coin acceptor/validator

## Installation

### Prerequisites

1. .NET 9.0 SDK
2. SQLite
3. Python 3.x (for Arduino API bridge)

### Steps

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd ChargingKiosk
   ```

2. **Restore NuGet packages**
   ```bash
   cd ChargingKiosk
   dotnet restore
   ```

3. **Apply database migrations**
   ```bash
   dotnet ef database update
   ```

4. **Configure Python API**
   - Update `appsettings.json` with your Python API URL:
   ```json
   "PythonApi": {
     "BaseUrl": "http://localhost:5000"
   }
   ```

5. **Run the application**
   ```bash
   dotnet run
   ```

6. **Access the application**
   - Open browser: `http://localhost:5000`
   - Or configure for Raspberry Pi deployment

## Python API Integration

The application communicates with Arduino through a Python API. Create a Flask/FastAPI server with the following endpoints:

### Required Endpoints

```python
POST /api/relay
{
  "slotNumber": 1,
  "state": true
}

POST /api/solenoid
{
  "slotNumber": 4,
  "lock": true
}

POST /api/uv-light
{
  "slotNumber": 5,
  "state": true
}

POST /api/fingerprint/verify
{
  "fingerprintId": 1
}

GET /api/coin-slot
Response: {
  "value": 5.00
}
```

See `python-api-sample/` directory for a complete implementation example.

## Configuration

### Coin Denominations

Default coin denominations (can be modified in Admin Panel):
- 1 Peso
- 5 Pesos
- 10 Pesos
- 20 Pesos

### Slot Configuration

Slots are initialized on service startup:
- Slots 1-3: Open (no security)
- Slots 4-9: Phone (with UV)
- Slots 10-13: Laptop (no UV)

To modify slot configuration, edit `Services/SlotService.cs`.

## Raspberry Pi Deployment

### 1. Publish the Application

```bash
dotnet publish -c Release -r linux-arm64 --self-contained
```

### 2. Copy to Raspberry Pi

```bash
scp -r bin/Release/net9.0/linux-arm64/publish/* solaradmin@solar.local:/home/sloradmin/charging-kiosk/ui
```

### 3. Set up as a Service

Create `/etc/systemd/system/charging-kiosk.service`:

```ini
[Unit]
Description=Solar Charging Station Kiosk
After=network.target

[Service]
WorkingDirectory=/home/pi/charging-kiosk
ExecStart=/home/pi/charging-kiosk/ChargingKiosk
Restart=always
RestartSec=10
User=pi

[Install]
WantedBy=multi-user.target
```

Enable and start:
```bash
sudo systemctl enable charging-kiosk
sudo systemctl start charging-kiosk
```

## Default Credentials

To create an admin account, register through the UI and manually add to admin role in database, or seed an admin user in the database migration.

## Security Considerations

1. Change default admin credentials
2. Enable HTTPS in production
3. Configure firewall rules on Raspberry Pi
4. Secure the Python API endpoints
5. Implement proper authentication for Arduino communication

## Troubleshooting

### Database Issues
```bash
# Reset database
rm Data/app.db
dotnet ef database update
```

### Service Not Starting
```bash
# Check logs
journalctl -u charging-kiosk -f
```

### Arduino Connection Issues
- Verify Python API is running
- Check `appsettings.json` for correct API URL
- Test endpoints manually with curl/Postman

## Future Enhancements

- [ ] Mobile app for remote monitoring
- [ ] QR code payment integration
- [ ] Email notifications for low funds
- [ ] Advanced analytics and reporting
- [ ] Multi-language support
- [ ] Cloud backup for transactions

## License

This project is licensed under the MIT License.

## Support

For issues and questions, please open an issue on the GitHub repository.

## Contributors

- Your Name/Team

---

Built with ❤️ using .NET, Blazor, and MudBlazor

