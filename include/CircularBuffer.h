#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <vector>
#include <numeric>

template <typename T>
class CircularBuffer {
private:
    std::vector<T> buffer;
    size_t head = 0;
    size_t capacity;
    bool full = false;

public:
    CircularBuffer(size_t size) : buffer(size), capacity(size) {}

    // 加入新數據
    void add(T item) {
        buffer[head] = item;
        head = (head + 1) % capacity;
        if (head == 0) full = true;
    }

    // 計算平均值 (用於均線)
    double getAverage() const {
        size_t current_size = full ? capacity : head;
        if (current_size == 0) return 0.0;
        T sum = std::accumulate(buffer.begin(), buffer.begin() + current_size, 0.0);
        return static_cast<double>(sum) / current_size;
    }
};

#endif