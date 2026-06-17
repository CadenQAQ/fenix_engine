#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <functional> // ⚡ 修复缺少 bind 占位符的问题

// 引入你手搓的核心基建
#include "include/ring_buffer.h"
#include "include/object_pool.h"

// 引入第三方网络与 JSON 库
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// WebSocket 客户端类型定义 (由于币安是 wss，必须开启 TLS 支持)
typedef websocketpp::client<websocketpp::config::asio_tls_client> ws_client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

// ==========================================
// 1. 业务定义：原生网络信封 (Raw Network Event)
// 黄金法则：Rx 线程只传原生字节，绝不在收包时做解析！
// ==========================================
struct RawNetworkEvent {
    uint64_t mac_timestamp;   // 收到网络包瞬间的硬件/CPU时钟
    size_t payload_length;    // 数据长度
    char payload[1024];       // 预分配的连续内存，防止动态 string 分配
};

const size_t QUEUE_CAPACITY = 1024; 
LockFreeObjectPool<RawNetworkEvent, QUEUE_CAPACITY> g_pool;
LockFreeRingBuffer<RawNetworkEvent*> g_queue(QUEUE_CAPACITY);

// 全局 WebSocket 客户端
ws_client c;

// ==========================================
// Rx 线程回调：极速网络收包 (WebSocket On Message)
// ==========================================
void on_message(websocketpp::connection_hdl hdl, ws_client::message_ptr msg) {
    // ⚡ 1. 收到包的第一瞬间，立刻打下时间戳！
    uint64_t hw_tsc = __rdtsc(); 
    
    // 2. 极速借内存
    RawNetworkEvent* ev = g_pool.allocate();
    if (!ev) {
        // 如果池子空了（背压），实盘中通常直接丢弃老行情或覆盖，这里简单打印报警
        cerr << "[Rx 报警] 内存池枯竭，发生严重背压！丢包!" << endl;
        return;
    }

    // 3. 将原生网络数据拷贝进我们的零拷贝信封
    const string& payload_str = msg->get_payload();
    ev->mac_timestamp = hw_tsc;
    ev->payload_length = min(payload_str.length(), (size_t)1023); // 防止溢出
    memcpy(ev->payload, payload_str.c_str(), ev->payload_length);
    ev->payload[ev->payload_length] = '\0'; // 补齐字符串结束符

    // 4. 将轻量级指针推入无锁队列！
    while (!g_queue.push(ev)) {
        _mm_pause(); // 发生微观拥堵时自旋
    }
}

// TLS 初始化配置 (连接 Binance wss 必须)
context_ptr on_tls_init(websocketpp::connection_hdl) {
    context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
    ctx->set_options(boost::asio::ssl::context::default_workarounds |
                     boost::asio::ssl::context::no_sslv2 | boost::asio::ssl::context::no_sslv3 |
                     boost::asio::ssl::context::single_dh_use);
    return ctx;
}

// ==========================================
// Tx 线程：实盘解析与订单簿维护
// ==========================================
void orderbook_tx_thread() {
    // 实际生产中这里会绑核 pin_thread_to_core(4);
    RawNetworkEvent* ev;
    
    cout << "[Engine] Tx 业务线程启动，等待 Binance 行情穿透..." << endl;

    while (true) {
        if (g_queue.pop(ev)) {
            // 1. 提取时间戳
            uint64_t rx_tsc = ev->mac_timestamp;
            
            try {
                // 2. ⚡ 解析 JSON (这在 HFT 里是极重度的操作)
                string raw_str(ev->payload, ev->payload_length);
                json j = json::parse(raw_str);

                // 解析币安 bookTicker 格式
                // 示例: {"u":400900217,"s":"BTCUSDT","b":"65000.50","B":"1.5","a":"65001.00","A":"2.0"}
                if (j.contains("b") && j.contains("a")) {
                    // JSON 里出来的是字符串，需要转成 double
                    double best_bid = stod(j["b"].get<string>()); 
                    double best_ask = stod(j["a"].get<string>());
                    
                    // 3. 计算端到端穿透延迟 (包含 JSON 解析的耗时！)
                    uint64_t tx_tsc = __rdtsc();
                    uint64_t latency_cycles = tx_tsc - rx_tsc;

                    cout << "\r[实盘 BBO] BTCUSDT | 买一: " << fixed << setprecision(2) << best_bid 
                         << " 卖一: " << best_ask 
                         << " | 内部穿透延迟: " << latency_cycles << " CPU 周期" << flush;
                }
            } catch (const exception& e) {
                // 容错处理
            }

            // 4. 用完立刻归还物理内存！
            g_pool.deallocate(ev);
            
        } else {
            _mm_pause(); // 队列为空，自旋等包
        }
    }
}

int main() {
    cout << "=========================================" << endl;
    cout << "🚀 Fenix Engine - Binance WebSocket 直连实盘" << endl;
    cout << "=========================================" << endl;

    // 1. 启动 Tx 业务线程
    thread tx(orderbook_tx_thread);

    // 2. 配置 WebSocket 客户端 (模拟 Rx 线程的底层设施)
    c.set_access_channels(websocketpp::log::alevel::none); // 关闭冗余网络日志
    c.set_error_channels(websocketpp::log::elevel::none);
    
    c.init_asio();
    // ⚡ 修复点：使用标准的 std::placeholders 命名空间
    c.set_tls_init_handler(bind(&on_tls_init, std::placeholders::_1));
    c.set_message_handler(bind(&on_message, std::placeholders::_1, std::placeholders::_2));

    websocketpp::lib::error_code ec;
    // 订阅 Binance BTC/USDT 的最优买卖价 (BookTicker) 流，毫秒级推送
    ws_client::connection_ptr con = c.get_connection("wss://stream.binance.com:9443/ws/btcusdt@bookTicker", ec);
    
    if (ec) {
        cout << "[网络错误] 连接失败: " << ec.message() << endl;
        return 1;
    }

    c.connect(con);
    
    cout << "[Network] 正在连接币安 (Binance) WebSocket 服务器..." << endl;
    
    // 启动 ASIO 网络事件循环 (这会阻塞主线程，相当于 Rx 线程的发动机)
    c.run(); 

    tx.join();
    return 0;
}
