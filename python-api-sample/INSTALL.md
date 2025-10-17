# Python API Installation Guide

Quick guide to set up the Python API bridge for the Solar Charging Station.

## Prerequisites

1. **Python 3.8 or higher** must be installed
2. **pip** (Python package installer) should be available

### Check Python Installation

Open Command Prompt (Windows) or Terminal (Mac/Linux) and run:

```bash
python --version
# or
python3 --version
```

You should see something like: `Python 3.11.0` or higher

### Check pip Installation

```bash
pip --version
# or
pip3 --version
```

## Installation Methods

### Method 1: Install Globally (Without Virtual Environment)

This installs packages system-wide, accessible from anywhere.

#### Windows (Command Prompt or PowerShell)

```cmd
cd python-api-sample
pip install -r requirements.txt
```

If you get permission errors, run as Administrator:

```cmd
pip install --user -r requirements.txt
```

Or explicitly use `python -m pip`:

```cmd
python -m pip install -r requirements.txt
```

#### Windows (Alternative)

Install each package individually:

```cmd
pip install Flask==3.0.0
pip install flask-cors==4.0.0
pip install pyserial==3.5
```

#### Mac/Linux

```bash
cd python-api-sample
pip3 install -r requirements.txt
```

If permission denied:

```bash
pip3 install --user -r requirements.txt
# or with sudo (not recommended)
sudo pip3 install -r requirements.txt
```

#### Raspberry Pi OS (Bookworm) - "Externally Managed Environment" Error

If you see: `error: externally-managed-environment`

**Solution 1: Use apt (Recommended for Raspberry Pi)**

```bash
sudo apt update
sudo apt install -y python3-flask python3-flask-cors python3-serial
```

**Solution 2: Break System Packages (Quick Fix)**

```bash
pip3 install -r requirements.txt --break-system-packages
```

**Solution 3: Use Virtual Environment (Best Practice)**

```bash
cd python-api-sample
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

See [Raspberry Pi Specific Setup](#raspberry-pi-specific-setup) section below for details.

### Method 2: Using Virtual Environment (Recommended for Development)

If you change your mind and want to use a virtual environment:

#### Windows

```cmd
cd python-api-sample
python -m venv venv
venv\Scripts\activate
pip install -r requirements.txt
```

#### Mac/Linux

```bash
cd python-api-sample
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

## Verify Installation

After installation, verify packages are installed:

```bash
pip list | findstr Flask
pip list | findstr flask-cors
pip list | findstr pyserial
```

You should see:

```
Flask                3.0.0
flask-cors           4.0.0
pyserial             3.5
```

## Required Packages

The `requirements.txt` contains:

| Package | Version | Purpose |
|---------|---------|---------|
| Flask | 3.0.0 | Web framework for API endpoints |
| flask-cors | 4.0.0 | Enable CORS for Blazor app communication |
| pyserial | 3.5 | Serial communication with Arduino |

## Running the API

Once installed, run the API:

### Windows

```cmd
cd python-api-sample
python app.py
```

### Mac/Linux

```bash
cd python-api-sample
python3 app.py
```

You should see:

```
Starting Solar Charging Station Python API...
Arduino connection: Simulated mode
 * Serving Flask app 'app'
 * Debug mode: on
 * Running on http://0.0.0.0:5000
```

## Testing the API

Open a browser and navigate to:

```
http://localhost:5000/health
```

You should see:

```json
{
  "status": "healthy",
  "arduino_connected": false
}
```

## Troubleshooting

### Problem: "pip is not recognized"

**Solution:**

1. Add Python to PATH:
   - Windows: System Properties → Environment Variables → Path
   - Add: `C:\Python311\` and `C:\Python311\Scripts\`

2. Or use full path:
   ```cmd
   python -m pip install -r requirements.txt
   ```

### Problem: "Permission denied"

**Solution:**

```bash
# Windows - Run as Administrator
# Or install for current user only:
pip install --user -r requirements.txt
```

### Problem: "No module named Flask"

**Solution:**

Check Python version used to install vs run:

```bash
# Install with python3
pip3 install Flask

# But running with python
python app.py  # Wrong!

# Fix: Use same Python version
python3 app.py  # Correct!
```

### Problem: Multiple Python Versions

**Solution:**

Specify Python version explicitly:

```cmd
py -3.11 -m pip install -r requirements.txt
py -3.11 app.py
```

### Problem: "externally-managed-environment" on Raspberry Pi

**Solution:**

See [Raspberry Pi Specific Setup](#raspberry-pi-specific-setup) section above.

Quick fix:

```bash
# Option 1: Use system packages (recommended)
sudo apt install python3-flask python3-flask-cors python3-serial

# Option 2: Override protection
pip3 install -r requirements.txt --break-system-packages

# Option 3: Use virtual environment
python3 -m venv venv && source venv/bin/activate && pip install -r requirements.txt
```

### Problem: "ImportError: No module named serial"

**Solution:**

The package is `pyserial` but imports as `serial`:

```bash
pip install pyserial  # Correct!
# NOT: pip install serial
```

### Problem: Port Already in Use

**Solution:**

```bash
# Windows - Find process on port 5000
netstat -ano | findstr :5000
taskkill /PID <process_id> /F

