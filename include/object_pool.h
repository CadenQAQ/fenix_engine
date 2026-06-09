// include/object_pool.h
#pragma once
#include <cstring>
#include <x86intrin.h>
#include "ring_buffer.h" // 引入你的无锁队列来作为底层的 Free-List

template <typename T, size_t Capacity>
class LockFreeObjectPool {
private:
    alignas(64) T memory[Capacity];
    LockFreeRingBuffer<size_t> free_indices{Capacity};

public:
    LockFreeObjectPool() {
        for (size_t i = 0; i < Capacity; ++i) {
            // 强行写入 0，触发缺页中断，提前占死物理内存
            std::memset(&memory[i], 0, sizeof(T));
            while (!free_indices.push(i)) {} 
        }
    }

    T* allocate() {
        size_t idx;
        if (free_indices.pop(idx)) {
            // Placement New：在预先分配好的物理内存上直接构造对象
            return new (&memory[idx]) T(); 
        }
        return nullptr; 
    }

    void deallocate(T* ptr) {
        if (!ptr) return;
        // 指针偏移计算，极速还原出索引
        size_t idx = ptr - memory; 
        while (!free_indices.push(idx)) { _mm_pause(); }
    }
};