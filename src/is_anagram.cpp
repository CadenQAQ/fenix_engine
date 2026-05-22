#include "../include/fenix_algo.h"
#include<iostream>
#include<unordered_map>
using namespace std;

/*只有小写字母的字符串是否有重复字符*/
bool is_anagram(const std::string& s1, const std::string& s2)
{
    unordered_map<char,int> m1;

    if(s1.size()!=s2.size())
    {
        return false;
    }

    for(const auto &c: s1)
    {
        m1[c]++;
    }

    for(const auto &c: s2)
    {
        m1[c]--;
    }

    for(const auto &c:m1)
    {
        if (c.second!=0)
        return false;
    }
   
    return true;
}

int main()
{
    string a = "badc";
    string b ="abbd";
    cout<<is_anagram(a,b);
}

/*量化真实场景落地：一篮子交易 (Basket Trading) 的秒级对账
1. Infra 人的本能联想：
乱序到达的网络数据包完整性校验，或者日志文件两端（发送端和接收端）的事件匹配。

2. 量化真实场景落地：ETF 篮子买卖对账
在做市商或指数套利策略中，我们经常使用一种叫“一篮子交易”的技术。
比如，策略决定买入沪深300指数，它会瞬间向交易所发出 300 笔不同股票的买单（假设这就是字符串 s1，里面包含了 A, B, C... 不同的股票代码）。

交易所撮合完成后，会把成交回报（Execution Report）发给你。但是！网络延迟和撮合引擎的队列会导致回报是完全乱序的（这就是字符串 s2，顺序变成了 C, A, B...）。

在盘中，风控系统必须做一个极速校验：“交易所返回的这堆乱序的股票成交，和我刚才发出去的那 300 只股票，是不是完美对应的？”
多成交了不行（错单），少成交了也不行（漏单）。这本质上就是判断 s1 和 s2 是不是 Anagram！

性能极客视角 (Hot Path 终极魔改)
痛点分析
你用了 unordered_map。在基础开发里这没毛病。
但对于 Infra 极客来说，unordered_map 的底层极其沉重：它要算 Hash 算法，要处理哈希冲突（链表或红黑树），最致命的是它要在堆内存（Heap）里给每一个新字符 new 一个节点。在纳秒级网关里，这等于自杀。

极致魔改：完美哈希 (Perfect Hashing) —— 把数组当 Map 用
核心思想： 既然我们知道所有的字符都属于 ASCII 码，而 ASCII 码最多只有 256 个（从 0 到 255）。我们为什么还要用 Map 呢？我们直接在栈上开一个长度为 256 的原生数组，把字符的 ASCII 码当成数组的下标，也就是绝对的“完美哈希”！

#include <iostream>
#include <string>

// 极致魔改：抛弃 Map，用定长数组降维打击
bool is_anagram_fast(const std::string& s1, const std::string& s2) {
    if (s1.size() != s2.size()) {
        return false;
    }

    // 栈上分配一个大小为 256 的数组，全初始化为 0。
    // 这比 unordered_map 快了成百上千倍，0 内存分配！
    int counts[256] = {0}; 

    // 一个循环搞定加减
    for (size_t i = 0; i < s1.size(); ++i) {
        // char 本质上就是一个数字。强制转成无符号作为下标
        counts[(unsigned char)s1[i]]++; 
        counts[(unsigned char)s2[i]]--;
    }

    // 最后检查是否有残留
    for (int i = 0; i < 256; ++i) {
        if (counts[i] != 0) {
            return false;
        }
    }
    return true;
}

int main() {
    std::string a = "badc";
    std::string b = "abbd";
    std::cout << std::boolalpha << is_anagram_fast(a, b) << std::endl;
    return 0;
}
*/