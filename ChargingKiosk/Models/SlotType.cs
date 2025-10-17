namespace ChargingKiosk.Models;

public enum SlotType
{
    Open,       // Slots 1-3: No security, just relay control
    Phone,      // Slots 4-9: Fingerprint, UV light, auto lock
    Laptop      // Slots 10-13: Fingerprint, auto lock (no UV)
}

