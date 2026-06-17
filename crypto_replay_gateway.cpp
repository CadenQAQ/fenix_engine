#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <iomanip>
#include <random>
#include "include/ring_buffer.h"
#include "include/object_pool.h"

using namespace std;

// ==========================================
// 1. 业务定义：币安 L2 订单簿更新信封 (Tick)
// ==========================================
struct BinanceTick {
    uint64_t mac_timestamp; // 网卡硬件时间 / 生产时间
    double price;           // 价格 (如 65000.50)
    double quantity;        // 数量 (如 1.5 BTC)
    bool is_bid;            // true: 买盘 (Bid), false: 卖盘 (Ask)
};

const size_t QUEUE_CAPACITY = 4096; // Crypto 行情爆发时包量大，适当调大队列
LockFreeObjectPool<BinanceTick, QUEUE_CAPACITY> g_pool;
LockFreeRingBuffer<BinanceTick*> g_queue(QUEUE_CAPACITY);

const uint64_t TOTAL_TICKS = 5'000'000; // 回放 500 万条行情

// ==========================================
// 模拟组件：在内存中极速生成 500 万条逼真的 BTC 行情
// ==========================================
vector<BinanceTick> generate_mock_binance_data() {
    cout << "[环境准备] 正在生成 " << TOTAL_TICKS << " 条 BTC/USDT 模拟行情数据..." << endl;
    vector<BinanceTick> mock_data;
    mock_data.reserve(TOTAL_TICKS);

    mt19937 gen(42); 
    normal_distribution<> price_dist(65000.0, 10.0); // 均价65000，波动10
    uniform_real_distribution<> qty_dist(0.001, 2.5); // 单笔数量
    uniform_int_distribution<> side_dist(0, 1);

    for (uint64_t i = 0; i < TOTAL_TICKS; ++i) {
        BinanceTick tick;
        tick.price = price_dist(gen);
        tick.quantity = qty_dist(gen);
        tick.is_bid = side_dist(gen) == 1;
        mock_data.push_back(tick);
    }
    cout << "[环境准备] 行情数据生成完毕！" << endl;
    return mock_data;
}

// ==========================================
// Rx 线程：极速行情回放器 (模拟 WebSocket 接收回调)
// ==========================================
void binance_rx_thread(const vector<BinanceTick>& mock_data) {
    // 实际生产：pin_thread_to_core(2);
    
    for (const auto& raw_tick : mock_data) {
        BinanceTick* ev = g_pool.allocate();
        while (!ev) {
            _mm_pause(); // 发生背压，等待 Tx 线程消费
            ev = g_pool.allocate();
        }

        // 将外部数据拷贝进我们零拷贝流水线的信封中
        *ev = raw_tick; 
        
        // ⚡ 极其重要：打上进入内部系统的纳秒级时间戳
        ev->mac_timestamp = __rdtsc(); 

        while (!g_queue.push(ev)) {
            _mm_pause(); 
        }
    }
}

// ==========================================
// Tx 线程：极速订单簿 (Orderbook) 重建与信号计算
// ==========================================
void orderbook_tx_thread() {
    // 实际生产：pin_thread_to_core(4);
    
    uint64_t received_count = 0;
    BinanceTick* ev;

    // 简易的 BBO (Best Bid / Best Offer) 追踪器
    double current_best_bid = 0.0;
    double current_best_ask = 99999999.0;
    
    uint64_t total_latency_cycles = 0;

    while (received_count < TOTAL_TICKS) {
        if (g_queue.pop(ev)) {
            // 1. 计算行情穿透延迟
            uint64_t latency = __rdtsc() - ev->mac_timestamp;
            total_latency_cycles += latency;

            // 2. ⚡ 核心业务逻辑：更新订单簿 BBO
            if (ev->is_bid) {
                if (ev->price > current_best_bid) {
                    current_best_bid = ev->price; // 买盘价格创出新高
                }
            } else {
                if (ev->price < current_best_ask) {
                    current_best_ask = ev->price; // 卖盘价格创出新低
                }
            }

            // 3. (可选) 极速策略信号：如果出现了套利空间 (Bid > Ask)，触发交易！
            if (current_best_bid >= current_best_ask) {
                // 触发发单逻辑... (此处略过，防止输出太多)
            }

            // 4. 用完立刻归还内存！
            g_pool.deallocate(ev);
            received_count++;
            
        } else {
            _mm_pause();
        }
    }

    cout << "\n[Engine] 盘口维护结束。最终最优盘口: [Bid: " 
         << current_best_bid << " | Ask: " << current_best_ask << "]" << endl;
    cout << "[Engine] 平均跨核与订单簿更新延迟: " 
         << (total_latency_cycles / TOTAL_TICKS) << " CPU 周期" << endl;
}

int main() {
    cout << "=========================================" << endl;
    cout << "🚀 Fenix Engine - Crypto 订单簿极速回放测试" << endl;
    cout << "=========================================" << endl;

    // 1. 预先生成回放数据
    vector<BinanceTick> mock_data = generate_mock_binance_data();

    auto start_time = chrono::high_resolution_clock::now();

    // 2. 点火！发射 Rx 和 Tx 线程
    thread rx(binance_rx_thread, std::ref(mock_data));
    thread tx(orderbook_tx_thread);

    rx.join();
    tx.join();

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end_time - start_time;
    
    cout << "\n📊 [实战业务压测报告]" << endl;
    cout << fixed << setprecision(2);
    cout << "处理总量: " << TOTAL_TICKS << " 条 L2 Tick 数据" << endl;
    cout << "总耗时:   " << elapsed.count() << " 秒" << endl;
    cout << "吞吐能力: " << (TOTAL_TICKS / elapsed.count()) / 1'000'000.0 << " Million Ticks/sec" << endl;
    cout << "=========================================" << endl;

    return 0;
}

/*if (ev->is_bid) {
    if (ev->price > current_best_bid) current_best_bid = ev->price;
} else { ... }
这不仅是一次内存读取，而是**实打实的量化业务计算**。系统在每一条消息流转时，不仅完成了借内存、还内存的基建动作，
还在纳秒间判断了当前市场的**最强买力（Best Bid）**和**最强卖力（Best Ask）**！
顶级 Crypto 做市商（Market Maker）收到 WebSocket 推送后的第一步核心逻辑就是这个。

### 🚀 部署实战

就像我们之前编译压测代码一样，你可以在你的 `hkoffice01` 机器上直接新建这个文件并编译：

```bash
# 1. 保存为 crypto_replay_gateway.cpp
# 2. 编译它
g++ -O3 -pthread crypto_replay_gateway.cpp -o build/crypto_replay
# 3. 运行它
./build/crypto_replay

当你跑完这个版本，观察一下吞吐量是否依然能维持在百万级别，以及引入了 `double` 类型的盘口更新逻辑后，CPU 周期的消耗有什么变化？跑出来结果发给我，我们接下来就可以准备接入真实的 Binance WebSocket 网络了！*/