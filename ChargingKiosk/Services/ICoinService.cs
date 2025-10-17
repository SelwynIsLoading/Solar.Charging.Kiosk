using ChargingKiosk.Models;

namespace ChargingKiosk.Services;

public interface ICoinService
{
    Task<List<CoinDenomination>> GetAllDenominationsAsync();
    Task<int> GetChargingMinutesForAmountAsync(decimal amount);
}

