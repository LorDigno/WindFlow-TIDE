#ifndef MAX_BIDS_STRUCTS_HPP
#define MAX_BIDS_STRUCTS_HPP

#include <string>
#include <cstdint>
#include <functional>
#include <limits>

// ============================================================================
// Struct: source_max_bids_from_3
// ============================================================================
struct source_max_bids_from_3 {
    int64_t auction_id;
    int64_t bidder;
    int64_t price;
    std::string channel;
    std::string url;
    uint64_t bid_dateTime;
    std::string extra;


};


// ============================================================================
// Struct: window_max_window_group_by_5_struct_out
// ============================================================================
struct window_max_window_group_by_5_struct_out {
    int64_t MAX_price = std::numeric_limits<int64_t>::lowest(); 
    uint64_t win_id = 0; 

    window_max_window_group_by_5_struct_out() = default;

    window_max_window_group_by_5_struct_out(uint64_t _id) 
        : win_id(_id) {}


};


// ============================================================================
// Struct: window_max_select_4_struct_out
// ============================================================================
struct window_max_select_4_struct_out {
    int64_t price;


    bool operator==(const window_max_select_4_struct_out& other) const {
        return price == other.price;
    }
};

namespace std {
    template<>
    struct hash<window_max_select_4_struct_out> {
        size_t operator()(const window_max_select_4_struct_out& k) const {
            size_t h = 0;
            h ^= std::hash<int64_t>{}(k.price) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };
}

// ============================================================================
// Struct: max_bids_select_1_struct_out
// ============================================================================
struct max_bids_select_1_struct_out {
    int64_t auction_id;
    int64_t price;
    int64_t bidder;


};


#endif // MAX_BIDS_STRUCTS_HPP