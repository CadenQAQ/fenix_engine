#include "../include/fenix_algo.h"
#include<iostream>
#include <vector>
#include <string>
#include <algorithm> // 为了使用 max

using namespace std;

string s1="abcabcbb";
string s2="bbbbb";
string s3="pwwkew";   



using namespace std;

int length_of_longest_substring(const std::string& s)
{
    int left = 0; // 修正 2：明确初始化
    int length = 0;
    

    vector<int> last_appear(256, -1); 


    for(int right = 0; right < s.size(); ++right)
    {
        // 直接取当前 right 指向的字符
        unsigned char c = s[right]; 
        
        if(last_appear[c] >= left)
        {
            left = last_appear[c] + 1;
        } 

        last_appear[c] = right;
        
        // 修正 4：此时 right 还没自增，直接套用公式
        length = max(length, right - left + 1); 
    }

    return length;
}

int main()
{
    string s1 = "abcabcbb";
    // 预期结果：3 (abc)
    cout << length_of_longest_substring(s1) << endl;
    return 0;
}

/*
🚀 量化真实场景落地：高频特征工程与“订单流毒性” (Order Flow Toxicity)
1. Infra 人的本能联想：
TCP 协议底层的滑动窗口（Sliding Window）流量控制，或者是日志监控中的“异常风暴”消抖机制。

2. 量化真实场景落地：微观结构特征提取 (Microstructure Feature Engineering)
在极高频（HFT）做市策略中，我们需要实时给当前的市场状态“打分”。
假设交易所一微秒内疯狂涌入了大量订单事件，我们将这些事件编码为字符：

a = 散户小单买入

b = 机构大单砸盘

c = 冰山订单撤单

d = 做市商修改报价

一瞬间你收到了一串事件流：s = "abcabcbb"。
策略研究员经常需要提取一个特征：在当前的市场切片中，最长的一段“完全不重复的连续动作”有多长？

如果重复动作极多（比如 "bbbbbb"，全是机构在无脑砸盘），说明市场情绪极度一致，呈现单边行情，做市商必须立刻撤单保命（这叫应对订单流毒性）。

如果一段窗口内包含了大量不同的动作（比如 "abcd"，买卖撤改交替出现），说明多空双方在激烈博弈，市场处于震荡洗盘状态，做市商可以双边挂单赚取差价。

你写的这个 length_of_longest_substring，就是一段在纳秒级别实时计算“市场博弈复杂度”的极速特征提取器！
*/