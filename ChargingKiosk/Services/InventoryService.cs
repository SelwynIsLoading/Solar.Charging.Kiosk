using ChargingKiosk.Data;
using ChargingKiosk.Models;
using Microsoft.EntityFrameworkCore;

namespace ChargingKiosk.Services;

public class InventoryService : IInventoryService
{
    private readonly ApplicationDbContext _context;
    private readonly ILogger<InventoryService> _logger;

    public InventoryService(ApplicationDbContext context, ILogger<InventoryService> logger)
    {
        _context = context;
        _logger = logger;
    }

    public async Task<List<Transaction>> GetTransactionsAsync(DateTime? startDate = null, DateTime? endDate = null)
    {
        var query = _context.Transactions.AsQueryable();

        if (startDate.HasValue)
        {
            query = query.Where(t => t.StartTime >= startDate.Value);
        }

        if (endDate.HasValue)
        {
            query = query.Where(t => t.StartTime <= endDate.Value);
        }

        return await query.OrderByDescending(t => t.StartTime).ToListAsync();
    }

    public async Task<decimal> GetTotalRevenueAsync(DateTime? startDate = null, DateTime? endDate = null)
    {
        var transactions = await GetTransactionsAsync(startDate, endDate);
        return transactions.Sum(t => t.TotalAmount);
    }

    public async Task AddTransactionAsync(Transaction transaction)
    {
        _context.Transactions.Add(transaction);
        await _context.SaveChangesAsync();
        _logger.LogInformation($"Transaction added for slot {transaction.SlotNumber}");
    }

    public async Task<List<CoinDenomination>> GetCoinDenominationsAsync()
    {
        return await _context.CoinDenominations.Where(d => d.IsActive).ToListAsync();
    }

    public async Task UpdateCoinDenominationAsync(CoinDenomination denomination)
    {
        _context.CoinDenominations.Update(denomination);
        await _context.SaveChangesAsync();
        _logger.LogInformation($"Coin denomination {denomination.Name} updated");
    }

    public async Task IncrementCoinCountAsync(int denominationId)
    {
        var denomination = await _context.CoinDenominations.FindAsync(denominationId);
        if (denomination != null)
        {
            denomination.DailyCount++;
            denomination.MonthlyCount++;
            denomination.YearlyCount++;
            await _context.SaveChangesAsync();
        }
    }

    public async Task<Dictionary<string, decimal>> GetDailyRevenueAsync()
    {
        var today = DateTime.Today;
        var transactions = await GetTransactionsAsync(today, today.AddDays(1));
        
        return new Dictionary<string, decimal>
        {
            { "Total", transactions.Sum(t => t.TotalAmount) },
            { "Open Slots", transactions.Where(t => t.SlotType == SlotType.Open).Sum(t => t.TotalAmount) },
            { "Phone Slots", transactions.Where(t => t.SlotType == SlotType.Phone).Sum(t => t.TotalAmount) },
            { "Laptop Slots", transactions.Where(t => t.SlotType == SlotType.Laptop).Sum(t => t.TotalAmount) }
        };
    }

    public async Task<Dictionary<string, decimal>> GetMonthlyRevenueAsync()
    {
        var startOfMonth = new DateTime(DateTime.Now.Year, DateTime.Now.Month, 1);
        var transactions = await GetTransactionsAsync(startOfMonth, DateTime.Now);
        
        return new Dictionary<string, decimal>
        {
            { "Total", transactions.Sum(t => t.TotalAmount) },
            { "Open Slots", transactions.Where(t => t.SlotType == SlotType.Open).Sum(t => t.TotalAmount) },
            { "Phone Slots", transactions.Where(t => t.SlotType == SlotType.Phone).Sum(t => t.TotalAmount) },
            { "Laptop Slots", transactions.Where(t => t.SlotType == SlotType.Laptop).Sum(t => t.TotalAmount) }
        };
    }

    public async Task<Dictionary<string, decimal>> GetYearlyRevenueAsync()
    {
        var startOfYear = new DateTime(DateTime.Now.Year, 1, 1);
        var transactions = await GetTransactionsAsync(startOfYear, DateTime.Now);
        
        return new Dictionary<string, decimal>
        {
            { "Total", transactions.Sum(t => t.TotalAmount) },
            { "Open Slots", transactions.Where(t => t.SlotType == SlotType.Open).Sum(t => t.TotalAmount) },
            { "Phone Slots", transactions.Where(t => t.SlotType == SlotType.Phone).Sum(t => t.TotalAmount) },
            { "Laptop Slots", transactions.Where(t => t.SlotType == SlotType.Laptop).Sum(t => t.TotalAmount) }
        };
    }
}

