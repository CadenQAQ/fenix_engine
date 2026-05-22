#include "../include/fenix_algo.h"

#include<iostream>
#include<vector>

using namespace std;


int binary_search(vector<int>& nums, int target)
{
    int left = 0;
    int right = nums.size() - 1;

    while (left <= right)
    {
        int mid = left + (right - left) / 2;

        if (nums[mid] == target)
            return mid;
        else if (nums[mid] < target)
            left = mid + 1;
        else
            right = mid - 1;
    }

    return -1;
}


// int main()
// {   
//     vector<int>num = {1,5,6,8,22,29,30,45};
//     cout<<binary_search(num, 30);
    
// }