#include "../include/fenix_algo.h"

ListNode* reverse_list(ListNode* head) {
    ListNode* prev = nullptr;
    ListNode* curr = head;

    // 当你 (curr) 还没有走到悬崖边缘 (nullptr) 时
    while (curr != nullptr) {
        
        // TODO: 请在这里写下 4 行拔插光纤的指针微操！
        // 1. 保住生命线 next_temp
        // 2. 拔下光纤向后插
        // 3. prev 往前挪
        // 4. curr 往前挪
       ListNode* next_temp = curr->next;
       curr->next = prev;
       prev = curr;
       curr = next_temp;
     
    }

    // 循环结束后，prev 就是新链表的头节点
    return prev;
}