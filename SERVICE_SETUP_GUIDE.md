# Auto-Start Service Setup Guide

This guide will help you set up the Charging Kiosk services to run automatically when the Raspberry Pi boots.

## Prerequisites

- Blazor app built and deployed to `/home/solaradmin/charging-kiosk/ui`
- Python API located at `/home/solaradmin/charging-kiosk/app.py`
- Python dependencies installed (from requirements.txt)

## Installation Steps

### 1. Copy Service Files to Systemd Directory

```bash
# Copy the service files
sudo cp charging-kiosk-ui.service /etc/systemd/system/
sudo cp charging-kiosk-api.service /etc/systemd/system/

# Set proper permissions
sudo chmod 644 /etc/systemd/system/charging-kiosk-ui.service
sudo chmod 644 /etc/systemd/system/charging-kiosk-api.service
```

### 2. Reload Systemd

```bash
sudo systemctl daemon-reload
```

### 3. Enable Services (Auto-start on Boot)

```bash
# Enable both services to start on boot
sudo systemctl enable charging-kiosk-ui.service
sudo systemctl enable charging-kiosk-api.service
```

### 4. Start Services Immediately

```bash
# Start both services now
sudo systemctl start charging-kiosk-ui.service
sudo systemctl start charging-kiosk-api.service
```

## Service Management Commands

### Check Service Status

```bash
# Check UI service status
sudo systemctl status charging-kiosk-ui.service

# Check API service status
sudo systemctl status charging-kiosk-api.service
```

### View Service Logs

```bash
# View UI logs (real-time)
sudo journalctl -u charging-kiosk-ui.service -f

# View API logs (real-time)
sudo journalctl -u charging-kiosk-api.service -f

# View last 100 lines of logs
sudo journalctl -u charging-kiosk-ui.service -n 100
sudo journalctl -u charging-kiosk-api.service -n 100
```

### Stop Services

```bash
sudo systemctl stop charging-kiosk-ui.service
sudo systemctl stop charging-kiosk-api.service
```

### Restart Services

```bash
sudo systemctl restart charging-kiosk-ui.service
sudo systemctl restart charging-kiosk-api.service
```

### Disable Auto-start

```bash
# Disable auto-start on boot
sudo systemctl disable charging-kiosk-ui.service
sudo systemctl disable charging-kiosk-api.service
```

## Troubleshooting

### Service Won't Start

1. **Check service status for errors:**
   ```bash
   sudo systemctl status charging-kiosk-ui.service
   sudo systemctl status charging-kiosk-api.service
   ```

2. **Check logs:**
   ```bash
   sudo journalctl -u charging-kiosk-ui.service -n 50
   sudo journalctl -u charging-kiosk-api.service -n 50
   ```

3. **Verify file paths:**
   ```bash
   # Check if Blazor executable exists
   ls -l /home/solaradmin/charging-kiosk/ui/ChargingKiosk
   
   # Check if Python API exists
   ls -l /home/solaradmin/charging-kiosk/app.py
   ```

4. **Test manually:**
   ```bash
   # Test Blazor app manually
   cd /home/solaradmin/charging-kiosk/ui
   ./ChargingKiosk
   
   # Test Python API manually
   cd /home/solaradmin/charging-kiosk
   python3 app.py
   ```

### Permissions Issues

If you get permission errors:

```bash
# Ensure solaradmin owns the files
sudo chown -R solaradmin:solaradmin /home/solaradmin/charging-kiosk

# Make Blazor executable
chmod +x /home/solaradmin/charging-kiosk/ui/ChargingKiosk
```

### Python Dependencies Not Found

If the Python service fails due to missing packages:

```bash
# Install dependencies
cd /home/solaradmin/charging-kiosk
pip3 install -r requirements.txt

# Or install system-wide
sudo pip3 install -r requirements.txt
```

### Port Already in Use

If you get port binding errors:

```bash
# Find what's using port 5000 (UI)
sudo lsof -i :5000

# Find what's using port 5001 (API)
sudo lsof -i :5001

# Kill the process if needed
sudo kill -9 <PID>
```

## Configuration Changes

### Change Blazor Port

Edit `/etc/systemd/system/charging-kiosk-ui.service`:

```ini
Environment=ASPNETCORE_URLS=http://0.0.0.0:8080
```

Then reload and restart:

```bash
sudo systemctl daemon-reload
sudo systemctl restart charging-kiosk-ui.service
```

### Change Python Port

Edit the `app.py` file to change the Flask port, then restart:

```bash
sudo systemctl restart charging-kiosk-api.service
```

## Deployment Workflow

When deploying updates:

```bash
# 1. Stop services
sudo systemctl stop charging-kiosk-ui.service
sudo systemctl stop charging-kiosk-api.service

# 2. Deploy new files (copy new build, etc.)

# 3. Restart services
sudo systemctl start charging-kiosk-ui.service
sudo systemctl start charging-kiosk-api.service

# 4. Check status
sudo systemctl status charging-kiosk-ui.service
sudo systemctl status charging-kiosk-api.service
```

## Automatic Restart

Both services are configured to automatically restart if they crash:
- `Restart=always`: Service will restart on any exit
- `RestartSec=10`: Wait 10 seconds before restarting

## Verification After Boot

After rebooting the Raspberry Pi:

```bash
# Reboot
sudo reboot

# After reboot, verify services are running
sudo systemctl status charging-kiosk-ui.service
sudo systemctl status charging-kiosk-api.service

# Check if ports are listening
sudo netstat -tulpn | grep :5000
sudo netstat -tulpn | grep :5001
```

## Service Dependencies

The services are configured to:
- Start after network is available (`After=network.target`)
- Start in multi-user mode (`WantedBy=multi-user.target`)
- Run as the `solaradmin` user
- Automatically restart on failure

## Notes

- Services will start automatically on every boot
- Logs are managed by systemd journald
- Services run with minimal privileges (as solaradmin user)
- Both services restart automatically if they crash

