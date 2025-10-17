namespace ChargingKiosk.Models;

public class CoinDenomination
{
    public int Id { get; set; }
    public string Name { get; set; } = string.Empty;
    public decimal Value { get; set; }
    public int ChargingMinutes { get; set; } // Minutes of charging time this coin provides
    public int DailyCount { get; set; }
    public int MonthlyCount { get; set; }
    public int YearlyCount { get; set; }
    public bool IsActive { get; set; } = true;
}

