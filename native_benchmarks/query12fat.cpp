#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <windflow.hpp>
#include <windflow_table_api.hpp>
#include "nexmark_streams.hpp"

struct Q12_Key {
    int64_t bidder = 0;
    
    // operator== obbligatorio per la risoluzione hash
    bool operator==(const Q12_Key& other) const {
        return bidder == other.bidder;
    }
};

namespace std {
    template<>
    struct hash<Q12_Key> {
        size_t operator()(const Q12_Key& k) const {
            return std::hash<int64_t>{}(k.bidder);
        }
    };
}

struct Q12_Group_Out {
    int64_t bidder = 0;
    int64_t processed = 0;
    uint64_t win_id = 0;

    // Costruttori imposti dall'architettura WindFlow FFAT
    Q12_Group_Out() = default;
    Q12_Group_Out(uint64_t _id) : win_id(_id) {}
    Q12_Group_Out(const Q12_Key& _key, uint64_t _id) 
        : bidder(_key.bidder), win_id(_id) {}
};

class Q12_FFAT_Lift {
public:
    void operator()(const Bid& in, Q12_Group_Out& out) {
        // La chiave (bidder) e l'ID finestra sono iniettati dal costruttore
        out.processed = 1; 
    }
};

class Q12_FFAT_Combine {
public:
    void operator()(const Q12_Group_Out& left, const Q12_Group_Out& right, Q12_Group_Out& out) {
        // out.bidder e out.win_id sono garantiti dal costruttore
        out.processed = left.processed + right.processed;
    }
};

int main(int argc, char* argv[]) {

    auto bid_src = Table_Source_Builder<Bid>(
            "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_datasets/30m_bid.csv", 
            [](const std::string& line, Bid& record, uint64_t& timestamp) {
                std::stringstream ss(line);
                std::string token;

                std::getline(ss, token, ','); record.auction_id = parse_BIGINT(token);
                std::getline(ss, token, ','); record.bidder = parse_BIGINT(token);
                std::getline(ss, token, ','); record.price = parse_BIGINT(token);
                std::getline(ss, token, ','); record.channel = parse_STRING(token);
                std::getline(ss, token, ','); record.url = parse_STRING(token);
                std::getline(ss, token, ','); record.bid_dateTime = parse_ISO8601(token); 
                std::getline(ss, token, ','); record.extra = parse_STRING(token);
            }
        )
        .withName("bid_source")
        .withHeader()
        .withParallelism(2, 134217728ULL) // Blocco da 128MB
        .build();

    // --- 2. FFAT WINDOWS (1000 ms) ---
    Q12_FFAT_Lift lift_logic;
    Q12_FFAT_Combine comb_logic;    
    auto ffat_window_group = wf::Ffat_Windows_Builder<Q12_FFAT_Lift, Q12_FFAT_Combine, Q12_Key>(lift_logic, comb_logic)
        .withName("q12_ingress_ffat_windows")
        .withParallelism(2)
        .withKeyBy([](const Bid& in) -> Q12_Key { return Q12_Key{in.bidder}; })
        .withTBWindows(std::chrono::milliseconds(1000), std::chrono::milliseconds(1000))
        .build();

    auto sink = Table_Sink_Builder<Q12_Group_Out>(
            "q12fat_output",
            [](const Q12_Group_Out& record, std::ostream& os) {
                os << record.bidder << "," << record.processed;
            }
        )
        .withName("q12_sink")
        .withHeader("bidder,processed")
        .withParallelism(2)
        .build();

    wf::PipeGraph topology(
        "q12_ffat", 
        wf::Execution_Mode_t::DEFAULT, 
        wf::Time_Policy_t::INGRESS_TIME
    );

    topology.add_source(bid_src)
            .add(ffat_window_group)
            .add_sink(sink);

    topology.run();
    return 0;
}