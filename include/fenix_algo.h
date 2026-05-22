#pragma once 
#include <vector>
#include<string>

using namespace std;

int binary_search(vector<int>& nums, int target);
int find_first(vector<int>& nums, int target);
int find_last(vector<int>& nums, int target);
bool is_anagram(const std::string& s1, const std::string& s2);
int length_of_longest_substring(const std::string& s);
int max_sum_given_fixed_length(const int* arr, int size, int k);
bool evaluate_packet_rules(const vector<string>& instructions);
void pending_list(const vector<int>& num, vector<int>& result);
int subarray_sum(const vector<int>& nums, int target_sub_sum);
int search_insert(vector<int> num, int target);

