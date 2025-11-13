namespace ChargingKiosk.Models;

public enum SlotType
{
    Open = 0,       // Slots 1-3: No security, just relay control
    Phone = 1,      // Slots 7-12: Fingerprint, UV light, auto lock
    Laptop = 2,     // Slots 13-16: Fingerprint, auto lock (no UV)
    Secure = 3      // Slots 4-6: Solenoid lock with fingerprint (no UV)
}

