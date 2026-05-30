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

    // 關閉終端機的大量輸出，以免洗版 (如果你引擎內有 cout，這裡搜尋時會印很多，實務上引擎的 cout 應該加開關，但為了速度我們先不管)
    for (int p = 10; p <= 20; p += 2) {           // 週期從 10 到 20
        for (double lower = 20; lower <= 40; lower += 5) { // 超賣線 20 到 40
            double upper = 100 - lower;           // 對稱超買線 80 到 60

            auto rsiStrat = std::make_unique<RSIStrategy>(p, lower, upper);
            // 使用 "temp" 作為名稱，這些 CSV 之後可以無視或刪除
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
    
    // 用最好的參數跑最後一次，並命名為 Best_RSI_Strategy，這會產出我們畫圖要用的 CSV
    auto bestRsiStrat = std::make_unique<RSIStrategy>(bestPeriod, bestLower, bestUpper);
    BacktestResult finalResult = engine.run(bars, std::move(bestRsiStrat), "Best_RSI_Strategy");

    std::cout << "\nGolden CSV exported as: logs/Best_RSI_Strategy_trades.csv\n";
    
    return 0;
}