# Linux/Mac
lsof -i :5000
kill <process_id>
```

Or change port in `app.py`:

```python
app.run(host='0.0.0.0', port=5001, debug=True)
```

## Upgrading Packages

To upgrade to latest versions:

```bash
pip install --upgrade Flask flask-cors pyserial
```

## Uninstalling

To remove packages:

```bash
pip uninstall Flask flask-cors pyserial
```

## Security Notes

⚠️ **Installing globally (without virtual environment):**

**Pros:**
- ✅ Simpler setup
- ✅ Works from any directory
- ✅ Good for production deployment

**Cons:**
- ❌ May conflict with other Python projects
- ❌ Affects system-wide Python packages
- ❌ Harder to manage different versions

**Recommendation for Raspberry Pi:**
- Install globally is fine for dedicated kiosk system
- The Pi will only run this application

**Recommendation for Development PC:**
- Use virtual environment to avoid conflicts
- Keeps your development environment clean

## Raspberry Pi Specific Setup

### "Externally Managed Environment" Error

Starting with Raspberry Pi OS Bookworm (Debian 12), Python prevents direct pip installations to protect system packages.

#### Why This Happens

Raspberry Pi OS uses PEP 668 to prevent breaking system Python packages. You'll see:

```
error: externally-managed-environment

× This environment is externally managed
╰─> To install Python packages system-wide, try apt install
    python3-xyz, where xyz is the package you are trying to
    install.
```

### Solution 1: Install with apt (RECOMMENDED)

Use system package manager - this is the safest method:

```bash
# Update package list
sudo apt update

# Install required packages
sudo apt install -y python3-flask python3-flask-cors python3-serial

# Verify installation
python3 -c "import flask; import flask_cors; import serial; print('All packages installed!')"
```

**Pros:**
- ✅ Official Raspberry Pi way
- ✅ Integrated with system updates
- ✅ Won't break on OS upgrades
- ✅ No conflicts with system packages

**Cons:**
- ❌ May have slightly older versions
- ❌ Limited to available Debian packages

### Solution 2: Use --break-system-packages (QUICK FIX)

Override the protection (use with caution):

```bash
pip3 install -r requirements.txt --break-system-packages
```

Or install individually:

```bash
pip3 install Flask==3.0.0 --break-system-packages
pip3 install flask-cors==4.0.0 --break-system-packages
pip3 install pyserial==3.5 --break-system-packages
```

**Pros:**
- ✅ Quick and simple
- ✅ Gets exact versions you want
- ✅ No virtual environment needed

**Cons:**
- ⚠️ May conflict with system packages
- ⚠️ Could break on system updates
- ⚠️ Not recommended by Raspberry Pi Foundation

### Solution 3: Virtual Environment (BEST PRACTICE)

Create isolated environment:

```bash
cd /home/pi/python-api-sample
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

**To run with venv:**

```bash
cd /home/pi/python-api-sample
source venv/bin/activate
python app.py
```

**Pros:**
- ✅ Isolated from system Python
- ✅ Can't break system packages
- ✅ Multiple Python environments possible
- ✅ Industry best practice

**Cons:**
- ❌ Must activate before running
- ❌ Slightly more complex
- ❌ Need to update systemd service (see below)

### Recommended for Production Kiosk

For a dedicated Raspberry Pi kiosk, use **Solution 1 (apt)** because:
- ✅ Most stable and reliable
- ✅ Survives OS updates
- ✅ Officially supported method
- ✅ Perfect for production deployment

### Package Name Mapping

| pip package | apt package |
|-------------|-------------|
| Flask | python3-flask |
| flask-cors | python3-flask-cors |
| pyserial | python3-serial |

### Verify Installation

```bash
python3 << EOF
import flask
import flask_cors
import serial
print("Flask version:", flask.__version__)
print("All packages working!")
EOF
```

## Auto-start on Raspberry Pi

Once installed, set up as a system service:

### Create Service File

Choose the configuration based on your installation method:

#### If Using apt or --break-system-packages

```bash
sudo nano /etc/systemd/system/charging-api.service
```

```ini
[Unit]
Description=Solar Charging Station Python API
After=network.target

[Service]
WorkingDirectory=/home/pi/python-api-sample
ExecStart=/usr/bin/python3 /home/pi/python-api-sample/app.py
Restart=always
RestartSec=10
User=pi
Environment=PYTHONUNBUFFERED=1

[Install]
WantedBy=multi-user.target
```

#### If Using Virtual Environment

```bash
sudo nano /etc/systemd/system/charging-api.service
```

```ini
[Unit]
Description=Solar Charging Station Python API
After=network.target

[Service]
WorkingDirectory=/home/pi/python-api-sample
ExecStart=/home/pi/python-api-sample/venv/bin/python /home/pi/python-api-sample/app.py
Restart=always
RestartSec=10
User=pi
Environment=PYTHONUNBUFFERED=1
Environment=VIRTUAL_ENV=/home/pi/python-api-sample/venv
Environment=PATH=/home/pi/python-api-sample/venv/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

[Install]
WantedBy=multi-user.target
```

### Enable and Start

```bash
sudo systemctl daemon-reload
sudo systemctl enable charging-api
sudo systemctl start charging-api
sudo systemctl status charging-api
```

### View Logs

```bash
journalctl -u charging-api -f
```

## Quick Commands Reference

```bash
# Install globally
pip install -r requirements.txt

# Run API
python app.py

# Check if running
curl http://localhost:5000/health

# View installed packages
pip list

# Check package info
pip show Flask

# Upgrade package
pip install --upgrade Flask

# Uninstall package
pip uninstall Flask
```

## Next Steps

1. ✅ Install Python packages (this guide)
2. 📡 Connect Arduino to computer/Raspberry Pi
3. 🔧 Update COM port in `app.py` (Line 13)
4. ▶️ Run `python app.py`
5. 🌐 Run Blazor app: `dotnet run`
6. 🎉 Test the complete system

## Support

For issues:
- Check Python version: `python --version`
- Check pip version: `pip --version`
- Verify installation: `pip list`
- Read error messages carefully
- Check firewall settings (port 5000)

---

**Quick Start:** `pip install -r requirements.txt && python app.py`

