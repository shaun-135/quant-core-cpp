#ifndef STOCKDATA_H
#define STOCKDATA_H

#include <cstdint>

struct StockData {
    uint64_t timestamp; // 幣安毫秒時間戳記 (例如: 1672531199000)
    double price;       // 交易價格 (即原 close)
    // 若後續需要擴充，可加入 volume 等欄位
};

#endif // STOCKDATA_H