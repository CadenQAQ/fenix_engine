#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <x86intrin.h>
#include <thread>
#include <pthread.h>
#include <linux/net_tstamp.h> 
#include "include/ring_buffer.h" 
#include "include/fenix_algo.h"

using namespace std;

// ⚡ 这里的队列明确声明：我只接收信封 (InternalEvent)
LockFreeRingBuffer<InternalEvent> g_ring_buffer(1024);

void pin_thread_to_core(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

// ==========================================
// 线程 1：极速收包 + 硬件时间戳解析 (Core 2)
// ==========================================
void network_rx_thread() {
    pin_thread_to_core(2);
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));

    int flags = SOF_TIMESTAMPING_RX_HARDWARE | SOF_TIMESTAMPING_RAW_HARDWARE;
    if (setsockopt(sock, SOL_SOCKET, SO_TIMESTAMPING, &flags, sizeof(flags)) < 0) {
        cerr << "警告: 无法开启硬件时间戳!" << endl;
    }
    cout << "[Rx Thread] Pinned to Core 2. Hardware Timestamping READY." << endl;

    char buffer[1024];
    char control_buf[1024]; 
    struct iovec iov = {buffer, sizeof(buffer)};
    struct msghdr msg = {};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    while (true) {
        msg.msg_control = control_buf;
        msg.msg_controllen = sizeof(control_buf);

        int bytes = recvmsg(sock, &msg, 0);
        
        if (bytes > 0) {
            uint64_t hw_ns = 0;
            
            for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
                if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SO_TIMESTAMPING) {
                    struct timespec *ts = (struct timespec *)CMSG_DATA(cmsg);
                    hw_ns = (uint64_t)ts[2].tv_sec * 1000000000ULL + ts[2].tv_nsec; 
                    break;
                }
            }

            if (hw_ns == 0) hw_ns = __rdtsc(); // 降级保护

            // 1. 强转出交易所的原始数据
            MarketHeader* hdr = (MarketHeader*)buffer;
            
            // 2. 组装信封 (InternalEvent)
            InternalEvent event;
            event.mac_timestamp = hw_ns;
            event.market_data = *hdr;

            // 3. 将组装好的信封推入队列！(这里解决了你的 push 报错)
            while (!g_ring_buffer.push(event)) { _mm_pause(); }
        }
    }
}

// ==========================================
// 线程 2：极速撮合/业务线程 (Core 4)
// ==========================================
void engine_tx_thread() {
    pin_thread_to_core(4);
    InternalEvent current_event; 
    
    while (true) {
        if (g_ring_buffer.pop(current_event)) {
            uint64_t consume_tsc = __rdtsc(); 
            
            // 因为降级了，这里的 mac_timestamp 其实是 Rx 线程打的 CPU 周期
            uint64_t rx_tsc = current_event.mac_timestamp;
            
            // ⚡ 算出纯粹的跨核流水线延迟！
            uint64_t latency_cycles = consume_tsc - rx_tsc;
            
            cout << "🚀 [Engine] Seq: " << current_event.market_data.seq_num 
                 << " | Rx周期: " << rx_tsc 
                 << " | 跨核延迟: " << latency_cycles << " 个 CPU 周期" << endl;
        } else {
            _mm_pause(); 
        }
    }
}
int main() {
    cout << "=== Fenix Gateway Engine v3.0 (PHC Hardware Timestamping) ===" << endl;
    std::thread rx_th(network_rx_thread);
    std::thread tx_th(engine_tx_thread);
    rx_th.join();
    tx_th.join();
    return 0;
}