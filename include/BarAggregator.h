#include <vector>
#include "StockData.h"

class BarAggregator {
private:
    uint64_t timeframe_ms;
    uint64_t current_bar_start;
    int64_t current_open;
    int64_t current_high;
    int64_t current_low;
    int64_t current_close;
    bool has_active_bar;

public:
    // 傳入時間區間 (毫秒)，例如 60000 代表 1 分鐘
    BarAggregator(uint64_t timeframe_ms) 
        : timeframe_ms(timeframe_ms), current_bar_start(0), 
          current_open(0), current_high(0), current_low(0), current_close(0), 
          has_active_bar(false) {}

    // 核心流式處理：傳入全部的 Tick，回傳聚合完成的 K 線陣列
    std::vector<BarData> aggregate(const std::vector<StockData>& ticks) {
        std::vector<BarData> bars;
        if (ticks.empty()) return bars;

        // 預分配記憶體，避免高頻再配置開銷
        bars.reserve(ticks.size() / 1000 + 1); 

        for (const auto& tick : ticks) {
            // 第一筆資料初始化
            if (!has_active_bar) {
                current_bar_start = tick.timestamp;
                current_open = tick.price;
                current_high = tick.price;
                current_low = tick.price;
                current_close = tick.price;
                has_active_bar = true;
                continue;
            }

            // 判斷是否跨越時間邊界
            if (tick.timestamp - current_bar_start >= timeframe_ms) {
                // 封裝並推入上一根完成的 K 線
                bars.push_back({current_bar_start, current_open, current_high, current_low, current_close});
                
                // 初始化下一根 K 線，以當前 Tick 作為開盤
                // 為了避免微秒級的時間漂移，新 Bar 起始時間應為基準時間的遞增
                current_bar_start = current_bar_start + timeframe_ms;
                // 若當前 Tick 時間落後太多，則直接校準至當前 Tick
                if (tick.timestamp > current_bar_start + timeframe_ms) {
                    current_bar_start = tick.timestamp;
                }
                current_open = tick.price;
                current_high = tick.price;
                current_low = tick.price;
                current_close = tick.price;
            } else {
                // 仍在同一根 K 線內，動態更新極值
                if (tick.price > current_high) current_high = tick.price;
                if (tick.price < current_low)  current_low = tick.price;
                current_close = tick.price;
            }
        }

        // 處理最後一根未滿區間的殘留 K 線
        if (has_active_bar) {
            bars.push_back({current_bar_start, current_open, current_high, current_low, current_close});
        }

        return bars;
    }
};