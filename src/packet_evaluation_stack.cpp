#include "../include/fenix_algo.h"
#include<iostream>
#include<stack>
#include<vector>
#include<string>

using namespace std;


using namespace std;

// 现实场景：用栈计算已经被拍扁的网络包/风控规则指令流 (后缀表达式)
bool evaluate_packet_rules(const vector<string>& instructions)
{
    // 这个栈里存的不是结构，而是“计算结果的临时停机坪”
    stack<bool> calc_stack;

    for (const string& inst : instructions)
    {
        if (inst == "AND")
        {
            // 遇到 AND 操作符，从栈顶弹出两个最近的计算结果
            bool result2 = calc_stack.top(); calc_stack.pop();
            bool result1 = calc_stack.top(); calc_stack.pop();
            // 计算并将新结果压回栈中
            calc_stack.push(result1 && result2);
        }
        else if (inst == "OR")
        {
            // 遇到 OR 操作符，同样弹出两个结果合并
            bool result2 = calc_stack.top(); calc_stack.pop();
            bool result1 = calc_stack.top(); calc_stack.pop();
            calc_stack.push(result1 || result2);
        }
        else
        {
            // 遇到具体条件（在实际系统中，这里会去读取包头对应的位掩码做判断）
            // 为了演示，我们假设 "TRUE_FLAG" 代表条件成立，其他代表不成立
            if (inst == "TRUE_FLAG") {
                calc_stack.push(true);
            } else {
                calc_stack.push(false);
            }
        }
    }

    // 指令流跑完后，栈里剩下的最后那个 bool，就是这个包的最终命运（放行 or 拦截）
    return calc_stack.top();
}

int main()
{
    // 模拟场景：判断一个包是否合法
    // 原始规则：(TRUE_FLAG AND FALSE_FLAG) OR TRUE_FLAG
    // 拍扁后的指令流：
    vector<string> rule_stream = {"TRUE_FLAG", "FALSE_FLAG", "AND", "TRUE_FLAG", "OR"};

    cout << boolalpha;
    cout << "数据包最终校验结果: " << evaluate_packet_rules(rule_stream) << endl; 
    // 输出: true (因为 false OR true = true)

    return 0;
}