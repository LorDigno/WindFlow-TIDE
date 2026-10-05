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

    Q12_Group_Out() = default;
    Q12_Group_Out(uint64_t _id) : win_id(_id) {}
    Q12_Group_Out(const Q12_Key& _key, uint64_t _id) 
        : bidder(_key.bidder), win_id(_id) {}
};

class Q12_Window_Logic {
public:
    void operator()(const Bid& in, Q12_Group_Out& out) {
        // Logica di Count Aggregation
        out.processed += 1;
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
                
                // NOTA: In INGRESS_TIME il parametro 'timestamp' è ignorato dal framework.
                // Il tempo viene assegnato dall'hardware al momento del push.
            }
        )
        .withName("bid_source")
        .withHeader()
        .withParallelism(5, 33554432ULL) 
        .build();

    Q12_Window_Logic win_logic;
    auto window_group = wf::Keyed_Windows_Builder(win_logic)
        .withName("q12_ingress_windows")
        .withParallelism(5)
        .withKeyBy([](const Bid& in) -> Q12_Key { return Q12_Key{in.bidder}; })
        .withTBWindows(std::chrono::milliseconds(1000), std::chrono::milliseconds(1000))
        .build();

    auto sink = Table_Sink_Builder<Q12_Group_Out>(
            "q12_output",
            [](const Q12_Group_Out& record, std::ostream& os) {
                os << record.bidder << "," << record.processed;
            }
        )
        .withName("q12_sink")
        .withHeader("bidder,processed")
        .withParallelism(5)
        .build();

    wf::PipeGraph topology(
        "q12", 
        wf::Execution_Mode_t::DEFAULT, 
        wf::Time_Policy_t::INGRESS_TIME
    );

    topology.add_source(bid_src)
            .add(window_group)
            .add_sink(sink);

    topology.run();
    return 0;
}