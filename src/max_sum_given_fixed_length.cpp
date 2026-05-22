#include "../include/fenix_algo.h"
#include<iostream>
#include<vector>
#include <algorithm>
using namespace std;

// int max_sum(const int *arr, int size,int k)
// {
   

//     vector<int>result ;

//     for(int i = 0; i <size-k+1;++i)
//     {
//         int sum = 0;
//         for (int j = 0;j<k;++j)
//         {
         
//             sum +=arr[i+j];
//         }
//         result.push_back(sum);


//     }
 
//     // for(auto const &v : result)
//     // {
//     //     cout<<v<<endl;
//     // }
//     int maxSum = *max_element(result.begin(), result.end());

//     return maxSum;
// }

/*指定长度窗口求和，找最大值*/
int max_sum_given_fixed_length(const int* arr, int size, int k)
{
    if (k <= 0 || size < k) return 0;

    int sum = 0;
    for (int i = 0; i < k; ++i)
        sum += arr[i];

    int max_num = sum;   // ✅ 关键：包含第一个窗口

    int left = 0;
    for (int j = k; j < size; ++j)
    {
        sum += arr[j] - arr[left];
        ++left;
        max_num = std::max(max_num, sum);
    }

    return max_num;
}
int main()
{
    int arr[5] = {1,2,3,4,5};
    cout<<max_sum_given_fixed_length(arr, 5,3); //3+4+5=12

}

/*
场景一：量化因子计算 —— 寻找“日内最大流动性时间段”（Volume Burst / Liquidity Profile）
1. 业务背景：假设你在写一个 VWAP（成交量加权平均价）的大单拆单算法。
你需要把 100 万股慢慢卖出去，为了不砸盘，你必须在市场交易最活跃的时间段多卖，在清淡的时间段少卖。
策略研究员扔给你一天 4800 秒（每秒一条）的成交量数据 arr，并问你：“今天盘中，哪连续的 300 秒（$K = 300$）成交量最大？”
2. 你的代码完美落地：如果用笨办法，每一秒都往后去算 300 秒的总和，一天要算 $4800 \times 300 = 144万$ 次加法。
但用你的这套“一进一出”的代码，窗口滑动一次，只要做1 次加法（加上新一秒的量）和 1 次减法（减去 300 秒前的老量）。
在纳秒必争的量化系统里，你把计算量从百万次压缩到了 4800 次。不仅找到了流动性最好的时间段，还几乎不占用 CPU 计算资源。

场景二：极速风控系统 —— 交易所限流阈值探测 (Message Rate Burst Detection)
1. Infra 背景：所有交易所（无论是传统的纳斯达克、深交所，还是 Crypto 的币安）都有极其严格的 API 频率限制（Rate Limit）。
比如规则是：“任意连续的 10 毫秒内，发送的订单总数不能超过 50 笔”，超过直接拔线封号。
2. 探测系统隐患：你的交易系统平时看起来很正常，一秒钟才发 200 个订单（平均 10 毫秒才 2 个）。但是，平均值是会骗人的。
有可能你的系统在遇到行情剧烈波动时，在某一个极其微小的 10 毫秒内，瞬间爆发发了 60 个单子，直接被交易所拉黑。
这时候，Infra 团队会拿着你的交易日志数组（arr 里存着每一毫秒发单的数量），设 $K = 10$，跑一遍你写的这段 max_sum。
如果返回的 max_num > 50，系统马上报警！它精准地指出了：虽然你平均速率合规，但在某个微观的 10 毫秒局部，你发生了严重的“流量微爆（Micro-burst）”。
⚡ 性能极客视角：为什么这行代码是精髓？你在代码里写的：sum += arr[j] - arr[left];在 CPU 底层看来，这叫“消除分支与循环（Loop Unrolling & Branch Elimination）”。
在连续的内存（数组）上，这种简单的加减法，CPU 连缓存都不用想，直接利用 L1 Cache 和流水线（Pipeline）一路平推过去。
现代编译器甚至会使用 SIMD（单指令多数据流）指令，把好几次加减法打包在一个时钟周期里算完。
总结一下：当你看到 “连续的 N 个”、“过去 X 分钟”、“定长窗口” 这种字眼，条件反射就应该祭出这套 + 新数据 - 老数据 的滑动窗口模板。
这也是所有移动平均线（SMA / Moving Average）的最底层实现！
*/

