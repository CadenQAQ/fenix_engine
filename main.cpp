// main.cpp
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <x86intrin.h> 
#include <thread>
#include <pthread.h>   
#include "include/ring_buffer.h" 

using namespace std;

LockFreeRingBuffer g_ring_buffer(1024);

void pin_thread_to_core(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

void network_rx_thread() {
    pin_thread_to_core(2);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));

    char buffer[1024];
    while (true) {
        int bytes = recvfrom(sock, buffer, sizeof(buffer), 0, nullptr, nullptr);
        if (bytes > 0) {
            MarketHeader* tick = (MarketHeader*)buffer;
            while (!g_ring_buffer.push(*tick)) { _mm_pause(); }
        }
    }
}

void engine_tx_thread() {
    pin_thread_to_core(4);
    MarketHeader current_tick;
    while (true) {
        if (g_ring_buffer.pop(current_tick)) {
            cout << "Boom! [Engine] 成功从 RingBuffer 拿到数据！当前 CPU 周期: " << __rdtsc() << endl;
        } else {
            _mm_pause(); 
        }
    }
}

int main() {
    cout << "=== Fenix Gateway Engine v2.0 (Dual-Thread Lock-Free) ===" << endl;
    std::thread rx_th(network_rx_thread);
    std::thread tx_th(engine_tx_thread);
    rx_th.join();
    tx_th.join();
    return 0;
}
