#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include "StockData.h"
#include "CircularBuffer.h"

// --- 輔助函式：清理字串 ---
std::string cleanStr(std::string str) {
    str.erase(std::remove(str.begin(), str.end(), '\"'), str.end());
    str.erase(std::remove(str.begin(), str.end(), ','), str.end());
    return str;
}

// --- 資料讀取模組 ---
std::vector<StockData> loadCSV(const std::string& filename) {
    std::vector<StockData> prices;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open " << filename << std::endl;
        return prices;
    }

    std::string line;
    std::getline(file, line); // Skip Header

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string date, priceStr;

        // 根據格式: "Date","Price","Open","High","Low"...
        std::getline(ss, date, ',');      // 第 1 欄: Date
        std::getline(ss, priceStr, ',');   // 第 2 欄: Price (收盤價)

        try {
            StockData d;
            d.date = cleanStr(date);
            d.close = std::stod(cleanStr(priceStr));
            prices.push_back(d);
        } catch (...) { continue; }
    }
    
    // 將資料從「新到舊」反轉為「舊到新」
    std::reverse(prices.begin(), prices.end());
    return prices;
}

// --- 2. 策略執行模組 ---
void runBacktest(const std::vector<StockData>& prices, int maPeriod) {
    std::vector<double> ma_list;
    CircularBuffer<double> window(maPeriod);

    // 計算均線
    for (const auto& data : prices) {
        window.add(data.close);
        ma_list.push_back(window.getAverage());
    }

    std::cout << "--- QuantCore Backtest Report ---" << std::endl;
    
    bool isHolding = false;
    double entryPrice = 0.0;
    double totalProfit = 0.0;
    int tradeCount = 0;

    // 從 maPeriod 開始，因為前面資料不足
    for (size_t i = maPeriod; i < prices.size(); ++i) {
        double currentPrice = prices[i].close;
        double currentMA = ma_list[i];
        double prevPrice = prices[i-1].close;
        double prevMA = ma_list[i-1];

        // 策略邏輯：黃金交叉買入 (昨天在MA下，今天衝上MA)
        if (!isHolding && prevPrice <= prevMA && currentPrice > currentMA) {
            isHolding = true;
            entryPrice = currentPrice;
            tradeCount++;
            std::cout << "[BUY]  " << prices[i].date << " @ " << entryPrice << std::endl;
        } 
        // 策略邏輯：死亡交叉賣出 (昨天在MA上，今天跌破MA)
        else if (isHolding && prevPrice >= prevMA && currentPrice < currentMA) {
            isHolding = false;
            double profit = currentPrice - entryPrice;
            totalProfit += profit;
            std::cout << "[SELL] " << prices[i].date << " @ " << currentPrice 
                      << " | Profit: " << profit << std::endl;
        }
    }

    std::cout << "---------------------------------" << std::endl;
    std::cout << "Total Trades: " << tradeCount << std::endl;
    std::cout << "Cumulative Profit: " << totalProfit << std::endl;
}

// --- 3. 主程式流程 ---
int main() {
    // A. 載入資料
    std::vector<StockData> prices = loadCSV("../data/2330.TW.csv");
    
    if (prices.empty()) {
        return 1;
    }

    std::cout << "Data loaded. Period: " << prices.front().date << " to " << prices.back().date << std::endl;
    std::cout << "Total records: " << prices.size() << std::endl;

    // B. 執行回測策略 (MA20)
    runBacktest(prices, 20);

    return 0;
}