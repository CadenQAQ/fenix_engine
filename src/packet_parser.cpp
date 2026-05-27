#include <iostream>
#include <arpa/inet.h> // 引入网络字节序转换函数 (ntohs, ntohl)
#include "../include/fenix_algo.h"

using namespace std;

void parse_market_packet(const uint8_t* raw_buffer) {
    const MarketHeader* header = reinterpret_cast<const MarketHeader*>(raw_buffer);

    // 🛡️ 硬件级装甲：把互联网的大端序，翻转回我们 x86 CPU 的小端序
    uint16_t real_length = ntohs(header->msg_length);
    uint16_t real_type   = ntohs(header->msg_type);
    uint32_t real_seq    = ntohl(header->seq_num);
    
    // 注意：网络标准库里没有 64 位的翻转，但在 Linux 下可以用 be64toh，
    // 这里为了通用性，我们暂时先打印原始的 timestamp，或者如果你引入了 <endian.h> 就可以直接用 be64toh(header->timestamp)
    uint64_t raw_ts = header->timestamp; 

    cout << "[Network RX] Packet Parsed safely!" << endl;
    cout << "  |- Real Length : " << real_length << " bytes" << endl;
    cout << "  |- Real Type   : " << real_type << endl;
    cout << "  |- Real Seq    : " << real_seq << endl;
}