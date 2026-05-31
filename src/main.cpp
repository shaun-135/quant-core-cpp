#include <iostream>
#include <vector>
#include <memory>
#include <iomanip>
#include "../include/StockData.h"
#include "../include/DataLoader.h"
#include "../include/Strategy.h"
#include "../include/BarAggregator.h"
#include "../include/BacktestEngine.h"

int main() {
    std::cout << "Loading Data...\n";
    std::vector<StockData> ticks = loadCSV("data/BTCUSDT-trades-2026-05-25.csv");
    if (ticks.empty()) return 1;

    BarAggregator aggregator(60000000LL);
    std::vector<BarData> bars = aggregator.aggregate(ticks);
    
    BacktestEngine engine;
    constexpr double SCALE = 100000000.0;

    std::cout << "\n=== Phase 1: Grid Search Optimization (RSI) ===\n";
    
    int64_t bestNetPnL = -999999999999LL;
    int bestPeriod = 0;
    double bestLower = 0, bestUpper = 0;

    for (int p = 10; p <= 20; p += 2) {           
        for (double lower = 20; lower <= 40; lower += 5) { 
            double upper = 100 - lower;           

            auto rsiStrat = std::make_unique<RSIStrategy>(p, lower, upper);
            BacktestResult res = engine.run(bars, std::move(rsiStrat), "temp_search"); 

            if (res.totalNetProfit > bestNetPnL) {
                bestNetPnL = res.totalNetProfit;
                bestPeriod = p;
                bestLower = lower;
                bestUpper = upper;
            }
        }
    }

    std::cout << "\n=== Optimization Complete ===\n";
    std::cout << "Best Parameters -> Period: " << bestPeriod 
              << ", Lower: " << bestLower 
              << ", Upper: " << bestUpper << "\n";
    std::cout << "Max Net PnL Expected: " << (bestNetPnL / SCALE) << " USD\n";

    std::cout << "\n=== Phase 2: Generating Golden Dataset for Python ===\n";
    
    auto bestRsiStrat = std::make_unique<RSIStrategy>(bestPeriod, bestLower, bestUpper);
    BacktestResult finalResult = engine.run(bars, std::move(bestRsiStrat), "Best_RSI_Strategy");

    std::cout << "\nGolden CSV exported as: logs/Best_RSI_Strategy_trades.csv\n";
    
    // ==========================================
    // Phase 3: Final Benchmark Report (終極對決報表)
    // ==========================================
    
    // 重新跑一次 MA 與 Buy & Hold 作為對照組
    auto maStrategy = std::make_unique<MACrossStrategy>(20, 2);
    auto bnhStrategy = std::make_unique<BuyAndHoldStrategy>();

    BacktestResult maResult = engine.run(bars, std::move(maStrategy), "MA_Strategy");
    BacktestResult bnhResult = engine.run(bars, std::move(bnhStrategy), "BuyAndHold_Strategy");

    std::cout << "\n=== Final QuantCore Benchmark Report ===\n";
    std::cout << "[MA Strategy] Net PnL: " << std::fixed << std::setprecision(2) << (maResult.totalNetProfit / SCALE) 
              << " | Trades: " << maResult.tradeCount << "\n";
    std::cout << "[Buy & Hold]  Net PnL: " << std::fixed << std::setprecision(2) << (bnhResult.totalNetProfit / SCALE) 
              << " | Trades: " << bnhResult.tradeCount << "\n";
    std::cout << "[Best RSI]    Net PnL: " << std::fixed << std::setprecision(2) << (finalResult.totalNetProfit / SCALE) 
              << " | Trades: " << finalResult.tradeCount << "\n";
    std::cout << "========================================\n";

    return 0;
}