#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include "../include/StockData.h"
#include "../include/DataLoader.h"

//測試最佳情況下的收益

void calculateMaxTheoreticalProfit(const std::vector<StockData>& ticks) {
    std::cout << "--- QuantCore Theoretical Max Profit Report ---" << std::endl;
    
    if (ticks.empty()) return;

    constexpr double DISPLAY_SCALE = 100000000.0; 
    constexpr int64_t TICK_SIZE = 100000000LL;
    
    // 完美對齊你目前的交易成本結構
    constexpr int64_t SLIPPAGE_TICKS = 1; 
    constexpr int64_t FEE_BPS = 4;        

    // DP 狀態初始化
    // cash: 手上不持有部位時的最大理論淨利
    // hold: 手上持有部位時的最大理論狀態（因先墊付買入成本，初始設為極小值）
    int64_t cash = 0;
    int64_t hold = -9223372036854775807LL; // int64_t 最小值

    for (const auto& data : ticks) {
        int64_t price = data.price; // 已是放大 10^8 的整數

        // 計算該 Tick 點位的精確買入總支出 (含滑價與手續費)
        int64_t buy_cost = price + (SLIPPAGE_TICKS * TICK_SIZE);
        buy_cost += (buy_cost * FEE_BPS) / 10000;

        // 計算該 Tick 點位的精確賣出總收入 (扣除滑價與手續費)
        int64_t sell_income = price - (SLIPPAGE_TICKS * TICK_SIZE);
        sell_income -= (sell_income * FEE_BPS) / 10000;

        // 動態規劃狀態轉移方程
        int64_t next_cash = std::max(cash, hold + sell_income);
        int64_t next_hold = std::max(hold, cash - buy_cost);

        cash = next_cash;   
        hold = next_hold;
    }

    std::cout << "Total Tick Records Analyzed: " << ticks.size() << std::endl;
    std::cout << "Theoretical Max Net Profit : " 
              << std::fixed << std::setprecision(2) << (cash / DISPLAY_SCALE) << " USD" << std::endl;
}

int main() {
    // 載入CSV
    std::vector<StockData> ticks = loadCSV("../data/BTCUSDT-trades-2026-05-25.csv");
    
    if (ticks.empty()) {
        return 1;
    }

    calculateMaxTheoreticalProfit(ticks);

    return 0;
}