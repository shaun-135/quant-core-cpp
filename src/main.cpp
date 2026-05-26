#include <iostream>
#include <vector>
#include <memory>
#include "../include/StockData.h"
#include "../include/DataLoader.h"
#include "../include/Strategy.h"

void runBacktest(const std::vector<StockData>& prices, std::unique_ptr<IStrategy> strategy) {
    std::cout << "--- QuantCore High-Frequency Backtest Report ---" << std::endl;
    
    bool isHolding = false;
    double entryPrice = 0.0;
    double totalProfit = 0.0;
    int tradeCount = 0;

    for (const auto& data : prices) {
        // 策略介面若原本傳入 double，此處直接帶入 data.price
        int signal = strategy->generateSignal(data.price); 

        if (!isHolding && signal == 1) {
            isHolding = true;
            entryPrice = data.price;
            tradeCount++;
            std::cout << "[BUY]  TS: " << data.timestamp << " @ " << entryPrice << std::endl;
        } 
        else if (isHolding && signal == -1) {
            isHolding = false;
            double profit = data.price - entryPrice;
            totalProfit += profit;
            std::cout << "[SELL] TS: " << data.timestamp << " @ " << data.price 
                      << " | Profit: " << profit << std::endl;
        }
    }

    std::cout << "---------------------------------" << std::endl;
    std::cout << "Total Trades: " << tradeCount << std::endl;
    std::cout << "Cumulative Profit: " << totalProfit << std::endl;
}

int main() {
    // 1. 載入幣安高頻資料 (路徑依實際下載檔案命名)
    std::vector<StockData> prices = loadCSV("../data/BTCUSDT-trades-2026-05-25.csv");
    
    if (prices.empty()) {
        return 1;
    }

    std::cout << "Data loaded. Start TS: " << prices.front().timestamp 
              << " to End TS: " << prices.back().timestamp << std::endl;
    std::cout << "Total Tick records: " << prices.size() << std::endl;

    // 2. 實例化 MA 策略 (快線/慢線參數需適配高頻 Tick 視窗大小，例如 200 筆交易均線)
    std::unique_ptr<IStrategy> myStrategy = std::make_unique<MACrossStrategy>(200);

    // 3. 執行回測
    runBacktest(prices, std::move(myStrategy));

    return 0;
}