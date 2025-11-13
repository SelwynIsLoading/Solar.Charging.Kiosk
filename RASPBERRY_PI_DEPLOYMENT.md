## Raspberry Pi Deployment

- **Prerequisites**
  - Raspberry Pi OS (Bookworm or later) with SSH enabled.
  - Network connectivity and a user with `sudo` rights (examples assume `pi`).
  - Arduino wired to the Pi via USB (check `ls /dev/ttyACM*` after connecting).

- **System Update**
  - `uname -m` (confirm `aarch64`).
  - `sudo apt update && sudo apt upgrade -y`
  - `sudo reboot`

- **Packages**
  - `sudo apt install -y python3 python3-venv python3-pip git`
  - Remove any stale Microsoft feed: `sudo rm -f /etc/apt/sources.list.d/microsoft-prod.list`
  - Install the Microsoft package feed (arm64):
    - `wget https://packages.microsoft.com/config/debian/12/packages-microsoft-prod.deb -O packages-microsoft-prod.deb`
    - `sudo dpkg -i packages-microsoft-prod.deb`
    - `rm packages-microsoft-prod.deb`
    - `echo "deb [arch=arm64] https://packages.microsoft.com/debian/12/prod bookworm main" | sudo tee /etc/apt/sources.list.d/microsoft-prod.list`
    - `sudo apt update`
  - Check runtime availability: `apt-cache policy dotnet-runtime-9.0`
    - If listed, install with `sudo apt install -y dotnet-runtime-9.0`.
    - If “Unable to locate package”, the arm64 runtime hasn’t been published yet—switch to a self-contained publish (see “Self-Contained Deploy”) and skip the runtime install.
  - `sudo usermod -a -G dialout pi` (gives access to `/dev/ttyACM0`, log out/in or reboot).

- **Project Layout**
  - Place the repository under `/opt/chargingkiosk`.
  - Ownership: `sudo chown -R pi:pi /opt/chargingkiosk`.
  - Python bridge virtual environment:
    - `cd /opt/chargingkiosk/python-api-sample`
    - `python3 -m venv .venv`
    - `source .venv/bin/activate`
    - `pip install -r requirements.txt`
    - `deactivate`

- **Environment Variables**
  - Update `app.py` if the Arduino enumerates on a different port (`/dev/ttyACM0` vs `/dev/ttyUSB0`).
  - Optionally set `ARDUINO_PORT` via systemd `Environment=` entries.

- **Systemd Service: Python Bridge**
  - Create `/etc/systemd/system/chargingkiosk-python.service`:
```
[Unit]
Description=Charging Kiosk Python Bridge
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
# Replace with your deployment user (e.g., `solaradmin`)
User=pi
WorkingDirectory=/opt/chargingkiosk/python-api-sample
Environment="PATH=/opt/chargingkiosk/python-api-sample/.venv/bin"
ExecStart=/opt/chargingkiosk/python-api-sample/.venv/bin/python app.py
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

- **Systemd Service: ASP.NET/Blazor**
  - Publish on the dev machine with .NET 9 SDK: `dotnet publish ChargingKiosk/ChargingKiosk.csproj -c Release -r linux-arm64 --self-contained false -o publish`.
  - Copy the `publish` directory to `/opt/chargingkiosk/app`.
  - Create `/etc/systemd/system/chargingkiosk-dotnet.service`:
```
[Unit]
Description=Charging Kiosk Blazor Server
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
# Replace with your deployment user (e.g., `solaradmin`)
User=pi
WorkingDirectory=/opt/chargingkiosk/app
Environment=ASPNETCORE_ENVIRONMENT=Production
# Update path if dotnet is elsewhere (e.g., `/home/solar/.dotnet/dotnet`)
ExecStart=/usr/bin/dotnet ChargingKiosk.dll
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

