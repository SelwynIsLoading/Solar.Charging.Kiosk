using ChargingKiosk.Models;
using Microsoft.AspNetCore.Identity.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore;

namespace ChargingKiosk.Data;

public class ApplicationDbContext(DbContextOptions<ApplicationDbContext> options)
    : IdentityDbContext<ApplicationUser>(options)
{
    public DbSet<Transaction> Transactions { get; set; }
    public DbSet<CoinDenomination> CoinDenominations { get; set; }

    protected override void OnModelCreating(ModelBuilder builder)
    {
        base.OnModelCreating(builder);

        // Seed coin denominations with charging time
        builder.Entity<CoinDenomination>().HasData(
            new CoinDenomination { Id = 1, Name = "1 Peso", Value = 1.00m, ChargingMinutes = 10, IsActive = true },
            new CoinDenomination { Id = 2, Name = "5 Pesos", Value = 5.00m, ChargingMinutes = 30, IsActive = true },
            new CoinDenomination { Id = 3, Name = "10 Pesos", Value = 10.00m, ChargingMinutes = 60, IsActive = true },
            new CoinDenomination { Id = 4, Name = "20 Pesos", Value = 20.00m, ChargingMinutes = 120, IsActive = true }
        );
    }
}