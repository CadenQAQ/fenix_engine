#include "../include/fenix_algo.h"

// 1. 系统开机，分配哨兵机柜
LRUCache::LRUCache(int capacity) {
    this->capacity = capacity;
    this->size = 0;
    head = new DLinkedNode(-1, -1);
    tail = new DLinkedNode(-1, -1);
    head->next = tail;
    tail->prev = head;
}

// 2. 【Infra 洁癖】系统关机，必须释放所有内存！
LRUCache::~LRUCache() {
    DLinkedNode* curr = head;
    while (curr != nullptr) {
        DLinkedNode* next_node = curr->next;
        delete curr;
        curr = next_node;
    }
}

// 3. 私有微操指令集 (完全就是你刚才推导的逻辑)
void LRUCache::addNode(DLinkedNode* node) {
    DLinkedNode* first_real = head->next;
    node->prev = head;
    head->next = node;
    node->next = first_real;
    first_real->prev = node;
}

void LRUCache::removeNode(DLinkedNode* node) {
    DLinkedNode* prev_node = node->prev;
    DLinkedNode* next_node = node->next;
    prev_node->next = next_node;
    next_node->prev = prev_node;
}

void LRUCache::moveToHead(DLinkedNode* node) {
    removeNode(node); // 拔下
    addNode(node);    // 插到头部
}

DLinkedNode* LRUCache::removeTail() {
    DLinkedNode* target = tail->prev;
    removeNode(target);
    return target;
}

// ==========================================
// 4. 面向客户的 API 层 (哈希表与链表的完美配合)
// ==========================================

int LRUCache::get(int key) {
    // 查哈希雷达，看是否在机房里
    if (cache.find(key) == cache.end()) {
        return -1; // 查无此人
    }
    // 找到了！瞬间拿到物理地址，并提拔为全场最 VIP
    DLinkedNode* node = cache[key];
    moveToHead(node); 
    return node->value;
}

void LRUCache::put(int key, int value) {
    if (cache.find(key) != cache.end()) {
        // 老客户发新包：更新数据，提拔到头部
        DLinkedNode* node = cache[key];
        node->value = value;
        moveToHead(node);
    } else {
        // 全新客户接入
        DLinkedNode* new_node = new DLinkedNode(key, value);
        cache[key] = new_node; // 注册到雷达
        addNode(new_node);     // 插到 VIP 席位
        size++;

        // 警告：内存水池满了！
        if (size > capacity) {
            DLinkedNode* tail_node = removeTail(); // 揪出最老的节点
            cache.erase(tail_node->key);           // 从雷达上抹除记录
            delete tail_node;                      // 真正的物理销毁！
            size--;
        }
    }
}