- **Enable Services**
  - `sudo systemctl daemon-reload`
  - `sudo systemctl enable chargingkiosk-python.service`
  - `sudo systemctl enable chargingkiosk-dotnet.service`
  - Start immediately with `sudo systemctl start chargingkiosk-python.service` and `sudo systemctl start chargingkiosk-dotnet.service`.

- **Verification**
  - `systemctl status chargingkiosk-python.service`
  - `systemctl status chargingkiosk-dotnet.service`
  - `journalctl -u chargingkiosk-python.service -f` (hardware logs).
  - `journalctl -u chargingkiosk-dotnet.service -f` (web app logs).
  - Browse to the configured port (default `http://<pi-ip>:5000`) or behind Nginx.

- **Self-Contained Deploy (Fallback)**
  - When `dotnet-runtime-9.0` is unavailable on arm64:
    - `dotnet publish ChargingKiosk/ChargingKiosk.csproj -c Release -r linux-arm64 --self-contained true -o publish`
    - (Optional) Trim unused libs: add `-p:PublishTrimmed=true` after verifying at runtime.
  - Deploy the publish output to `/opt/chargingkiosk/app`.
  - Adjust the systemd service `ExecStart=/opt/chargingkiosk/app/ChargingKiosk` (no `dotnet` host needed).
  - Copy files from dev machine to Pi (replace `<version>` with the folder name under `ChargingKioskReleases`):
    - Git Bash / WSL: `scp -r "C:/Users/cauba/Documents/selwyn dev/2025/ChargingKioskReleases/<version>/*" pi@<pi-ip>:/opt/chargingkiosk/app/`
    - PowerShell: `scp -r "C:\Users\cauba\Documents\selwyn dev\2025\ChargingKioskReleases\<version>\*" pi@<pi-ip>:/opt/chargingkiosk/app/`

- **Manual Runtime Drop (Alternative)**
  - Use Microsoft’s installer script (avoids brittle direct URLs):
    - Install or refresh certificates: `sudo apt install -y --reinstall ca-certificates`
    - Regenerate bundle: `sudo update-ca-certificates`
    - Optional: sync date/time to avoid TLS failures: `sudo timedatectl set-ntp true`
    - (If the download still fails with “certificate not trusted”, import DigiCert’s root manually):
      - `wget https://cacerts.digicert.com/DigiCertGlobalRootG2.crt.pem`
      - `sudo install -m 644 DigiCertGlobalRootG2.crt.pem /usr/local/share/ca-certificates/DigiCertGlobalRootG2.crt`
      - `sudo update-ca-certificates`
      - `rm DigiCertGlobalRootG2.crt.pem`
    - `wget https://dot.net/v1/dotnet-install.sh -O dotnet-install.sh`
    - `chmod +x dotnet-install.sh`
    - Create install dir: `sudo mkdir -p /usr/share/dotnet`
    - Install runtime: `sudo ./dotnet-install.sh --channel 9.0 --runtime aspnetcore --runtime-version latest --install-dir /usr/share/dotnet`
    - Optional SDK (for builds on-device): `sudo ./dotnet-install.sh --channel 9.0 --install-dir /usr/share/dotnet`
    - Remove script: `rm dotnet-install.sh`
  - Export environment in `/etc/profile.d/dotnet.sh`:
    - `printf 'export DOTNET_ROOT=/usr/share/dotnet\nexport PATH=$PATH:/usr/share/dotnet\n' | sudo tee /etc/profile.d/dotnet.sh`
    - `sudo chmod 644 /etc/profile.d/dotnet.sh`
  - Reboot or `source /etc/profile.d/dotnet.sh` (for current shell: `. /etc/profile.d/dotnet.sh`), then confirm with `dotnet --info`.

- **Reboot Test**
  - `sudo reboot`
  - After reboot, confirm both services show `active (running)`.

- **Updates**
  - Stop services: `sudo systemctl stop chargingkiosk-python.service chargingkiosk-dotnet.service`.
  - Deploy new code, reinstall dependencies if needed.
  - Start services again; tail logs for the first minutes.


