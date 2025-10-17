namespace ChargingKiosk.Models;

public class ChargingSlot
{
    public int SlotNumber { get; set; }
    public SlotType Type { get; set; }
    public SlotStatus Status { get; set; }
    public DateTime? StartTime { get; set; }
    public DateTime? EndTime { get; set; }
    public decimal CoinsInserted { get; set; }
    public int ChargingMinutesAllocated { get; set; } // Total minutes allocated from coins
    public bool IsRelayOn { get; set; }
    public bool IsLocked { get; set; }
    public int? FingerprintId { get; set; }
    
    public string DisplayName => Type switch
    {
        SlotType.Open => $"Open Slot {SlotNumber}",
        SlotType.Phone => $"Phone Slot {SlotNumber}",
        SlotType.Laptop => $"Laptop Slot {SlotNumber}",
        _ => $"Slot {SlotNumber}"
    };
    
    public string IconClass => Type switch
    {
        SlotType.Open => "bi bi-plug",
        SlotType.Phone => "bi bi-phone",
        SlotType.Laptop => "bi bi-laptop",
        _ => "bi bi-outlet"
    };
    
    public string StatusColor => Status switch
    {
        SlotStatus.Available => "success",
        SlotStatus.InUse => "warning",
        SlotStatus.Sanitizing => "info",
        SlotStatus.Locked => "primary",
        SlotStatus.OutOfService => "error",
        _ => "default"
    };
}

