#include "../include/BacktestEngine.h"
#include <iostream>
#include <fstream>
#include <iomanip>

BacktestResult BacktestEngine::run(const std::vector<BarData>& bars, 
                                   std::unique_ptr<IStrategy> strategy, 
                                   const std::string& strategyName) {
    std::cout << "--- Starting Backtest: " << strategyName << " ---\n";
    
    // 初始化 CSV 輸出串流，動態生成檔案名稱防止互相覆蓋
    std::ofstream csvFile("logs/" + strategyName + "_trades.csv");
    if (!csvFile.is_open()) {
        std::cerr << "[Error] Failed to open CSV file for strategy: " << strategyName << "\n";
    } else {
        // 寫入 CSV 標題列
        csvFile << "timestamp,action,price,net_pnl,fees_paid\n";
    }

    bool isHolding = false;
    int64_t entryPrice = 0;  
    int64_t entryFee = 0;    
    
    int64_t totalGrossProfit = 0; 
    int64_t totalNetProfit = 0;   
    int64_t totalFeesPaid = 0;    
    int tradeCount = 0;
    
    constexpr double DISPLAY_SCALE = 100000000.0; 
    constexpr int64_t TICK_SIZE = 1000000LL; // 0.01 USDT
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

            // 終端機 Log
            std::cout << "[" << strategyName << "] [BUY]  BarTS: " << bars[i+1].timestamp 
                      << " @ " << std::fixed << std::setprecision(2) << (entryPrice / DISPLAY_SCALE) << "\n";
            
            // 寫入 CSV
            if (csvFile.is_open()) {
                csvFile << bars[i+1].timestamp << ",BUY," 
                        << (entryPrice / DISPLAY_SCALE) << ",0," 
                        << (entryFee / DISPLAY_SCALE) << "\n";
            }
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

            // 終端機 Log
            std::cout << "[" << strategyName << "] [SELL] BarTS: " << bars[i+1].timestamp 
                      << " @ " << std::fixed << std::setprecision(2) << (exitPrice / DISPLAY_SCALE) 
                      << " | Net: " << (netProfit / DISPLAY_SCALE) << "\n";
            
            // 寫入 CSV
            if (csvFile.is_open()) {
                csvFile << bars[i+1].timestamp << ",SELL," 
                        << (exitPrice / DISPLAY_SCALE) << "," 
                        << (netProfit / DISPLAY_SCALE) << "," 
                        << (exitFee / DISPLAY_SCALE) << "\n";
            }
        }
    }

    // 期末強制平倉結算
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
                  
        if (csvFile.is_open()) {
            csvFile << bars.back().timestamp << ",FORCED_CLOSE," 
                    << (exitPrice / DISPLAY_SCALE) << "," 
                    << (netProfit / DISPLAY_SCALE) << "," 
                    << (exitFee / DISPLAY_SCALE) << "\n";
        }
    }
    
    std::cout << "--------------------------------------\n";
    
    if (csvFile.is_open()) {
        csvFile.close();
    }

    return {totalGrossProfit, totalNetProfit, totalFeesPaid, tradeCount};
}