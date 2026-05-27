// include/ring_buffer.h
#pragma once
#include <atomic>
#include <vector>
#include <cstdint>
#include "fenix_algo.h" // 引入我们之前定义的 MarketHeader

// 单生产者单消费者无锁环形队列 (SPSC Lock-free Ring Buffer)
class LockFreeRingBuffer {
private:
    std::vector<MarketHeader> buffer;
    const size_t capacity;
    const size_t mask;

    // ⚡ 核心魔法：原子游标，强制 CPU 保证可见性，但不加系统锁
    // alignas(64) 是为了防止伪共享(False Sharing)，确保两个游标不在同一个 CPU 缓存行里打架
    alignas(64) std::atomic<size_t> write_idx;
    alignas(64) std::atomic<size_t> read_idx;

public:
    // capacity 必须是 2 的幂次方，比如 1024
    LockFreeRingBuffer(size_t cap) 
        : capacity(cap), mask(cap - 1), write_idx(0), read_idx(0) {
        buffer.resize(capacity);
    }

    // Core 0 (网卡解析线程) 调用：推入订单
    bool push(const MarketHeader& item) {
        size_t current_write = write_idx.load(std::memory_order_relaxed);
        size_t next_write = current_write + 1;

        // 如果写指针追上了读指针，说明环形队列满了！
        if ((next_write - read_idx.load(std::memory_order_acquire)) > capacity) {
            return false; // 队列满，丢弃或重试
        }

        // 把数据写入连续数组 (使用位运算实现极速环形折返)
        buffer[current_write & mask] = item;

        // 🛡️ 内存屏障：确保数据写完之后，再更新 write_idx
        write_idx.store(next_write, std::memory_order_release);
        return true;
    }

    // Core 1 (撮合引擎线程) 调用：拉取订单
    bool pop(MarketHeader& item) {
        size_t current_read = read_idx.load(std::memory_order_relaxed);

        // 如果读指针等于写指针，说明队列空了，没活干
        if (current_read == write_idx.load(std::memory_order_acquire)) {
            return false; // 队列空
        }

        // 把数据读出来
        item = buffer[current_read & mask];

        // 🛡️ 内存屏障：确保数据读完之后，再更新 read_idx，把坑位还给生产者
        read_idx.store(current_read + 1, std::memory_order_release);
        return true;
    }
};