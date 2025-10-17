using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace ChargingKiosk.Migrations
{
    /// <inheritdoc />
    public partial class Edit_CoinDenominations : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.AddColumn<int>(
                name: "ChargingMinutes",
                table: "CoinDenominations",
                type: "INTEGER",
                nullable: false,
                defaultValue: 0);

            migrationBuilder.UpdateData(
                table: "CoinDenominations",
                keyColumn: "Id",
                keyValue: 1,
                column: "ChargingMinutes",
                value: 10);

            migrationBuilder.UpdateData(
                table: "CoinDenominations",
                keyColumn: "Id",
                keyValue: 2,
                column: "ChargingMinutes",
                value: 30);

            migrationBuilder.UpdateData(
                table: "CoinDenominations",
                keyColumn: "Id",
                keyValue: 3,
                column: "ChargingMinutes",
                value: 60);

            migrationBuilder.UpdateData(
                table: "CoinDenominations",
                keyColumn: "Id",
                keyValue: 4,
                column: "ChargingMinutes",
                value: 120);
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropColumn(
                name: "ChargingMinutes",
                table: "CoinDenominations");
        }
    }
}
