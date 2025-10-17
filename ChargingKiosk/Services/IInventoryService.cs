using ChargingKiosk.Models;

namespace ChargingKiosk.Services;

public interface IInventoryService
{
    Task<List<Transaction>> GetTransactionsAsync(DateTime? startDate = null, DateTime? endDate = null);
    Task<decimal> GetTotalRevenueAsync(DateTime? startDate = null, DateTime? endDate = null);
    Task AddTransactionAsync(Transaction transaction);
    Task<List<CoinDenomination>> GetCoinDenominationsAsync();
    Task UpdateCoinDenominationAsync(CoinDenomination denomination);
    Task IncrementCoinCountAsync(int denominationId);
    Task<Dictionary<string, decimal>> GetDailyRevenueAsync();
    Task<Dictionary<string, decimal>> GetMonthlyRevenueAsync();
    Task<Dictionary<string, decimal>> GetYearlyRevenueAsync();
}

