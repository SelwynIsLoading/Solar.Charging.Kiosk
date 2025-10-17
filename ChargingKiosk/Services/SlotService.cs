using ChargingKiosk.Models;

namespace ChargingKiosk.Services;

public class SlotService : ISlotService
{
    private readonly List<ChargingSlot> _slots;
    private readonly IArduinoApiService _arduinoService;
    private readonly ILogger<SlotService> _logger;

    public SlotService(IArduinoApiService arduinoService, ILogger<SlotService> logger)
    {
        _arduinoService = arduinoService;
        _logger = logger;
        _slots = InitializeSlots();
    }

    private List<ChargingSlot> InitializeSlots()
    {
        var slots = new List<ChargingSlot>();
        
        // Slots 1-3: Open (no security)
        for (int i = 1; i <= 3; i++)
        {
            slots.Add(new ChargingSlot
            {
                SlotNumber = i,
                Type = SlotType.Open,
                Status = SlotStatus.Available
            });
        }
        
        // Slots 4-9: Phone (with fingerprint and UV)
        for (int i = 4; i <= 9; i++)
        {
            slots.Add(new ChargingSlot
            {
                SlotNumber = i,
                Type = SlotType.Phone,
                Status = SlotStatus.Available
            });
        }
        
        // Slots 10-13: Laptop (with fingerprint, no UV)
        for (int i = 10; i <= 13; i++)
        {
            slots.Add(new ChargingSlot
            {
                SlotNumber = i,
                Type = SlotType.Laptop,
                Status = SlotStatus.Available
            });
        }
        
        return slots;
    }

    public List<ChargingSlot> GetAllSlots()
    {
        return _slots;
    }

    public ChargingSlot GetSlot(int slotNumber)
    {
        return _slots.FirstOrDefault(s => s.SlotNumber == slotNumber) 
               ?? throw new ArgumentException($"Slot {slotNumber} not found");
    }

    public async Task<bool> StartChargingAsync(int slotNumber, decimal coinsInserted, int? fingerprintId = null)
    {
        var slot = GetSlot(slotNumber);
        
        if (slot.Status != SlotStatus.Available)
        {
            _logger.LogWarning($"Slot {slotNumber} is not available");
            return false;
        }

        // For Phone and Laptop slots, store the enrolled fingerprint ID
        // No verification needed here - fingerprint was just enrolled in the UI
        if ((slot.Type == SlotType.Phone || slot.Type == SlotType.Laptop) && fingerprintId.HasValue)
        {
            slot.FingerprintId = fingerprintId;
            _logger.LogInformation($"Slot {slotNumber} secured with fingerprint ID: {fingerprintId}");
        }

        slot.CoinsInserted = coinsInserted;
        slot.StartTime = DateTime.Now;
        slot.Status = SlotStatus.InUse;
        
        // Turn on relay
        await ControlRelayAsync(slotNumber, true);
        
        // For Phone slots, start UV sanitization
        if (slot.Type == SlotType.Phone)
        {
            await StartUVSanitizationAsync(slotNumber);
        }
        
        // Lock the slot if it's Phone or Laptop type
        if (slot.Type == SlotType.Phone || slot.Type == SlotType.Laptop)
        {
            await LockSlotAsync(slotNumber, true);
        }
        
        return true;
    }
    
    public void SetChargingMinutes(int slotNumber, int minutes)
    {
        var slot = GetSlot(slotNumber);
        slot.ChargingMinutesAllocated = minutes;
    }

    public async Task<bool> StopChargingAsync(int slotNumber)
    {
        var slot = GetSlot(slotNumber);
        
        if (slot.Status != SlotStatus.InUse && slot.Status != SlotStatus.Locked)
        {
            _logger.LogWarning($"Slot {slotNumber} is not in use");
            return false;
        }

        slot.EndTime = DateTime.Now;
        slot.Status = SlotStatus.Available;
        slot.FingerprintId = null;
        
        // Turn off relay
        await ControlRelayAsync(slotNumber, false);
        
        // Unlock the slot
        if (slot.Type == SlotType.Phone || slot.Type == SlotType.Laptop)
        {
            await LockSlotAsync(slotNumber, false);
        }
        
        return true;
    }

    public async Task<bool> ControlRelayAsync(int slotNumber, bool turnOn)
    {
        var slot = GetSlot(slotNumber);
        slot.IsRelayOn = turnOn;
        
        try
        {
            await _arduinoService.ControlRelayAsync(slotNumber, turnOn);
            _logger.LogInformation($"Relay for slot {slotNumber} turned {(turnOn ? "ON" : "OFF")}");
            return true;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, $"Failed to control relay for slot {slotNumber}");
            return false;
        }
    }

    public async Task<bool> LockSlotAsync(int slotNumber, bool lockState)
    {
        var slot = GetSlot(slotNumber);
        slot.IsLocked = lockState;
        
        try
        {
            await _arduinoService.ControlSolenoidAsync(slotNumber, lockState);
            _logger.LogInformation($"Slot {slotNumber} {(lockState ? "locked" : "unlocked")}");
            return true;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, $"Failed to lock/unlock slot {slotNumber}");
            return false;
        }
    }

    public async Task<bool> StartUVSanitizationAsync(int slotNumber)
    {
        var slot = GetSlot(slotNumber);
        
        if (slot.Type != SlotType.Phone)
        {
            return false;
        }
        
        slot.Status = SlotStatus.Sanitizing;
        
        try
        {
            await _arduinoService.ControlUVLightAsync(slotNumber, true);
            _logger.LogInformation($"UV sanitization started for slot {slotNumber}");
            
            // Wait for 15 seconds
            await Task.Delay(15000);
            
            await _arduinoService.ControlUVLightAsync(slotNumber, false);
            slot.Status = SlotStatus.Locked;
            _logger.LogInformation($"UV sanitization completed for slot {slotNumber}");
            
            return true;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, $"Failed UV sanitization for slot {slotNumber}");
            slot.Status = SlotStatus.InUse;
            return false;
        }
    }

    public async Task<bool> VerifyFingerprintAsync(int fingerprintId)
    {
        try
        {
            return await _arduinoService.VerifyFingerprintAsync(fingerprintId);
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Fingerprint verification failed");
            return false;
        }
    }

    public async Task<bool> UnlockTemporaryAsync(int slotNumber, int fingerprintId)
    {
        var slot = GetSlot(slotNumber);
        
        // Only phone and laptop slots can be unlocked
        if (slot.Type != SlotType.Phone && slot.Type != SlotType.Laptop)
        {
            _logger.LogWarning($"Slot {slotNumber} is not a secured slot type");
            return false;
        }
        
        // Verify the slot is in use and locked
        if (slot.Status != SlotStatus.InUse && slot.Status != SlotStatus.Locked)
        {
            _logger.LogWarning($"Slot {slotNumber} is not in use");
            return false;
        }
        
        // Verify fingerprint matches
        if (!slot.FingerprintId.HasValue || slot.FingerprintId.Value != fingerprintId)
        {
            _logger.LogWarning($"Fingerprint ID mismatch for slot {slotNumber}");
            
            // Still attempt verification with Arduino
            var verified = await VerifyFingerprintAsync(fingerprintId);
            if (!verified)
            {
                return false;
            }
        }
        
        try
        {
            // Send temporary unlock command (2-second pulse)
            await _arduinoService.UnlockTemporaryAsync(slotNumber);
            _logger.LogInformation($"Slot {slotNumber} temporarily unlocked for device access");
            return true;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, $"Failed to temporarily unlock slot {slotNumber}");
            return false;
        }
    }
}

