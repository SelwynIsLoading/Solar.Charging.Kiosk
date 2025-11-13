namespace ChargingKiosk.Services;

public interface IAdminSettingsService
{
    bool AutoActivateCoinAcceptor { get; set; }
    bool AllowCoinBypass { get; set; }
    bool EnableCoinSimulationButton { get; set; }
}


