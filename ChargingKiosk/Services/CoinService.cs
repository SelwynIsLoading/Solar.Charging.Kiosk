using ChargingKiosk.Data;
using ChargingKiosk.Models;
using Microsoft.EntityFrameworkCore;

namespace ChargingKiosk.Services;

public class CoinService : ICoinService
{
    private readonly ApplicationDbContext _context;

    public CoinService(ApplicationDbContext context)
    {
        _context = context;
    }

    public async Task<List<CoinDenomination>> GetAllDenominationsAsync()
    {
        // Load data first, then order in memory to avoid SQLite decimal ordering issues
        var denominations = await _context.CoinDenominations
            .Where(d => d.IsActive)
            .ToListAsync();
            
        return denominations.OrderBy(d => d.Value).ToList();
    }

    public async Task<int> GetChargingMinutesForAmountAsync(decimal amount)
    {
        var denominations = await GetAllDenominationsAsync();
        
        // Calculate total charging minutes based on coin values
        // This is a simple implementation - can be enhanced for exact change
        int totalMinutes = 0;
        decimal remainingAmount = amount;

        // Start from highest denomination to lowest
        foreach (var denom in denominations.OrderByDescending(d => d.Value))
        {
            while (remainingAmount >= denom.Value)
            {
                totalMinutes += denom.ChargingMinutes;
                remainingAmount -= denom.Value;
            }
        }

        return totalMinutes;
    }
}

