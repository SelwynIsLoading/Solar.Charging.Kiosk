namespace ChargingKiosk.Services;

public class AdminSettingsService : IAdminSettingsService
{
    private readonly object _lock = new();
    private bool _autoActivateCoinAcceptor = true;
    private bool _allowCoinBypass;
    private bool _enableCoinSimulationButton;

    public bool AutoActivateCoinAcceptor
    {
        get
        {
            lock (_lock)
            {
                return _autoActivateCoinAcceptor;
            }
        }
        set
        {
            lock (_lock)
            {
                _autoActivateCoinAcceptor = value;
            }
        }
    }

    public bool AllowCoinBypass
    {
        get
        {
            lock (_lock)
            {
                return _allowCoinBypass;
            }
        }
        set
        {
            lock (_lock)
            {
                _allowCoinBypass = value;
            }
        }
    }

    public bool EnableCoinSimulationButton
    {
        get
        {
            lock (_lock)
            {
                return _enableCoinSimulationButton;
            }
        }
        set
        {
            lock (_lock)
            {
                _enableCoinSimulationButton = value;
            }
        }
    }
}


