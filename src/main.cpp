#include <iostream>
#include <vector>
#include <memory>
#include <iomanip>
#include <string>
#include "../include/StockData.h"
#include "../include/DataLoader.h"
#include "../include/Strategy.h"
#include "../include/BarAggregator.h"

// 定義回測結果結構
struct BacktestResult {
    int64_t totalGrossProfit;
    int64_t totalNetProfit;
    int64_t totalFeesPaid;
    int tradeCount;
};

// 獨立化回測邏輯，接收策略並回傳結果
BacktestResult runBarBacktest(const std::vector<BarData>& bars, std::unique_ptr<IStrategy> strategy, const std::string& strategyName) {
    std::cout << "--- Starting Backtest: " << strategyName << " ---\n";
    
    bool isHolding = false;
    int64_t entryPrice = 0;  
    int64_t entryFee = 0;    
    
    int64_t totalGrossProfit = 0; 
    int64_t totalNetProfit = 0;   
    int64_t totalFeesPaid = 0;    
    int tradeCount = 0;
    
    constexpr double DISPLAY_SCALE = 100000000.0; 
    constexpr int64_t TICK_SIZE = 1000000LL;
    constexpr int64_t SLIPPAGE_TICKS = 1; 
    constexpr int64_t FEE_BPS = 4;        

    for (size_t i = 0; i < bars.size() - 1; ++i) {
        int signal = strategy->generateSignal(bars[i].close); 

        if (!isHolding && signal == 1) {
            isHolding = true;
            tradeCount++;
            
            int64_t executionPrice = bars[i+1].open;
            entryPrice = executionPrice + (SLIPPAGE_TICKS * TICK_SIZE);
            entryFee = (entryPrice * FEE_BPS) / 10000;
            totalFeesPaid += entryFee;

            std::cout << "[" << strategyName << "] [BUY]  BarTS: " << bars[i+1].timestamp 
                      << " @ " << std::fixed << std::setprecision(2) << (entryPrice / DISPLAY_SCALE) << "\n";
        } 
        else if (isHolding && signal == -1) {
            isHolding = false;
            
            int64_t executionPrice = bars[i+1].open;
            int64_t exitPrice = executionPrice - (SLIPPAGE_TICKS * TICK_SIZE);
            int64_t exitFee = (exitPrice * FEE_BPS) / 10000;
            totalFeesPaid += exitFee;

            int64_t grossProfit = exitPrice - entryPrice; 
            int64_t netProfit = grossProfit - entryFee - exitFee;
            
            totalGrossProfit += grossProfit;
            totalNetProfit += netProfit;

            std::cout << "[" << strategyName << "] [SELL] BarTS: " << bars[i+1].timestamp 
                      << " @ " << std::fixed << std::setprecision(2) << (exitPrice / DISPLAY_SCALE) 
                      << " | Net: " << (netProfit / DISPLAY_SCALE) << "\n";
        }
    }

    if (isHolding) {
        tradeCount++;
        int64_t exitPrice = bars.back().close - (SLIPPAGE_TICKS * TICK_SIZE);
        int64_t exitFee = (exitPrice * FEE_BPS) / 10000;
        totalFeesPaid += exitFee;

        int64_t grossProfit = exitPrice - entryPrice;
        int64_t netProfit = grossProfit - entryFee - exitFee;
        
        totalGrossProfit += grossProfit;
        totalNetProfit += netProfit;

        std::cout << "[" << strategyName << "] [FORCED CLOSE] @ " 
                  << std::fixed << std::setprecision(2) << (exitPrice / DISPLAY_SCALE) 
                  << " | Net: " << (netProfit / DISPLAY_SCALE) << "\n";
    }
    
    std::cout << "--------------------------------------\n";

    return {totalGrossProfit, totalNetProfit, totalFeesPaid, tradeCount};
}

int main() {
    std::vector<StockData> ticks = loadCSV("data/BTCUSDT-trades-2026-05-25.csv");
    if (ticks.empty()) return 1;

    BarAggregator aggregator(60000000LL);
    std::vector<BarData> bars = aggregator.aggregate(ticks);

    auto maStrategy = std::make_unique<MACrossStrategy>(20, 2);
    auto bnhStrategy = std::make_unique<BuyAndHoldStrategy>();

    BacktestResult maResult = runBarBacktest(bars, std::move(maStrategy), "MA Strategy");
    BacktestResult bnhResult = runBarBacktest(bars, std::move(bnhStrategy), "Buy & Hold");

    constexpr double DISPLAY_SCALE = 100000000.0;
    std::cout << "\n=== QuantCore Benchmark Report ===\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "[MA Strategy] Net PnL: " << (maResult.totalNetProfit / DISPLAY_SCALE) 
              << " | Trades: " << maResult.tradeCount 
              << " | Fees: " << (maResult.totalFeesPaid / DISPLAY_SCALE) << "\n";
    std::cout << "[Buy & Hold]  Net PnL: " << (bnhResult.totalNetProfit / DISPLAY_SCALE) 
              << " | Trades: " << bnhResult.tradeCount 
              << " | Fees: " << (bnhResult.totalFeesPaid / DISPLAY_SCALE) << "\n";
    std::cout << "==================================\n";

    return 0;
}