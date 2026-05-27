// include/orderbook.h
#pragma once
#include <map>
#include <cstdint>
#include <iostream>

// 订单簿核心组件
class Orderbook {
private:
    // 🌳 卖盘 (Asks)：卖家想卖高价，但市场优先撮合最低价。
    // Key: 价格 (Price), Value: 该价格上的总挂单量 (Qty)
    // 默认按照 std::less 排序：低价在树的最左端 (begin)
    std::map<uint32_t, uint32_t> asks; 

    // 🌳 买盘 (Bids)：买家想捡便宜，但市场优先撮合最高价。
    // 我们强制使用 std::greater<uint32_t>，让高价永远排在树的最前面 (begin)
    std::map<uint32_t, uint32_t, std::greater<uint32_t>> bids;

public:
    Orderbook() {}

    // 接收网络层解析出来的纯净订单，挂入红黑树
    // side: 1 代表买 (Bid), 2 代表卖 (Ask)
// include/orderbook.h (只展示修改的核心部分)

    // 升级版：带有主动撮合逻辑的订单接收器
    void add_order(uint16_t side, uint32_t price, uint32_t qty) {
        if (side == 1) { // 🟢 暴躁的买家来了 (Bid)
            // 只要我还想买(qty>0)，且卖盘还有人(!empty)，且卖一价 <= 我的最高出价
            while (qty > 0 && !asks.empty() && asks.begin()->first <= price) {
                auto best_ask = asks.begin(); // 锁定卖一价（红黑树最左侧）
                uint32_t ask_price = best_ask->first;
                
                // 计算这次能吃掉多少？取“我想要的”和“他能给的”两者中的最小值
                uint32_t match_qty = std::min(qty, best_ask->second);
                
                std::cout << "⚡ [MATCH] BUYER crossed spread! Trade at Price: " 
                          << ask_price << " | Qty: " << match_qty << std::endl;

                // 双方同时扣减弹药
                qty -= match_qty;
                best_ask->second -= match_qty;

                // 💀 如果这个卖家的货被吃光了，直接把这个节点从红黑树里连根拔起！
                if (best_ask->second == 0) {
                    asks.erase(best_ask); // O(log N) 极速删除
                }
            }
            // 经过一顿狂吃，如果自己还有剩的，就乖乖挂到买盘树上排队
            if (qty > 0) {
                bids[price] += qty;
            }
        } 
        else if (side == 2) { // 🔴 暴躁的卖家来了 (Ask)
            // 只要我还想卖，且买盘还有人，且买一价 >= 我的最低要价
            while (qty > 0 && !bids.empty() && bids.begin()->first >= price) {
                auto best_bid = bids.begin(); // 锁定买一价（红黑树最前端）
                uint32_t bid_price = best_bid->first;
                
                uint32_t match_qty = std::min(qty, best_bid->second);
                
                std::cout << "⚡ [MATCH] SELLER crossed spread! Trade at Price: " 
                          << bid_price << " | Qty: " << match_qty << std::endl;

                qty -= match_qty;
                best_bid->second -= match_qty;

                if (best_bid->second == 0) {
                    bids.erase(best_bid);
                }
            }
            if (qty > 0) {
                asks[price] += qty;
            }
        }
    }

    // 打印当前 L2 盘口状态 (类似你在炒股软件上看到的十档行情)
    void print_snapshot() {
        std::cout << "\n=== 📊 Fenix L2 Orderbook Snapshot ===" << std::endl;
        
        std::cout << "[ ASKS ] (Sellers)" << std::endl;
        // 倒序打印少部分，模拟真实的上方卖盘
        for (auto it = asks.rbegin(); it != asks.rend(); ++it) {
            std::cout << "  Price: " << it->first << " | Qty: " << it->second << std::endl;
        }

        std::cout << "-----------------------------------" << std::endl;

        std::cout << "[ BIDS ] (Buyers)" << std::endl;
        // 顺序打印，买一价（最高价）永远在最上面
        for (auto it = bids.begin(); it != bids.end(); ++it) {
             std::cout << "  Price: " << it->first << " | Qty: " << it->second << std::endl;
        }
        std::cout << "=======================================\n" << std::endl;
    }
};