#include <iostream>
#include <vector>
#include <memory>
#include <iomanip>
#include "../include/StockData.h"
#include "../include/DataLoader.h"
#include "../include/Strategy.h"
#include "../include/BarAggregator.h"
#include "../include/BacktestEngine.h" // 引入新架構

int main() {
    // 1. 載入資料 (路徑已對齊專案根目錄)
    std::vector<StockData> ticks = loadCSV("data/BTCUSDT-trades-2026-05-25.csv");
    if (ticks.empty()) return 1;

    // 2. K線降頻
    BarAggregator aggregator(60000000LL);
    std::vector<BarData> bars = aggregator.aggregate(ticks);

    // 3. 實例化策略
    auto maStrategy = std::make_unique<MACrossStrategy>(20, 2);
    auto bnhStrategy = std::make_unique<BuyAndHoldStrategy>();
    auto rsiStrategy = std::make_unique<RSIStrategy>(14); // 實例化 14 週期的 RSI 策略

    // 4. 初始化獨立的回測引擎
    BacktestEngine engine;

    // 5. 執行多策略回測 (內部會自動於 logs/ 產出獨立的 CSV 檔)
    BacktestResult maResult = engine.run(bars, std::move(maStrategy), "MA_Strategy");
    BacktestResult bnhResult = engine.run(bars, std::move(bnhStrategy), "BuyAndHold_Strategy");
    BacktestResult rsiResult = engine.run(bars, std::move(rsiStrategy), "RSI_Strategy"); // 執行 RSI 回測

    // 6. 印出最終基準線比較報表
    constexpr double DISPLAY_SCALE = 100000000.0;
    std::cout << "\n=== QuantCore Benchmark Report ===\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "[MA Strategy] Net PnL: " << (maResult.totalNetProfit / DISPLAY_SCALE) 
              << " | Trades: " << maResult.tradeCount 
              << " | Fees: " << (maResult.totalFeesPaid / DISPLAY_SCALE) << "\n";
    std::cout << "[RSI Strategy] Net PnL: " << (rsiResult.totalNetProfit / DISPLAY_SCALE) 
              << " | Trades: " << rsiResult.tradeCount 
              << " | Fees: " << (rsiResult.totalFeesPaid / DISPLAY_SCALE) << "\n";
    std::cout << "[Buy & Hold]  Net PnL: " << (bnhResult.totalNetProfit / DISPLAY_SCALE) 
            << " | Trades: " << bnhResult.tradeCount 
            << " | Fees: " << (bnhResult.totalFeesPaid / DISPLAY_SCALE) << "\n";
    std::cout << "==================================\n";

    return 0;
}