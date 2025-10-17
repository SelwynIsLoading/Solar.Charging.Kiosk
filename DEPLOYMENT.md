# Deployment Guide - Solar Charging Station

Complete guide for deploying to Raspberry Pi 4B.

## Prerequisites

- ✅ .NET application built and published
- ✅ Raspberry Pi 4B with Raspberry Pi OS
- ✅ SSH access to Raspberry Pi
- ✅ Python API ready

---

## Step 1: Publish the .NET Application

From your development machine (Windows):

```powershell
# Navigate to project directory
cd "C:\Users\cauba\Documents\selwyn dev\2025\ChargingKiosk"

# Publish for Raspberry Pi (ARM64)
dotnet publish .\ChargingKiosk\ChargingKiosk.csproj -c Release -r linux-arm64 --self-contained
```

**Output location:**
```
ChargingKiosk\bin\Release\net9.0\linux-arm64\publish\
```

---

## Step 2: Copy to Raspberry Pi

### Method 1: Using SCP (Secure Copy)

**From PowerShell/Command Prompt:**

```powershell
# Basic syntax
scp -r ".\ChargingKiosk\bin\Release\net9.0\linux-arm64\publish\*" pi@raspberrypi.local:/home/pi/charging-kiosk/ui

# Or with custom hostname
scp -r ".\ChargingKiosk\bin\Release\net9.0\linux-arm64\publish\*" solaradmin@solar.local:/home/solaradmin/charging-kiosk/ui

# Or with IP address
scp -r ".\ChargingKiosk\bin\Release\net9.0\linux-arm64\publish\*" pi@192.168.1.100:/home/pi/charging-kiosk/ui
```

**Common Issues:**

If you get "Host key verification failed":
```powershell
# Add to known hosts
ssh pi@raspberrypi.local
# Type 'yes' when prompted, then exit
exit

# Now try scp again
```

If you get "Permission denied":
```bash
# On Raspberry Pi, create directory first:
ssh pi@raspberrypi.local
mkdir -p ~/charging-kiosk/ui
exit

# Then try scp again
```

### Method 2: Using WinSCP (Windows GUI)

1. Download WinSCP: https://winscp.net/
2. Install and open WinSCP
3. Connect to Raspberry Pi:
   - Host: `raspberrypi.local` or `192.168.1.100`
   - Username: `pi` (or your username)
   - Password: (your Pi password)
4. Navigate to local: `ChargingKiosk\bin\Release\net9.0\linux-arm64\publish\`
5. Navigate to remote: `/home/pi/charging-kiosk/ui/`
6. Drag and drop all files from left to right

### Method 3: Using FileZilla

1. Download FileZilla: https://filezilla-project.org/
2. Use SFTP protocol
3. Connect and transfer files

---

## Step 3: Copy Python API

```powershell
# Copy Python API files
scp -r ".\python-api-sample\*" pi@raspberrypi.local:/home/pi/charging-kiosk/api
```

---

## Step 4: Setup on Raspberry Pi

SSH into your Raspberry Pi:

```bash
ssh pi@raspberrypi.local
# or
ssh solaradmin@solar.local
# or
ssh pi@192.168.1.100
```

### Install .NET Runtime (if not already installed)

```bash
# Check if .NET is installed
dotnet --version

# If not installed:
wget https://dot.net/v1/dotnet-install.sh
chmod +x dotnet-install.sh
sudo ./dotnet-install.sh --channel 9.0 --runtime aspnetcore

# Or use apt (Raspberry Pi OS)
sudo apt update
sudo apt install -y aspnetcore-runtime-9.0
```

### Install Python Dependencies

```bash
cd /home/pi/charging-kiosk/api

# Option 1: Use system packages (recommended)
sudo apt update
sudo apt install -y python3-flask python3-flask-cors python3-serial

# Option 2: Use pip with --break-system-packages
pip3 install -r requirements.txt --break-system-packages
```

### Set Permissions

```bash
# Make the .NET app executable
chmod +x /home/pi/charging-kiosk/ui/ChargingKiosk

# Set permissions for serial port (Arduino connection)
sudo usermod -a -G dialout $USER
sudo chmod 666 /dev/ttyACM0  # or your Arduino port
```

---

## Step 5: Test Run

### Test .NET Application

```bash
cd /home/pi/charging-kiosk/ui
./ChargingKiosk
```

**Expected output:**
```
info: Microsoft.Hosting.Lifetime[14]
      Now listening on: http://0.0.0.0:5000
info: Microsoft.Hosting.Lifetime[0]
      Application started.
```

**Access from browser:**
- On Pi: `http://localhost:5000`
- From network: `http://raspberrypi.local:5000`

Press `Ctrl+C` to stop

### Test Python API

