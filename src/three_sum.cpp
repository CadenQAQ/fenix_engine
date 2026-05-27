#include "../include/fenix_algo.h"

/*找到不同的三个数相加等于0*/
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

vector<vector<int>> threeSum(vector<int>& nums) {
    vector<vector<int>> result;
    int n = nums.size();
    
    // 边界情况：数组长度小于3，无法形成三元组
    if (n < 3) return result;
    
    // 步骤1：排序，这是双指针法的基础
    sort(nums.begin(), nums.end());
    
    // 步骤2：固定第一个数，遍历到倒数第三个
    for (int first = 0; first < n - 2; first++) {
        // 跳过重复的第一个数（避免结果重复）
        if (first > 0 && nums[first] == nums[first - 1]) {
            continue;
        }
        
        // 优化：如果第一个数已经大于0，后面的数都大于0，不可能和为0
        if (nums[first] > 0) {
            break;
        }
        
        // 步骤3：双指针找另外两个数
        int second = first + 1;
        int third = n - 1;
        int target = -nums[first];  // 需要 nums[second] + nums[third] = -nums[first]
        
        while (second < third) {
            int currentSum = nums[second] + nums[third];
            
            if (currentSum == target) {
                // 找到一组解
                result.push_back({nums[first], nums[second], nums[third]});
                
                // 跳过重复的第二个数
                while (second < third && nums[second] == nums[second + 1]) {
                    second++;
                }
                // 跳过重复的第三个数
                while (second < third && nums[third] == nums[third - 1]) {
                    third--;
                }
                
                // 移动指针继续寻找下一组解
                second++;
                third--;
            }
            else if (currentSum < target) {
                // 和太小，左指针右移（增大和）
                second++;
            }
            else {
                // 和太大，右指针左移（减小和）
                third--;
            }
        }
    }
    
    return result;
}

// int main()
// {
//     vector<int> nums = {-1,0,1,2,-1,-4};
//     vector<vector<int>> result = threeSum(nums);

//      for(auto &v :result)
//      {
//         for(auto &value:v)
//         {
//             cout<<value<<'\n';
//         }

//         cout<<endl;
//      }

//      return 0;
// }

/*
业务背景：
这就是 three_sum.cpp 的真正用武之地。
量化研究员给了你一堆股票的“因子暴露度（Beta）”，排好序放在数组里：
[-4, -1, -1, 0, 1, 2]
研究员要求：“立刻给我找出 3 只股票，让它们的风险敞口相加刚好等于 0。我要构建一个完全对冲的投资组合！”

2. 你的代码完美落地：

最外层套一个 for 循环，先固定一只股票（比如固定 -4）。

剩下的问题瞬间降维成了 Two Sum II：你只需要在剩下的数组 [-1, -1, 0, 1, 2] 里，用 left 和 right 两个指针向中间对撞，寻找和为 4 的两只股票。

你在微秒级给出了答案：由于数组里最大只能凑出 1 + 2 = 3，凑不够 4。算法立刻告诉你：包含 -4 的风险中性组合不存在。
*/