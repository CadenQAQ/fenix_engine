// main.cpp
#include <iostream>
#include <vector>
#include "fenix_algo.h"

using namespace std;

int main() {
    // 注入 Fenix 灵魂
    cout << "=== Fenix Gateway Engine v1.0 ===" << endl;
    cout << "System Booting... Modules Loaded." << endl;
    
    vector<int> nums = {1, 5, 6, 8, 22, 29, 30, 45};
    int target = 30;
    int result = binary_search(nums, target);
    
    cout << "Target " << target << " found at index: " << result << endl;
    
    return 0;
}