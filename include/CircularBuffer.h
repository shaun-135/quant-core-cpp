#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <vector>

template <typename T>
class CircularBuffer {
private:
    std::vector<T> buffer;
    size_t head = 0;
    size_t capacity;
    bool full = false;
    T running_sum = 0.0; // O(1) 核心：滾動總和

public:
    CircularBuffer(size_t size) : buffer(size), capacity(size) {}

    bool isFull() const { return full; }

    void add(T item) {
        if (full) {
            running_sum -= buffer[head]; // 扣除即將被覆蓋的舊資料
        }

        running_sum += item;
        buffer[head] = item;

        head = (head + 1) % capacity;
        if (!full && head == 0) {
            full = true;
        }
    }

    int64_t getAverage() const {
        size_t current_size = full ? capacity : head;
        if (current_size == 0) return 0;
        return static_cast<int64_t>(running_sum) / current_size; // O(1) 直接計算
    }
};

#endif

