#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <iomanip>
#include "include/ring_buffer.h"
#include "include/object_pool.h"

using namespace std;

// 1. 定义极其轻量的测试信封
struct TestEvent {
    uint64_t timestamp;
    uint64_t payload;
};

// 2. 组装终极架构：1024 物理容量的内存池 + 传递指针的无锁队列
const size_t QUEUE_CAPACITY = 1024;
LockFreeObjectPool<TestEvent, QUEUE_CAPACITY> g_pool;
LockFreeRingBuffer<TestEvent*> g_queue(QUEUE_CAPACITY);

// 3. 压测目标：狂灌 1000 万条数据
const uint64_t TOTAL_MESSAGES = 10'000'000;

// ==========================================
// 生产者：模拟极端行情海啸 (狂暴 Rx 线程)
// ==========================================
void producer_thread() {
    // 实际生产中这里会绑核 pin_thread_to_core(2);
    
    for (uint64_t i = 0; i < TOTAL_MESSAGES; ++i) {
        // 1. 极速借内存
        TestEvent* ev = g_pool.allocate();
        
        // 如果池子空了（背压极其严重，连内存都没得借了）
        while (!ev) {
            _mm_pause(); // 等待消费者归还内存
            ev = g_pool.allocate();
        }

        // 2. 写入数据
        ev->timestamp = __rdtsc();
        ev->payload = i;

        // 3. 推入队列（遇满则疯狂自旋，模拟背压）
        while (!g_queue.push(ev)) {
            _mm_pause(); 
        }
    }
}

// ==========================================
// 消费者：模拟极速撮合引擎 (狂暴 Tx 线程)
// ==========================================
void consumer_thread() {
    // 实际生产中这里会绑核 pin_thread_to_core(4);
    
    uint64_t received_count = 0;
    TestEvent* ev;

    while (received_count < TOTAL_MESSAGES) {
        // 1. 尝试从队列拉取指针
        if (g_queue.pop(ev)) {
            // (模拟处理业务逻辑) volatile 阻止编译器把这段代码优化掉
            volatile uint64_t data = ev->payload; 
            
            // 2. 极速归还内存给池子！
            g_pool.deallocate(ev);
            
            received_count++;
        } else {
            // 队列为空，自旋等待
            _mm_pause();
        }
    }
}

int main() {
    cout << "=========================================" << endl;
    cout << "🚀 Fenix Engine 极限吞吐量压测 (C++11)" << endl;
    cout << "=========================================" << endl;
    cout << "架构配置: MemoryPool + LockFreeRingBuffer (Zero-Copy)" << endl;
    cout << "测试数量: " << TOTAL_MESSAGES << " 条跨核消息" << endl;
    cout << "队列容量: " << QUEUE_CAPACITY << " 槽位 (强制背压测试)" << endl;
    cout << "压测进行中..." << endl;

    // 记录开始时间 (使用高精度系统时钟)
    auto start_time = chrono::high_resolution_clock::now();

    // 发射线程！
    thread producer(producer_thread);
    thread consumer(consumer_thread);

    producer.join();
    consumer.join();

    // 记录结束时间
    auto end_time = chrono::high_resolution_clock::now();
    
    // 计算耗时
    chrono::duration<double> elapsed = end_time - start_time;
    double seconds = elapsed.count();
    
    // 计算吞吐量 (Ops / Sec)
    double ops_per_sec = TOTAL_MESSAGES / seconds;
    // 计算平均跨核延迟 (纳秒)
    double avg_latency_ns = (seconds * 1'000'000'000) / TOTAL_MESSAGES;

    cout << "\n📊 [压测报告]" << endl;
    cout << fixed << setprecision(2);
    cout << "总耗时:     " << seconds << " 秒" << endl;
    cout << "极限吞吐量: " << ops_per_sec / 1'000'000.0 << " Million Ops/sec (百万次/秒)" << endl;
    cout << "单次平均耗时: " << avg_latency_ns << " ns (纳秒) - 包含借内存、跨核传指针、还内存的完整闭环" << endl;
    cout << "=========================================" << endl;

    return 0;
}