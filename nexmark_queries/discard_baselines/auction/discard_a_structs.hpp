#ifndef DISCARD_A_STRUCTS_HPP
#define DISCARD_A_STRUCTS_HPP

#include <string>
#include <cstdint>
#include <functional>
#include <limits>

// ============================================================================
// Struct: source_discard_a_from_2
// ============================================================================
struct source_discard_a_from_2 {
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


#endif // DISCARD_A_STRUCTS_HPP