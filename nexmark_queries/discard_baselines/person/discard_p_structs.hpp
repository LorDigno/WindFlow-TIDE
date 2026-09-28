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
    int64_t person_id;
    std::string name;
    std::string email_address;
    std::string credit_card;
    std::string city;
    std::string state;
    uint64_t person_dateTime;
    std::string extra;


};


#endif // DISCARD_P_STRUCTS_HPP