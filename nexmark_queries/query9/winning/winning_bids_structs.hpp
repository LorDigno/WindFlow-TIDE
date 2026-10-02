#ifndef WINNING_BIDS_STRUCTS_HPP
#define WINNING_BIDS_STRUCTS_HPP

#include <string>
#include <cstdint>
#include <functional>
#include <limits>

// ============================================================================
// Struct: source_winning_bids_from_4
// ============================================================================
struct source_winning_bids_from_4 {
    int64_t auction_id;
    int64_t bidder;
    int64_t price;
    std::string channel;
    std::string url;
    uint64_t bid_dateTime;
    std::string extra;


};


// ============================================================================
// Struct: source_winning_bids_from_5
// ============================================================================
struct source_winning_bids_from_5 {
    int64_t auction_id;
    std::string item_name;
    std::string description;
    int64_t initial_bid;
    int64_t reserve;
    uint64_t auction_dateTime;
    uint64_t expires;
    int64_t seller;
    int64_t category;


};


// ============================================================================
// Struct: source_winning_bids_from_4_unified_source_winning_bids_from_5
// ============================================================================
struct source_winning_bids_from_4_unified_source_winning_bids_from_5 {
    int64_t auction_id;
    int64_t bidder;
    int64_t price;
    std::string channel;
    std::string url;
    uint64_t bid_dateTime;
    std::string extra;
    std::string item_name;
    std::string description;
    int64_t initial_bid;
    int64_t reserve;
    uint64_t auction_dateTime;
    uint64_t expires;
    int64_t seller;
    int64_t category;


};


// ============================================================================
// Struct: winning_bids_join_interval_3_key_struct
// ============================================================================
struct winning_bids_join_interval_3_key_struct {
    int64_t auction_id;


    bool operator==(const winning_bids_join_interval_3_key_struct& other) const {
        return auction_id == other.auction_id;
    }
};

namespace std {
    template<>
    struct hash<winning_bids_join_interval_3_key_struct> {
        size_t operator()(const winning_bids_join_interval_3_key_struct& k) const {
            size_t h = 0;
            h ^= std::hash<int64_t>{}(k.auction_id) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };
}

// ============================================================================
// Struct: winning_bids_window_group_by_2_struct_out
// ============================================================================
struct winning_bids_window_group_by_2_struct_out {
    int64_t auction_id;
    int64_t MAX_price = std::numeric_limits<int64_t>::lowest(); 
    uint64_t win_id = 0; 

    winning_bids_window_group_by_2_struct_out() = default;

    winning_bids_window_group_by_2_struct_out(uint64_t _id) 
        : win_id(_id) {}

    winning_bids_window_group_by_2_struct_out(const winning_bids_join_interval_3_key_struct& _key, uint64_t _id) 
        : auction_id(_key.auction_id), win_id(_id) {}

};


// ============================================================================
// Struct: winning_bids_select_1_struct_out
// ============================================================================
struct winning_bids_select_1_struct_out {
    int64_t auction_id;
    int64_t winning_price;


};


#endif // WINNING_BIDS_STRUCTS_HPP