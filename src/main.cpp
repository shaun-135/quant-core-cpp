#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm> // 用於移除字元
#include "StockData.h"

// 輔助函式：清理字串中的引號與逗號
std::string cleanStr(std::string str) {
    str.erase(std::remove(str.begin(), str.end(), '\"'), str.end());
    str.erase(std::remove(str.begin(), str.end(), ','), str.end());
    return str;
}

int main() {
    std::ifstream file("../data/2330.TW.csv");
    if (!file.is_open()) {
        std::cerr << "Fatal Error: Cannot open data/2330.TW.csv" << std::endl;
        return 1;
    }

    std::string line;
    std::getline(file, line); // Skip Header

    std::vector<StockData> prices;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string date, priceStr;

        // 處理 Date: 讀取第一個引號內的內容
        std::getline(ss, date, ','); 

        std::string fullPart;
        std::vector<std::string> fields;
        std::string segment;
        
        date = cleanStr(date);
        
        // 讀取剩下的部分直到下一個逗號，但要考慮引號內的逗號
        std::getline(ss, priceStr, ','); 

        // 如果 priceStr 開頭有引號但結尾沒有，代表它被中間的逗號切斷了
        if (!priceStr.empty() && priceStr.front() == '\"' && priceStr.back() != '\"') {
            std::string extra;
            std::getline(ss, extra, ',');
            priceStr += extra; // 把 250.00" 接回來
        }

        priceStr = cleanStr(priceStr);

        try {
            StockData d;
            d.date = date;
            d.close = std::stod(priceStr); 
        } catch (...) {
            continue; 
        }
    }

    std::cout << "--- QuantCore Prototype Success ---" << std::endl;
    std::cout << "Total records loaded: " << prices.size() << std::endl;
    if (!prices.empty()) {
        std::cout << "Sample -> Date: " << prices.front().date << " | Price: " << prices.front().close << std::endl;
    }

    return 0;
}