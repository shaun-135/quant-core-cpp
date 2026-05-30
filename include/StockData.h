#ifndef STOCKDATA_H
#define STOCKDATA_H

#include <cstdint>

struct StockData {
    uint64_t timestamp; // 幣安毫秒時間戳記 (例如: 1672531199000)
    int64_t price;       // 交易價格 (即原 close)
    // 若後續需要擴充，可加入 volume 等欄位
};

struct BarData {
    uint64_t timestamp; // 該 K 線的起始時間戳
    int64_t open;       // 開盤價
    int64_t high;       // 最高價
    int64_t low;        // 最低價
    int64_t close;      // 收盤價
};

#endif // STOCKDATA_H