#include "../include/fenix_algo.h"
#include<iostream>
#include<vector>
#include<algorithm>

using namespace std;

int search_insert(vector<int> num, int target)
{
    int left = 0;
    int right = num.size() - 1;


    
    while (left <= right)
    {
        int mid = left + (right - left) / 2;

        if (num[mid] >= target)
            right = mid - 1;
        else
            left = mid + 1;
    }

    //if (left < num.size() && num[left] == target)
    return left;



}

// int main()
// {
//     vector<int> nums = {0,4,6,9,13,15};
    

//     cout<<search_insert(nums, 8);
// }