#pragma once 
#include <vector>
#include<string>
#include <unordered_map>

using namespace std;

#include <cstdint> // 引入标准定长整数类型 (uint16_t, uint32_t 等)

// 告诉编译器：接下来的结构体，按 1 字节绝对紧凑对齐，不准加 Padding！
#pragma pack(push, 1)

// 交易所标准报文头 (Market Data Header)
struct MarketHeader {
    uint16_t msg_length; // 报文总长度 (2 字节)
    uint16_t msg_type;   // 报文类型 (2 字节，比如 1=订单，2=成交)
    uint32_t seq_num;    // 序列号 (4 字节，防丢包)
    uint64_t timestamp;  // 纳秒级时间戳 (8 字节)
};

// 恢复编译器默认的对齐方式
#pragma pack(pop)

// 链表节点定义 (POD 结构体)
struct ListNode {
    int val;
    ListNode* next;
    // 构造函数，方便快速创建节点
    ListNode(int x) : val(x), next(nullptr) {} 
};

// 升级版：双向链表节点 (多了 prev 光纤)
struct DLinkedNode {
    int key;
    int value;
    DLinkedNode* prev;
    DLinkedNode* next;
    DLinkedNode() : key(0), value(0), prev(nullptr), next(nullptr) {}
    DLinkedNode(int _key, int _value) : key(_key), value(_value), prev(nullptr), next(nullptr) {}
};

// 极速 LRU 缓存组件
/*
capacity 是你向操作系统申请的“最大连接数”（比如 10000）。在网关启动时设定，一旦定死，绝不能超发，否则会引发内存溢出（OOM）。

size 是当前网关里实际存活的活跃连接数。

动作： 每次新来一个 IP，size++。当 size > capacity 时，立刻触发网关的“无情裁员”机制，把最老的连接踢掉，然后 size--。

unordered_map<int, DLinkedNode*> cache; (上帝视角的雷达探测器)设计目的： 实现绝对的 $O(1)$ 极速寻址。物理意义： 链表最大的缺点是“找人太慢”，要从头遍历。
这个哈希表就像是机房里的“全局路由表”。int key 是客户的 ID（比如 IP 地址）。DLinkedNode* 是这个客户在双向链表里对应的绝对内存物理地址。
动作： 当客户 888 再次发来数据包时，网关不需要去链表里苦苦寻找。
直接查雷达 cache[888]，瞬间拿到这个节点在内存里的指针，然后对它进行“提拔”操作。
*/
class LRUCache {
private:
    std::unordered_map<int, DLinkedNode*> cache; // 上帝视角的雷达
    DLinkedNode* head; // 虚拟头节点（哨兵）
    DLinkedNode* tail; // 虚拟尾节点（哨兵）
    int size;
    int capacity;

    // 两个内部核心微操：拔光纤、插光纤
    void addNode(DLinkedNode* node);
    void removeNode(DLinkedNode* node);
    void moveToHead(DLinkedNode* node);
    DLinkedNode* removeTail();

public:
    LRUCache(int capacity);
    ~LRUCache(); // Infra 洁癖：记得释放内存
    int get(int key);
    void put(int key, int value);
};

// 算法声明
ListNode* reverse_list(ListNode* head);

int binary_search(vector<int>& nums, int target);
int find_first(vector<int>& nums, int target);
int find_last(vector<int>& nums, int target);
bool is_anagram(const std::string& s1, const std::string& s2);
int length_of_longest_substring(const std::string& s);
int max_sum_given_fixed_length(const int* arr, int size, int k);
bool evaluate_packet_rules(const vector<string>& instructions);
void pending_list(const vector<int>& num, vector<int>& result);
int subarray_sum(const vector<int>& nums, int target_sub_sum);
int search_insert(vector<int> num, int target);
vector<vector<int>> threeSum(vector<int>& nums);
bool has_cycle(ListNode* head);

void parse_market_packet(const uint8_t* raw_buffer);

