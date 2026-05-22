#include "../include/fenix_algo.h"
#include <iostream>
#include <vector>

using namespace std;
/*有可能重复元素的，两次二分法
Find First and Last Position of Element in Sorted Array
*/

int find_first(vector<int>& nums, int target)
{
    int left = 0;
    int right = nums.size() - 1;

    while (left <= right)
    {
        int mid = left + (right - left) / 2;

        if (nums[mid] >= target)
            right = mid - 1;
        else
            left = mid + 1;
    }

    if (left < nums.size() && nums[left] == target)
        return left;

    return -1;
}

int find_last(vector<int>& nums, int target)
{
    int left = 0;
    int right = nums.size() - 1;

    while (left <= right)
    {
        int mid = left + (right - left) / 2;

        if (nums[mid] <= target)
            left = mid + 1;
        else
            right = mid - 1;
    }

    if (right >= 0 && nums[right] == target)
        return right;

    return -1;
}

vector<int> search_range(vector<int>& nums, int target)
{
    return {find_first(nums, target), find_last(nums, target)};
}

int main()
{
    vector<int> nums = {1,2,2,2,3};
    vector<int> result = search_range(nums, 2);

    for (auto v : result)
        cout << v << endl;
}

/*
场景一：行情回放与分析 —— 提取“同一微秒内”的并发事件簇
1. 量化数据背景：在交易所的高频数据流中，时间戳（Timestamp）通常是递增的。
但是，由于交易所撮合引擎的批量处理机制，或者网络数据包的聚合（Batching），
系统经常会在同一个时间戳（比如同一微秒）爆发性地产生几十上百条成交记录。
此时，按时间戳排序的数组 nums 里，就会出现大量重复的时间戳：[..., 10245, 10245, 10245, 10246, ...]。
2. 你的代码完美落地：策略研究员需要计算：“在 10245 这一微秒内，市场总共成交了多少资金？”
你用 find_first 瞬间锁定这批订单的起始内存地址。你用 find_last 瞬间锁定这批订单的结束内存地址。
拿到 [left, right] 之后，底层代码直接拿着这两个指针去做一个内存块的批量聚合（SIMD 指令或者简单的遍历求和）。
如果不用这种两次二分法，面对海量的同级别数据，你就无法精准切割出时间切片，回测系统就无法还原某一微秒的微观盘口状态。

场景二：极速撮合引擎 —— 订单簿（Orderbook）同价位订单的批量清算
1. 交易核心背景：假设你在写一个微型的极速撮合引擎。买盘的订单按照价格从低到高排序存放在一个数组里。
在某个热门股票上，价格为 $10.50 的位置上，挂着几百个不同散户和机构的买单。
此时，数组的形态是按照价格排序的订单集合：[..., 10.49, 10.50, 10.50, 10.50, 10.51, ...]。
2. 你的代码完美落地：突然，市场砸下来一个巨大的市价卖单，要把 $10.50 这个价位的所有买单全部吃光！
作为撮合引擎，你必须在纳秒级把这些 $10.50 的买单全部标记为“已成交”。
引擎传入 target = 10.50。调用 find_first 和 find_last，立刻得到了 $10.50 价格档位的订单范围：比如第 500 个到第 850 个。
接下来，你根本不需要一个个去遍历比较价格，直接用一个简单的 for (int i = 500; i <= 850; ++i)，甚至直接用 memset 批量修改这块连续内存的状态。
你用 $O(\log N)$ 的极小代价，框出了一片可以被集体屠宰的“羊群”。

⚡ 性能极客视角：为什么不用“二分找出一个，然后向两边扩散遍历”？
很多初学者看到这道题，第一反应是：“我用普通二分法先随便找到一个 2，然后写个 while 循环向左找开头，再向右找结尾，这不是只用写一个二分查找吗？”
作为 Infra 工程师，你必须在代码审查（Code Review）时直接毙掉那种写法！
底层灾难推演：假设这只股票停牌了，一整天只有 $10.50 这个价格，数组里有 1000 万个重复的 $10.50。
向两边遍历的写法： 找到中间后，向左遍历 500 万次，向右遍历 500 万次。时间复杂度瞬间退化成了 $O(N)$。系统被死死卡住。
你写的两次二分法： 无论有多少个重复元素，找左边界只要 $\log_2(1000万) \approx 24$ 次，找右边界也只要 24 次。
一共 48 次循环，绝对的性能碾压。在超低延迟系统中，“避免最坏情况（Worst-case Execution Time, WCET）”比优化平均情况更重要。
你的这种写法，完美封死了性能退化的任何可能性，保证了网关无论面对多极端的行情风暴，延迟始终是一根直线。
*/