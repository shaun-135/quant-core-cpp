#pragma once
#include "CircularBuffer.h"
#include <cmath>

class IStrategy {
public:
    virtual ~IStrategy() = default;
    virtual int generateSignal(int64_t currentPrice) = 0; 
};

class MACrossStrategy : public IStrategy {
private:
    CircularBuffer<int64_t> maBuffer;
    int windowSize;
    int count;
    int64_t threshold_value;
    
    // 新增：狀態機 (1: 做多狀態, -1: 做空狀態, 0: 空手)
    int current_position; 

public:
    MACrossStrategy(int windowSize, int ticks) 
        : maBuffer(windowSize), windowSize(windowSize), count(0), 
          threshold_value(ticks * 1000000LL), current_position(0) {}

    int generateSignal(int64_t currentPrice) override {
        maBuffer.add(currentPrice);
        count++;

        if (count < windowSize) {
            return 0; 
        }

        int64_t currentMA = maBuffer.getAverage();
        int signal = 0;

        // 如果當前不是多單，且價格突破上緣 -> 買入
        if (current_position <= 0 && currentPrice > (currentMA + threshold_value)) {
            current_position = 1; 
            signal = 1;
        } 
        // 如果當前不是空單，且價格跌破下緣 -> 賣出
        else if (current_position >= 0 && currentPrice < (currentMA - threshold_value)) {
            current_position = -1;
            signal = -1;
        }

        return signal;
    }
};

// 附加於 include/Strategy.h 中
class BuyAndHoldStrategy : public IStrategy {
private:
    bool hasBought = false;

public:
    // 根據你 main.cpp 的呼叫邏輯，參數為 int64_t closePrice
    int generateSignal(int64_t closePrice) override {
        if (!hasBought) {
            hasBought = true;
            return 1; // 買入訊號
        }
        return 0; // 不動
    }
};


class RSIStrategy : public IStrategy {
private:
    int period;
    double lowerBound; // 超賣線 (例如 30)
    double upperBound; // 超買線 (例如 70)
    CircularBuffer<int64_t> gainBuffer;
    CircularBuffer<int64_t> lossBuffer;
    bool isHolding;
    int64_t lastClose;

public:
    // 修改建構子，接收週期與上下邊界
    RSIStrategy(int period, double lowerBound, double upperBound) 
        : period(period), lowerBound(lowerBound), upperBound(upperBound),
          gainBuffer(period), lossBuffer(period), isHolding(false), lastClose(0) {}
    int generateSignal(int64_t closePrice) override {
        // 第一筆資料沒有歷史價格，無法計算漲跌，直接紀錄後跳過
        if (lastClose == 0) {
            lastClose = closePrice;
            return 0;
        }

        // 1. 計算與前一根 K 線的差異
        int64_t diff = closePrice - lastClose;
        lastClose = closePrice;

        // 2. 分流寫入緩衝區 (上漲寫入 gain，下跌寫入 loss)
        if (diff > 0) {
            gainBuffer.add(diff);
            lossBuffer.add(0); // 沒跌，記為 0
        } else {
            gainBuffer.add(0); // 沒漲，記為 0
            lossBuffer.add(-diff); // 取絕對值存入
        }

        // 3. 資料筆數未達 period，指標無效，不動作
        if (!gainBuffer.isFull()) {
            return 0;
        }

        // 4. O(1) 獲取平均漲跌幅
        int64_t avgGain = gainBuffer.getAverage();
        int64_t avgLoss = lossBuffer.getAverage();

        // 5. 邊界條件處理
        if (avgGain == 0 && avgLoss == 0) return 0;
        if (avgLoss == 0) return (isHolding ? -1 : 0); // 絕對強勢，無跌幅

        // 6. 計算相對強弱指標 RSI
        double rs = static_cast<double>(avgGain) / static_cast<double>(avgLoss);
        double rsi = 100.0 - (100.0 / (1.0 + rs));

        // 使用動態變數替換原本寫死的 30.0 與 70.0
        if (!isHolding && rsi < lowerBound) {
            isHolding = true;
            return 1;
        } 
        else if (isHolding && rsi > upperBound) {
            isHolding = false;
            return -1;
        }

        return 0;
    }
};