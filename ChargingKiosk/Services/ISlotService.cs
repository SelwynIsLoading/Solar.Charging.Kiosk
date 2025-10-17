using ChargingKiosk.Models;

namespace ChargingKiosk.Services;

public interface ISlotService
{
    List<ChargingSlot> GetAllSlots();
    ChargingSlot GetSlot(int slotNumber);
    Task<bool> StartChargingAsync(int slotNumber, decimal coinsInserted, int? fingerprintId = null);
    Task<bool> StopChargingAsync(int slotNumber);
    Task<bool> ControlRelayAsync(int slotNumber, bool turnOn);
    Task<bool> LockSlotAsync(int slotNumber, bool lockState);
    Task<bool> StartUVSanitizationAsync(int slotNumber);
    Task<bool> VerifyFingerprintAsync(int fingerprintId);
    Task<bool> UnlockTemporaryAsync(int slotNumber, int fingerprintId);
    void SetChargingMinutes(int slotNumber, int minutes);
}

