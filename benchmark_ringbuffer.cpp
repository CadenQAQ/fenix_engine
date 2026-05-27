// main.cpp
// main.cpp
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <chrono>
#include "ring_buffer.h" // 包含你写的无锁队列和 MarketHeader
#include "include/fenix_algo.h"

using namespace std;
using namespace std::chrono;

const int MSG_COUNT = 5000000; // 500万次极速吞吐测试

// ==========================================
// 🐢 传统组：带锁的 std::queue
// ==========================================
queue<MarketHeader> mutex_queue;
mutex mtx;

void producer_mutex() {
    MarketHeader msg;
    for (int i = 1; i <= MSG_COUNT; ++i) {
        msg.seq_num = i;
        // 必须加锁才能推进去
        mtx.lock();
        mutex_queue.push(msg);
        mtx.unlock();
    }
}

void consumer_mutex() {
    int received = 0;
    MarketHeader msg;
    while (received < MSG_COUNT) {
        mtx.lock();
        if (!mutex_queue.empty()) {
            msg = mutex_queue.front();
            mutex_queue.pop();
            received++;
        }
        mtx.unlock();
    }
}

// ==========================================
// 🚀 极速组：你的 Fenix 无锁环形队列
// ==========================================
LockFreeRingBuffer<MarketHeader> lf_queue(1024);// 容量 1024 的环

void producer_lf() {
    MarketHeader msg;
    for (int i = 1; i <= MSG_COUNT; ++i) {
        msg.seq_num = i;
        // 疯狂轮询，直到推入成功。没有任何 OS 锁！
        while (!lf_queue.push(msg)) {
            // 队列满了就在 CPU 上空转等待 (Spin)
        }
    }
}

void consumer_lf() {
    int received = 0;
    MarketHeader msg;
    while (received < MSG_COUNT) {
        // 疯狂轮询，直到拉取成功。没有任何 OS 锁！
        if (lf_queue.pop(msg)) {
            received++;
        }
    }
}

// ==========================================
// ⏱️ 裁判系统
// ==========================================
int main() {
    cout << "=== Fenix Multi-Thread Benchmark (5,000,000 Messages) ===" << endl;

    // 1. 测试传统 Mutex
    cout << "\n[1] Running Traditional Mutex Queue..." << flush;
    auto start_mtx = high_resolution_clock::now();
    
    thread t1_mtx(producer_mutex);
    thread t2_mtx(consumer_mutex);
    t1_mtx.join();
    t2_mtx.join();
    
    auto end_mtx = high_resolution_clock::now();
    auto duration_mtx = duration_cast<milliseconds>(end_mtx - start_mtx).count();
    cout << " Done! Time: " << duration_mtx << " ms" << endl;

    // 2. 测试 Lock-Free Ring Buffer
    cout << "[2] Running Lock-Free Ring Buffer..." << flush;
    auto start_lf = high_resolution_clock::now();
    
    thread t1_lf(producer_lf);
    thread t2_lf(consumer_lf);
    t1_lf.join();
    t2_lf.join();
    
    auto end_lf = high_resolution_clock::now();
    auto duration_lf = duration_cast<milliseconds>(end_lf - start_lf).count();
    cout << " Done! Time: " << duration_lf << " ms" << endl;

    // 3. 战报总结
    cout << "\n📊 Performance Ratio: Lock-Free is " 
         << (double)duration_mtx / duration_lf << "x FASTER!" << endl;

    return 0;
}