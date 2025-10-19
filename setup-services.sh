#!/bin/bash

# Charging Kiosk Service Setup Script
# This script sets up systemd services for auto-start on boot

set -e

echo "======================================"
echo "Charging Kiosk Service Setup"
echo "======================================"
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

# Verify files exist
echo "Checking if required files exist..."

if [ ! -f "charging-kiosk-ui.service" ]; then
    echo "Error: charging-kiosk-ui.service not found"
    exit 1
fi

if [ ! -f "charging-kiosk-api.service" ]; then
    echo "Error: charging-kiosk-api.service not found"
    exit 1
fi

if [ ! -d "/home/solaradmin/charging-kiosk/ui" ]; then
    echo "Warning: /home/solaradmin/charging-kiosk/ui directory not found"
    read -p "Continue anyway? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

if [ ! -f "/home/solaradmin/charging-kiosk/app.py" ]; then
    echo "Warning: /home/solaradmin/charging-kiosk/app.py not found"
    read -p "Continue anyway? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

echo "✓ All checks passed"
echo ""

# Copy service files
echo "Installing service files..."
cp charging-kiosk-ui.service /etc/systemd/system/
cp charging-kiosk-api.service /etc/systemd/system/
chmod 644 /etc/systemd/system/charging-kiosk-ui.service
chmod 644 /etc/systemd/system/charging-kiosk-api.service
echo "✓ Service files installed"
echo ""

# Reload systemd
echo "Reloading systemd daemon..."
systemctl daemon-reload
echo "✓ Systemd reloaded"
echo ""

# Enable services
echo "Enabling services to start on boot..."
systemctl enable charging-kiosk-ui.service
systemctl enable charging-kiosk-api.service
echo "✓ Services enabled"
echo ""

# Ask if user wants to start services now
read -p "Do you want to start the services now? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "Starting services..."
    systemctl start charging-kiosk-ui.service
    systemctl start charging-kiosk-api.service
    echo "✓ Services started"
    echo ""
    
    # Show status
    echo "======================================"
    echo "Service Status:"
    echo "======================================"
    echo ""
    echo "UI Service:"
    systemctl status charging-kiosk-ui.service --no-pager -l
    echo ""
    echo "API Service:"
    systemctl status charging-kiosk-api.service --no-pager -l
fi

echo ""
echo "======================================"
echo "Setup Complete!"
echo "======================================"
echo ""
echo "Services will now start automatically on boot."
echo ""
echo "Useful commands:"
echo "  sudo systemctl status charging-kiosk-ui.service"
echo "  sudo systemctl status charging-kiosk-api.service"
echo "  sudo journalctl -u charging-kiosk-ui.service -f"
echo "  sudo journalctl -u charging-kiosk-api.service -f"
echo ""
echo "See SERVICE_SETUP_GUIDE.md for more information."

