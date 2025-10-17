namespace ChargingKiosk.Services;

public interface IArduinoApiService
{
    Task<bool> ControlRelayAsync(int slotNumber, bool turnOn);
    Task<bool> ControlSolenoidAsync(int slotNumber, bool locked);
    Task<bool> ControlUVLightAsync(int slotNumber, bool turnOn);
    Task<bool> VerifyFingerprintAsync(int fingerprintId);
    Task<bool> EnrollFingerprintAsync(int fingerprintId);
    Task<bool> DeleteFingerprintAsync(int fingerprintId);
    Task<decimal> GetCoinSlotValueAsync();
    Task<bool> UnlockTemporaryAsync(int slotNumber);
}
