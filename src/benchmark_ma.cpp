#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include "../include/LinearBuffer.h"   // 已修正為相對 include 目錄
#include "../include/CircularBuffer.h" // 已修正為相對 include 目錄

// ==========================================
// 核心切換開關：每次編譯請只保留一個巨集處於開啟狀態，避免 CPU Cache 污染影響準確度
// ==========================================
// #define TEST_LINEAR
#define TEST_CIRCULAR 
// ==========================================

int main() {
    const size_t DATA_SIZE = 1000000;
    const size_t WINDOW_SIZE = 1000; 
    constexpr int64_t PRICE_SCALE = 100000000LL; // 10^8 放大因子

    std::cout << "[Benchmark] Generating " << DATA_SIZE << " random int64_t data points...\n";
    std::vector<int64_t> mockData(DATA_SIZE); // 改為 int64_t
    
    std::mt19937 gen(42); 
    // 改用 64 位元整數分佈，模擬真實放大的 Tick 價格
    std::uniform_int_distribution<int64_t> dist(100LL * PRICE_SCALE, 200LL * PRICE_SCALE);
    for (size_t i = 0; i < DATA_SIZE; ++i) {
        mockData[i] = dist(gen);
    }

    std::cout << "[Benchmark] Starting execution...\n";

// --- 測試 LinearBuffer O(N) 隔離區塊 ---
#ifdef TEST_LINEAR
    // 注意：請確保你的 LinearBuffer.h 內部也已將型態全面改為 int64_t
    LinearBuffer linearBuf(WINDOW_SIZE); 
    
    auto startLinear = std::chrono::high_resolution_clock::now();
    for (int64_t val : mockData) { // 改為 int64_t
        linearBuf.push(val);
        volatile int64_t avg = linearBuf.getAverage(); // 改為 int64_t
    }
    auto endLinear = std::chrono::high_resolution_clock::now();
    auto durationLinear = std::chrono::duration_cast<std::chrono::microseconds>(endLinear - startLinear).count();
    
    std::cout << "----------------------------------------\n";
    std::cout << "Window Size  : " << WINDOW_SIZE << "\n";
    std::cout << "Linear  O(N) : " << durationLinear << " us\n";
    std::cout << "----------------------------------------\n";
#endif

// --- 測試 CircularBuffer O(1) 隔離區塊 ---
#ifdef TEST_CIRCULAR
    CircularBuffer<int64_t> circularBuf(WINDOW_SIZE); // 實例化為 int64_t
    
    auto startCircular = std::chrono::high_resolution_clock::now();
    for (int64_t val : mockData) { // 改為 int64_t
        circularBuf.add(val); 
        volatile int64_t avg = circularBuf.getAverage(); // 改為 int64_t
    }
    auto endCircular = std::chrono::high_resolution_clock::now();
    auto durationCircular = std::chrono::duration_cast<std::chrono::microseconds>(endCircular - startCircular).count();
    
    std::cout << "----------------------------------------\n";
    std::cout << "Window Size  : " << WINDOW_SIZE << "\n";
    std::cout << "Circular O(1): " << durationCircular << " us\n";
    std::cout << "----------------------------------------\n";
#endif

    return 0;
}