```bash
cd /home/pi/charging-kiosk/api
python3 app.py
```

**Expected output:**
```
Starting Solar Charging Station Python API...
Arduino connection: Simulated mode (or Connected)
* Running on http://0.0.0.0:5000
```

**Test endpoint:**
```bash
curl http://localhost:5000/health
```

Press `Ctrl+C` to stop

---

## Step 6: Create System Services

### Create .NET Service

```bash
sudo nano /etc/systemd/system/charging-ui.service
```

**Content:**
```ini
[Unit]
Description=Solar Charging Station UI
After=network.target

[Service]
WorkingDirectory=/home/pi/charging-kiosk/ui
ExecStart=/home/pi/charging-kiosk/ui/ChargingKiosk
Restart=always
RestartSec=10
User=pi
Environment=ASPNETCORE_ENVIRONMENT=Production
Environment=ASPNETCORE_URLS=http://0.0.0.0:5000

[Install]
WantedBy=multi-user.target
```

### Create Python API Service

```bash
sudo nano /etc/systemd/system/charging-api.service
```

**Content:**
```ini
[Unit]
Description=Solar Charging Station Python API
After=network.target
Before=charging-ui.service

[Service]
WorkingDirectory=/home/pi/charging-kiosk/api
ExecStart=/usr/bin/python3 /home/pi/charging-kiosk/api/app.py
Restart=always
RestartSec=10
User=pi
Environment=PYTHONUNBUFFERED=1

[Install]
WantedBy=multi-user.target
```

### Enable and Start Services

```bash
# Reload systemd
sudo systemctl daemon-reload

# Enable services (auto-start on boot)
sudo systemctl enable charging-api
sudo systemctl enable charging-ui

# Start services
sudo systemctl start charging-api
sudo systemctl start charging-ui

# Check status
sudo systemctl status charging-api
sudo systemctl status charging-ui
```

---

## Step 7: Configure Networking

### Allow Firewall (if enabled)

```bash
# Allow HTTP traffic
sudo ufw allow 5000/tcp
sudo ufw allow 8080/tcp
```

### Get Raspberry Pi IP Address

```bash
hostname -I
# or
ip addr show
```

### Access from Other Devices

From any device on your network:
```
http://192.168.1.100:5000
# or
http://raspberrypi.local:5000
# or
http://solar.local:5000
```

---

## Step 8: Configure Auto-Start on Boot

### Update appsettings.json on Pi

```bash
cd /home/pi/charging-kiosk/ui
nano appsettings.json
```

Update Python API URL:
```json
{
  "PythonApi": {
    "BaseUrl": "http://localhost:5000"
  }
}
```

### Set Python API Port

Edit `app.py` if needed to avoid port conflict:

```bash
nano /home/pi/charging-kiosk/api/app.py
```

Change to different port if 5000 is used:
```python
app.run(host='0.0.0.0', port=5001, debug=False)  # Use port 5001
```

Then update the .NET appsettings.json:
```json
"PythonApi": {
  "BaseUrl": "http://localhost:5001"
}
```

---

## Step 9: Database Migration

```bash
cd /home/pi/charging-kiosk/ui

# Create database (first run)
./ChargingKiosk

# Or manually run migrations
dotnet ef database update
```

---

## Monitoring and Logs

### View Service Logs

```bash
# Python API logs
sudo journalctl -u charging-api -f

# .NET UI logs
sudo journalctl -u charging-ui -f

# Both at once
sudo journalctl -u charging-api -u charging-ui -f
```

### Restart Services

```bash
sudo systemctl restart charging-api
sudo systemctl restart charging-ui
```

### Stop Services

```bash
sudo systemctl stop charging-api
sudo systemctl stop charging-ui
```

---

## Updating the Application

### Update .NET Application

**On development machine:**
```powershell
# 1. Make changes
# 2. Re-publish
dotnet publish .\ChargingKiosk\ChargingKiosk.csproj -c Release -r linux-arm64 --self-contained

# 3. Copy to Pi (overwrite)
scp -r ".\ChargingKiosk\bin\Release\net9.0\linux-arm64\publish\*" pi@raspberrypi.local:/home/pi/charging-kiosk/ui
```

**On Raspberry Pi:**
```bash
# Restart service to pick up changes
sudo systemctl restart charging-ui
```

### Update Python API

```powershell
# Copy updated Python files
scp ".\python-api-sample\app.py" pi@raspberrypi.local:/home/pi/charging-kiosk/api/
```

```bash
# On Pi: Restart service
sudo systemctl restart charging-api
```

---

## Troubleshooting

### .NET App Won't Start

```bash
# Check logs
sudo journalctl -u charging-ui -n 50

# Check if port is in use
sudo netstat -tulpn | grep :5000

# Test manually
cd /home/pi/charging-kiosk/ui
./ChargingKiosk
```

