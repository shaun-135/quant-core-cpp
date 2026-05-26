#pragma once
#include <string>
#include <vector>
#include "StockData.h"

// 輔助函式：清理字串
std::string cleanStr(std::string str);

// 資料讀取模組
std::vector<StockData> loadCSV(const std::string& filename);