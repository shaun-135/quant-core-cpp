#pragma once
#include "CircularBuffer.h"

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