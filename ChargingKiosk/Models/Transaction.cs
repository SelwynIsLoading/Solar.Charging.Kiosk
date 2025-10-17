namespace ChargingKiosk.Models;

public class Transaction
{
    public int Id { get; set; }
    public int SlotNumber { get; set; }
    public DateTime StartTime { get; set; }
    public DateTime? EndTime { get; set; }
    public decimal TotalAmount { get; set; }
    public SlotType SlotType { get; set; }
    public int? FingerprintId { get; set; }
}

