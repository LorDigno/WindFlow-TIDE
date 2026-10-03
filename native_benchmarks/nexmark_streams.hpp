#ifndef NEXMARK_STREAMS_HPP
#define NEXMARK_STREAMS_HPP

#include <string>
#include <cstdint>

// 1. Stream delle Persone (NEXMark Person)
struct Person {
    int64_t person_id = 0;
    std::string name;
    std::string email_address;
    std::string credit_card;
    std::string city;
    std::string state;
    uint64_t person_dateTime = 0;
    std::string extra;
};

// 2. Stream delle Aste (NEXMark Auction)
struct Auction {
    int64_t auction_id = 0;
    std::string item_name;
    std::string description;
    int64_t initial_bid = 0;
    int64_t reserve = 0;
    uint64_t auction_dateTime = 0;
    uint64_t expires = 0;
    int64_t seller = 0;
    int64_t category = 0;
};

// 3. Stream delle Offerte (NEXMark Bid)
struct Bid {
    int64_t auction_id = 0;
    int64_t bidder = 0;
    int64_t price = 0;
    std::string channel;
    std::string url;
    uint64_t bid_dateTime = 0;
    std::string extra;
};

#endif // NEXMARK_STREAMS_HPP