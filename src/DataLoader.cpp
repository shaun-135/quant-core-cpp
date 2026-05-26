#include "../include/DataLoader.h"
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>

std::vector<StockData> loadCSV(const std::string& filename) {
    std::vector<StockData> dataList;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return dataList;
    }

    std::string line;
    dataList.reserve(1000000); 

    int lineNumber = 0;
    while (std::getline(file, line)) {
        lineNumber++;
        if (line.empty()) continue;

        // 粗略過濾 CSV 標頭列：若該行包含英文字母則跳過 (假設第一行是標頭)
        if (lineNumber == 1 && (line.find("time") != std::string::npos || line.find("price") != std::string::npos)) {
            continue; 
        }

        size_t start = 0;
        size_t end = line.find(',');
        int columnIndex = 0;

        StockData data{};
        bool hasPrice = false;
        bool hasTimestamp = false;

        try {
            while (end != std::string::npos) {
                std::string token = line.substr(start, end - start);
                
                if (columnIndex == 1) { // Price 欄位
                    data.price = std::stod(token);
                    hasPrice = true;
                } else if (columnIndex == 4) { // Trades 資料的 Time 欄位索引為 4
                    data.timestamp = std::stoull(token);
                    hasTimestamp = true;
                }

                start = end + 1;
                end = line.find(',', start);
                columnIndex++;
            }

            // 處理最後一個欄位 (如果需要的欄位剛好在末尾)
            if (columnIndex == 4) {
                data.timestamp = std::stoull(line.substr(start));
                hasTimestamp = true;
            }

            if (hasPrice && hasTimestamp) {
                dataList.push_back(data);
            }
        } 
        catch (const std::exception& e) {
            // 精準印出錯誤發生位置與內容，放棄該行資料但確保程式繼續執行
            std::cerr << "[Warning] Parsing error at line " << lineNumber 
                      << ": " << e.what() << " | Raw line: " << line << std::endl;
        }
    }

    file.close();
    return dataList;
}