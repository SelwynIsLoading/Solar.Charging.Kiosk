using System.Net.Http.Json;

namespace ChargingKiosk.Services;

public class ArduinoApiService : IArduinoApiService
{
    private readonly HttpClient _httpClient;
    private readonly ILogger<ArduinoApiService> _logger;
    private readonly string _pythonApiBaseUrl;

    public ArduinoApiService(HttpClient httpClient, IConfiguration configuration, ILogger<ArduinoApiService> logger)
    {
        _httpClient = httpClient;
        _logger = logger;
        _pythonApiBaseUrl = configuration["PythonApi:BaseUrl"] ?? "http://localhost:8000";
        _httpClient.BaseAddress = new Uri(_pythonApiBaseUrl);
    }

    public async Task<bool> ControlRelayAsync(int slotNumber, bool turnOn)
    {
        try
        {
            var response = await _httpClient.PostAsJsonAsync("/api/relay", new
            {
                slotNumber,
                state = turnOn
            });
            
            return response.IsSuccessStatusCode;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, $"Failed to control relay for slot {slotNumber}");
            return false;
        }
    }

    public async Task<bool> ControlSolenoidAsync(int slotNumber, bool lockState, int durationSeconds = 0)
    {
        try
        {
            var response = await _httpClient.PostAsJsonAsync("/api/solenoid", new
            {
                slotNumber,
                locked = lockState,
                duration = durationSeconds
            });
            
            return response.IsSuccessStatusCode;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, $"Failed to control solenoid for slot {slotNumber}");
            return false;
        }
    }

    public async Task<bool> ControlUVLightAsync(int slotNumber, bool turnOn)
    {
        try
        {
            var response = await _httpClient.PostAsJsonAsync("/api/uv-light", new
            {
                slotNumber,
                state = turnOn
            });
            
            return response.IsSuccessStatusCode;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, $"Failed to control UV light for slot {slotNumber}");
            return false;
        }
    }

    public async Task<bool> VerifyFingerprintAsync(int fingerprintId)
    {
        try
        {
            _logger.LogInformation("Verifying fingerprint ID: {FingerprintId}", fingerprintId);
            var response = await _httpClient.PostAsJsonAsync("/api/fingerprint/verify", new
            {
                fingerprintId
            });
            
            if (response.IsSuccessStatusCode)
            {
                var result = await response.Content.ReadFromJsonAsync<FingerprintResponse>();
                return result?.IsValid ?? false;
            }
            
            return false;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to verify fingerprint");
            return false;
        }
    }

    public async Task<bool> EnrollFingerprintAsync(int fingerprintId)
    {
        try
        {
            _logger.LogInformation($"Enrolling fingerprint ID: {fingerprintId}");
            
            var response = await _httpClient.PostAsJsonAsync("/api/fingerprint/enroll", new
            {
                fingerprintId
            });
            
            if (response.IsSuccessStatusCode)
            {
                var result = await response.Content.ReadFromJsonAsync<EnrollmentResponse>();
                
                if (result?.Success == true)
                {
                    _logger.LogInformation($"Fingerprint {fingerprintId} enrolled successfully");
                    return true;
                }
            }
            
            _logger.LogWarning($"Fingerprint enrollment failed for ID: {fingerprintId}");
            return false;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to enroll fingerprint");
            return false;
        }
    }

    public async Task<decimal> GetCoinSlotValueAsync()
    {
        try
        {
            var response = await _httpClient.GetAsync("/api/coin-slot");
            
            if (response.IsSuccessStatusCode)
            {
                var result = await response.Content.ReadFromJsonAsync<CoinSlotResponse>();
                return result?.Value ?? 0;
            }
            
            return 0;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to get coin slot value");
            return 0;
        }
    }

    public async Task<bool> UnlockTemporaryAsync(int slotNumber)
    {
        try
        {
            _logger.LogInformation($"Temporary unlock for slot {slotNumber}");
            
            var response = await _httpClient.PostAsJsonAsync("/api/solenoid/unlock-temp", new
            {
                slotNumber
            });
            
            return response.IsSuccessStatusCode;
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, $"Failed to temporarily unlock slot {slotNumber}");
            return false;
        }
    }
    
    private class FingerprintResponse
    {
        public bool IsValid { get; set; }
        public int FingerprintId { get; set; }
        public int Confidence { get; set; }
    }
    
    private class EnrollmentResponse
    {
        public bool Success { get; set; }
        public int FingerprintId { get; set; }
        public string Message { get; set; } = string.Empty;
    }
    
    private class CoinSlotResponse
    {
        public decimal Value { get; set; }
    }
}

