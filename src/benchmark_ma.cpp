#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <fstream>
#include <cstdlib> // 提供 system() 呼叫
#include "../include/LinearBuffer.h"
#include "../include/CircularBuffer.h"

int main() {
    const size_t DATA_SIZE = 1000000; 
    constexpr int64_t PRICE_SCALE = 100000000LL;

    std::cout << "[Benchmark] Generating " << DATA_SIZE << " random int64_t data points...\n";
    std::vector<int64_t> mockData(DATA_SIZE);
    std::mt19937 gen(42); 
    std::uniform_int_distribution<int64_t> dist(100LL * PRICE_SCALE, 200LL * PRICE_SCALE);
    for (size_t i = 0; i < DATA_SIZE; ++i) {
        mockData[i] = dist(gen);
    }

    // 開啟 CSV 檔案準備寫入結果
    std::ofstream csvFile("logs/benchmark.csv");
    csvFile << "window_size,linear_time_us,circular_time_us\n";

    std::cout << "[Benchmark] Starting Time Complexity Analysis...\n";

    // 測試不同的 Window Size，從 100 測到 5000，每次增加 500
    for (size_t window = 100; window <= 5000; window += 500) {
        
        // 1. 測試 O(N) 線性緩衝區
        LinearBuffer linearBuf(window); 
        auto startL = std::chrono::high_resolution_clock::now();
        for (int64_t val : mockData) {
            linearBuf.push(val);
            volatile int64_t avg = linearBuf.getAverage(); 
        }
        auto endL = std::chrono::high_resolution_clock::now();
        long long timeL = std::chrono::duration_cast<std::chrono::microseconds>(endL - startL).count();

        // 2. 測試 O(1) 環狀緩衝區
        CircularBuffer<int64_t> circularBuf(window); 
        auto startC = std::chrono::high_resolution_clock::now();
        for (int64_t val : mockData) {
            circularBuf.add(val); 
            volatile int64_t avg = circularBuf.getAverage(); 
        }
        auto endC = std::chrono::high_resolution_clock::now();
        long long timeC = std::chrono::duration_cast<std::chrono::microseconds>(endC - startC).count();

        // 輸出至終端機與 CSV
        std::cout << "Window: " << window << " | Linear O(N): " << timeL << " us | Circular O(1): " << timeC << " us\n";
        csvFile << window << "," << timeL << "," << timeC << "\n";
    }

    csvFile.close();
    std::cout << "[Benchmark] Data exported to logs/benchmark.csv\n";

    // C++ 呼叫 Python 腳本直接畫圖
    std::cout << "[Benchmark] Invoking Python for visualization...\n";
    std::system("python plot_benchmark.py");

    return 0;
}