#pragma once
#include "CircularBuffer.h"

// 策略純虛擬類別 (介面)
class IStrategy {
public:
    virtual ~IStrategy() = default;
    // 回傳 1 (買入), -1 (賣出), 0 (無動作)
    virtual int generateSignal(double currentPrice) = 0; 
};

// 具體策略：MA 交叉
class MACrossStrategy : public IStrategy {
private:
    CircularBuffer<double> maBuffer;
    int windowSize;
    int count;
    double prevMA;
    double prevPrice;

public:
    MACrossStrategy(int windowSize) 
        : maBuffer(windowSize), windowSize(windowSize), count(0), prevMA(0.0), prevPrice(0.0) {}

    int generateSignal(double currentPrice) override {
        maBuffer.add(currentPrice);
        count++;

        // 資料筆數不足以計算初始均線時，不動作
        if (count < windowSize) {
            prevPrice = currentPrice;
            return 0; 
        }

        double currentMA = maBuffer.getAverage();
        int signal = 0;

        // 確保有上一筆的 MA 可以進行交叉比對
        if (count > windowSize) {
            if (prevPrice <= prevMA && currentPrice > currentMA) {
                signal = 1; // 黃金交叉
            } 
            else if (prevPrice >= prevMA && currentPrice < currentMA) {
                signal = -1; // 死亡交叉
            }
        }

        prevPrice = currentPrice;
        prevMA = currentMA;
        return signal;
    }
};