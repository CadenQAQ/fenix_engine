// include/ring_buffer.h
#pragma once
#include <atomic>
#include <vector>
#include <cstdint>

// 升级为泛型模板，完美支持 InternalEvent 或任何其他事件结构
template <typename T>
class LockFreeRingBuffer {
private:
    std::vector<T> buffer;
    const size_t capacity;
    const size_t mask;

    // ⚡ 核心魔法：保持原有的 64 字节缓存行对齐，确保读写游标在不同的 L1/L2 Cache Line 中，彻底杜绝伪共享
    alignas(64) std::atomic<size_t> write_idx;
    alignas(64) std::atomic<size_t> read_idx;

public:
    // capacity 必须是 2 的幂次方，比如 1024
    LockFreeRingBuffer(size_t cap) 
        : capacity(cap), mask(cap - 1), write_idx(0), read_idx(0) {
        buffer.resize(capacity);
    }

    // Core 2 (网卡收包线程) 调用：推入信封事件
    bool push(const T& item) {
        size_t current_write = write_idx.load(std::memory_order_relaxed);
        size_t next_write = current_write + 1;

        // 如果写指针追上了读指针，说明环形队列满了
        if ((next_write - read_idx.load(std::memory_order_acquire)) > capacity) {
            return false; // 队列满，触发背压（Backpressure）
        }

        // 把数据写入连续数组 (使用位运算实现极速环形折返)
        buffer[current_write & mask] = item;

        // 🛡️ 内存屏障：确保数据完全写入 buffer 后，再对外可见地更新 write_idx
        write_idx.store(next_write, std::memory_order_release);
        return true;
    }

    // Core 4 (撮合引擎线程) 调用：拉取信封事件
    bool pop(T& item) {
        size_t current_read = read_idx.load(std::memory_order_relaxed);

        // 如果读指针等于写指针，说明队列空了，没活干
        if (current_read == write_idx.load(std::memory_order_acquire)) {
            return false; // 队列空
        }

        // 把数据读出来
        item = buffer[current_read & mask];

        // 🛡️ 内存屏障：确保数据完全读出到局部变量后，再更新 read_idx，把坑位还给生产者
        read_idx.store(current_read + 1, std::memory_order_release);
        return true;
    }
};