### Python API Won't Start

```bash
# Check logs
sudo journalctl -u charging-api -n 50

# Test manually
cd /home/pi/charging-kiosk/api
python3 app.py
```

### Can't Access from Network

```bash
# Check if service is listening
sudo netstat -tulpn | grep ChargingKiosk

# Test locally first
curl http://localhost:5000

# Check firewall
sudo ufw status
```

### Permission Errors

```bash
# Fix ownership
sudo chown -R pi:pi /home/pi/charging-kiosk

# Fix permissions
chmod -R 755 /home/pi/charging-kiosk
chmod +x /home/pi/charging-kiosk/ui/ChargingKiosk
```

---

## Quick Deployment Script

Save this as `deploy.ps1` on your Windows machine:

```powershell
# Quick deployment script
$piHost = "pi@raspberrypi.local"  # Change this
$projectPath = ".\ChargingKiosk\ChargingKiosk.csproj"

Write-Host "Publishing application..." -ForegroundColor Green
dotnet publish $projectPath -c Release -r linux-arm64 --self-contained

Write-Host "Copying to Raspberry Pi..." -ForegroundColor Green
scp -r ".\ChargingKiosk\bin\Release\net9.0\linux-arm64\publish\*" "$piHost:/home/pi/charging-kiosk/ui/"
scp -r ".\python-api-sample\*" "$piHost:/home/pi/charging-kiosk/api/"

Write-Host "Restarting services on Pi..." -ForegroundColor Green
ssh $piHost "sudo systemctl restart charging-api && sudo systemctl restart charging-ui"

Write-Host "Deployment complete!" -ForegroundColor Green
Write-Host "Access at: http://raspberrypi.local:5000" -ForegroundColor Cyan
```

Run with:
```powershell
.\deploy.ps1
```

---

## Performance Tips

### For Raspberry Pi 4B

1. **Increase swap space** (if running low on memory):
```bash
sudo dphys-swapfile swapoff
sudo nano /etc/dphys-swapfile
# Change CONF_SWAPSIZE=100 to CONF_SWAPSIZE=1024
sudo dphys-swapfile setup
sudo dphys-swapfile swapon
```

2. **Optimize database**:
```bash
sqlite3 /home/pi/charging-kiosk/ui/Data/app.db "VACUUM;"
```

3. **Monitor resources**:
```bash
htop
# or
free -h
df -h
```

---

## Security Recommendations

### Change Default Credentials

Create admin user in database or through UI.

### Enable HTTPS (Optional)

```bash
# Generate self-signed certificate
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes

# Update appsettings.json to use HTTPS
```

### Lock Down SSH

```bash
# Disable password authentication (use key-based only)
sudo nano /etc/ssh/sshd_config
# Set: PasswordAuthentication no
sudo systemctl restart ssh
```

---

## Backup Strategy

### Backup Database

```bash
# Manual backup
cp /home/pi/charging-kiosk/ui/Data/app.db ~/backups/app.db.$(date +%Y%m%d)

# Automated daily backup (cron)
crontab -e
# Add line:
0 2 * * * cp /home/pi/charging-kiosk/ui/Data/app.db ~/backups/app.db.$(date +\%Y\%m\%d)
```

### Backup Configuration

```bash
# Backup entire charging-kiosk directory
tar -czf charging-kiosk-backup.tar.gz /home/pi/charging-kiosk
```

---

## Current Deployment Status

✅ **Application Published Successfully!**

**Published to:**
```
C:\Users\cauba\Documents\selwyn dev\2025\ChargingKiosk\
  └─ ChargingKiosk\bin\Release\net9.0\linux-arm64\publish\
```

**Next steps:**

1. Copy to Pi (use command below)
2. Set up services
3. Test and verify

---

## Ready-to-Use Commands

### Copy UI to Pi

```powershell
scp -r ".\ChargingKiosk\bin\Release\net9.0\linux-arm64\publish\*" pi@raspberrypi.local:/home/pi/charging-kiosk/ui/
```

### Copy Python API to Pi

```powershell
scp -r ".\python-api-sample\*" pi@raspberrypi.local:/home/pi/charging-kiosk/api/
```

### Set Up Everything (SSH into Pi first)

```bash
# Create directories
mkdir -p ~/charging-kiosk/ui
mkdir -p ~/charging-kiosk/api

# After copying files:
chmod +x ~/charging-kiosk/ui/ChargingKiosk

# Install Python packages
cd ~/charging-kiosk/api
sudo apt install -y python3-flask python3-flask-cors python3-serial

# Create and enable services (see Step 6 above)
```

---

**Your application is ready to deploy!** 🚀

