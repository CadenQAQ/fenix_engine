#include "../include/fenix_algo.h"
#include<iostream>
#include<vector>
#include<stack>
using namespace std;

/*Infra 人的本能联想
在 Linux 网络编程（如 epoll）或高频行情接收中，这叫 “Pending List”。
当你收到一个请求（或一笔订单），如果当前无法撮合，你会把它丢进一个容器里挂起（Pending），
等后面来了更大的价格（或满足了条件事件），再把它挨个弹出来处理。

想象你是量化交易系统里的一段风控代码。现在行情源源不断地进来一堆订单的价格（温度）：[73, 74, 75, 71, 69, 72]。

你的任务是：当一个高价（高气温）进来时，去触发并释放之前所有比它低的订单（低气温）。

栈在这个过程中，扮演的就是一个“垃圾未处理清单”。因为栈是“后进先出（LIFO）”的，正好符合“越晚进来的低价，越容易被最快打破”的逻辑。*/

void pending_list(const vector<int>& num, vector<int>& result)
{
    stack<int> s1; // 专门用来存“还在等待升温的日子的下标”

    for(int i = 0; i < num.size(); ++i)
    {
        // 核心：如果栈不为空，且今天气温打破了栈顶等待者的气温
        while(!s1.empty() && num[i] > num[s1.top()])
        {
            int prev_day = s1.top(); // 获取等待者的下标,s1存的是 1,2,3..这些数，不是值
            s1.pop();                // 解除等待状态，弹出

            // 算账：今天(i) 减去 过去那一天(prev_day) 就是等待天数
            // 直接精准写入对应的槽位
            result[prev_day] = i - prev_day; 
        }
        
        // 今天的气温也要进栈去等待未来的更高温
        s1.push(i);
    }
    // 循环结束后，栈里剩下的那些下标，就是永远等不到更高温的。
    // 因为我们初始化 result 时全是 0，所以它们自然就是 0，无需额外处理。
}

int main()
{
    vector<int> num = {73, 74, 75, 71, 69, 72};
    
    // Infra 习惯：提前分配好精确的内存空间，并全部初始化为 0
    vector<int> result(num.size(), 0); 

    pending_list(num, result);

    for(auto v : result)
    {
        cout << v << ' ';
    }
    cout << '\n';
    
    // 预期输出: 1 1 0 2 1 0
    return 0;
}