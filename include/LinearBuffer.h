#ifndef LINEAR_BUFFER_H
#define LINEAR_BUFFER_H

#include <vector>
#include <numeric>
#include <cstddef>

class LinearBuffer {
private:
    std::vector<double> buffer;
    size_t capacity;

public:
    explicit LinearBuffer(size_t cap) : capacity(cap) {
        // 預留空間避免 std::vector 擴容 (Reallocation) 帶來的額外複雜度干擾
        buffer.reserve(capacity + 1); 
    }

    void push(double value) {
        buffer.push_back(value);
        if (buffer.size() > capacity) {
            // 觸發 O(N) 的記憶體搬移
            buffer.erase(buffer.begin());
        }
    }

    double getAverage() const {
        if (buffer.empty()) return 0.0;
        // 觸發 O(N) 的線性走訪加總
        double sum = std::accumulate(buffer.begin(), buffer.end(), 0.0);
        return sum / buffer.size();
    }
};

#endif // LINEAR_BUFFER_H