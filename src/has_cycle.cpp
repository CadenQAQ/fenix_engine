#include "../include/fenix_algo.h"
#include <iostream>

// src/has_cycle.cpp
#include "../include/fenix_algo.h"

bool has_cycle(ListNode* head) {
    // 防御：如果是空机房，或者只有一台没有连线的交换机，绝对不可能有环
    if (head == nullptr || head->next == nullptr) {
        return false;
    }

    ListNode* slow = head; // 乌龟
    ListNode* fast = head; // 兔子

    // 核心物理引擎：
    // 兔子跑得快，所以探雷的重任在兔子身上。只要兔子前面没悬崖，就一直跑。
    while (fast != nullptr && fast->next != nullptr) {
        slow = slow->next;          // 乌龟走 1 步
        fast = fast->next->next;    // 兔子走 2 步

        // 如果在某一台交换机相遇了，直接拉响警报！
        if (slow == fast) {
            return true;
        }
    }

    // 兔子顺利走到了 nullptr，说明这是一条正常的单向光纤，安全。
    return false;
}

/*
1. 网络 Infra 背景：在核心机房里，交换机之间通过光纤连接，形成了一个单向的链路。
正常情况下，数据包顺着链路走，最后要么到达终点，要么被丢弃（遇到 nullptr）。
但是，如果某个网络工程师配置 BGP 路由表时手抖了，让最后一台交换机的光纤，
不小心插回了前面的某台交换机上，会发生什么？这在网络层叫路由环路（Routing Loop）。
数据包会在这个圈里无限循环，疯狂消耗网络带宽，最终引发“广播风暴（Broadcast Storm）”，
把整个机房的网络瞬间打瘫。
2. 你的防线：你需要在 Fenix 引擎里写一个极速探测雷达：给它一条链路的起点 head，
它必须在微秒级判断出，这条链路到底有没有环（Cycle）。🚫 为什么不能用哈希表（Hash Set）？
如果是互联网写 Java/Python 的程序员，第一反应肯定是：“这题我会！我建一个 HashSet，
每走过一个交换机，就把它的 IP（内存地址）存进去。
如果走到一个交换机，发现它已经在 Set 里了，说明有环！”在 Infra 层面，直接毙掉！
因为建立 HashSet 需要疯狂地在堆内存上 new 节点。
当一秒钟几百万个数据包涌入雷达时，你的 HashSet 会把操作系统的内存打爆。
我们要的是 $O(1)$ 的空间复杂度：绝对不允许申请任何额外的内存！
🏎️ 底层解法：“龟兔赛跑”双指针 (Floyd's Tortoise and Hare)既然不能做历史记录，我们就用相对速度来破局。
请在脑子里建立这样一个物理模型：想象一个跑道。如果跑道是直的（没有环），跑得快的人（兔子）永远会先到达终点（nullptr）。
但如果跑道是一个圆圈（有环），跑得快的人不仅不会遇到终点，反而会在绕圈的过程中，从背后“套圈”追上跑得慢的人（乌龟）！*/