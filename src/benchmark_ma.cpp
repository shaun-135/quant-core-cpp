#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include "../include/LinearBuffer.h"
// #include "../include/CircularBuffer.h" // 請確保引入已完成的 O(1) 實作

int main() {
    const size_t DATA_SIZE = 1000000;
    const size_t WINDOW_SIZE = 1000; // 放大視窗以突顯複雜度差異

    std::cout << "[Benchmark] Generating " << DATA_SIZE << " random data points...\n";
    std::vector<double> mockData(DATA_SIZE);
    
    // 鎖定亂數種子以確保每次 Benchmark 基準一致
    std::mt19937 gen(42); 
    std::uniform_real_distribution<double> dist(100.0, 200.0);
    for (size_t i = 0; i < DATA_SIZE; ++i) {
        mockData[i] = dist(gen);
    }

    LinearBuffer linearBuf(WINDOW_SIZE);
    // CircularBuffer circularBuf(WINDOW_SIZE); // 依你實際的 API 初始化

    std::cout << "[Benchmark] Starting execution...\n";

    // --- 測試 LinearBuffer O(N) ---
    auto startLinear = std::chrono::high_resolution_clock::now();
    for (double val : mockData) {
        linearBuf.push(val);
        volatile double avg = linearBuf.getAverage(); // volatile 防止編譯器優化掉未使用的回傳值
    }
    auto endLinear = std::chrono::high_resolution_clock::now();
    auto durationLinear = std::chrono::duration_cast<std::chrono::microseconds>(endLinear - startLinear).count();

    // --- 測試 CircularBuffer O(1) ---
    /*
    auto startCircular = std::chrono::high_resolution_clock::now();
    for (double val : mockData) {
        circularBuf.push(val);
        volatile double avg = circularBuf.getAverage();
    }
    auto endCircular = std::chrono::high_resolution_clock::now();
    auto durationCircular = std::chrono::duration_cast<std::chrono::microseconds>(endCircular - startCircular).count();
    */

    std::cout << "----------------------------------------\n";
    std::cout << "Window Size  : " << WINDOW_SIZE << "\n";
    std::cout << "Linear  O(N) : " << durationLinear << " us\n";
    // std::cout << "Circular O(1) : " << durationCircular << " us\n";
    std::cout << "----------------------------------------\n";

    return 0;
}