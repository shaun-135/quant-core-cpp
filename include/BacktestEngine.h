#ifndef BACKTEST_ENGINE_H
#define BACKTEST_ENGINE_H

#include <vector>
#include <memory>
#include <string>
#include "StockData.h" // 確保能讀取 BarData 結構
#include "Strategy.h"  // 確保能讀取 IStrategy 介面

struct BacktestResult {
    int64_t totalGrossProfit;
    int64_t totalNetProfit;
    int64_t totalFeesPaid;
    int tradeCount;
};

class BacktestEngine {
public:
    BacktestEngine() = default;
    ~BacktestEngine() = default;

    // 執行回測，並自動將交易紀錄匯出至對應的 CSV 檔案
    BacktestResult run(const std::vector<BarData>& bars, 
                       std::unique_ptr<IStrategy> strategy, 
                       const std::string& strategyName);
};

#endif // BACKTEST_ENGINE_H