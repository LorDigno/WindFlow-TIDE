#ifndef DISCARD_P_STRUCTS_HPP
#define DISCARD_P_STRUCTS_HPP

#include <string>
#include <cstdint>
#include <functional>
#include <limits>

// ============================================================================
// Struct: source_discard_p_from_2
// ============================================================================
struct source_discard_p_from_2 {
    int64_t auction_id;
    int64_t bidder;
    int64_t price;
    std::string channel;
    std::string url;
    uint64_t bid_dateTime;
    std::string extra;


};


#endif // DISCARD_P_STRUCTS_HPP