# Service Quick Reference

## One-Time Setup

```bash
# Copy these files to your Raspberry Pi, then run:
chmod +x setup-services.sh
sudo ./setup-services.sh
```

## Common Commands

### Check if Services are Running
```bash
sudo systemctl status charging-kiosk-ui.service
sudo systemctl status charging-kiosk-api.service
```

### Start Services
```bash
sudo systemctl start charging-kiosk-ui.service
sudo systemctl start charging-kiosk-api.service
```

### Stop Services
```bash
sudo systemctl stop charging-kiosk-ui.service
sudo systemctl stop charging-kiosk-api.service
```

### Restart Services (after updates)
```bash
sudo systemctl restart charging-kiosk-ui.service
sudo systemctl restart charging-kiosk-api.service
```

### View Logs (Real-time)
```bash
# UI logs
sudo journalctl -u charging-kiosk-ui.service -f

# API logs
sudo journalctl -u charging-kiosk-api.service -f

# Both logs
sudo journalctl -u charging-kiosk-ui.service -u charging-kiosk-api.service -f
```

### View Recent Logs
```bash
# Last 50 lines
sudo journalctl -u charging-kiosk-ui.service -n 50
sudo journalctl -u charging-kiosk-api.service -n 50
```

## Deployment Workflow

```bash
# 1. Stop services
sudo systemctl stop charging-kiosk-ui.service charging-kiosk-api.service

# 2. Update files (copy new build)

# 3. Start services
sudo systemctl start charging-kiosk-ui.service charging-kiosk-api.service

# 4. Check logs
sudo journalctl -u charging-kiosk-ui.service -u charging-kiosk-api.service -f
```

## Troubleshooting

### Service won't start
```bash
# Check detailed status
sudo systemctl status charging-kiosk-ui.service -l
sudo systemctl status charging-kiosk-api.service -l

# View all logs
sudo journalctl -u charging-kiosk-ui.service --no-pager
sudo journalctl -u charging-kiosk-api.service --no-pager
```

### Check if ports are in use
```bash
sudo netstat -tulpn | grep :5000  # UI port
sudo netstat -tulpn | grep :5001  # API port (adjust if different)
```

### Verify after reboot
```bash
sudo systemctl is-active charging-kiosk-ui.service
sudo systemctl is-active charging-kiosk-api.service
sudo systemctl is-enabled charging-kiosk-ui.service
sudo systemctl is-enabled charging-kiosk-api.service
```

## File Locations

- **Blazor UI**: `/home/solaradmin/charging-kiosk/ui`
- **Python API**: `/home/solaradmin/charging-kiosk/app.py`
- **Service Files**: `/etc/systemd/system/charging-kiosk-*.service`
- **Logs**: `sudo journalctl -u <service-name>`

## URLs (Default)

- **Blazor UI**: http://raspberry-pi-ip:5000
- **Python API**: http://raspberry-pi-ip:5001 (or port configured in app.py)

