#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <chrono>
#include <windflow.hpp>
#include <windflow_table_api.hpp>
#include "nexmark_streams.hpp"


struct Q9_Unified {
    int64_t auction_id = 0;
    int64_t price = 0;
    uint64_t auction_dateTime = 0;
    uint64_t expires = 0;
    uint64_t bid_dateTime = 0;
};

struct Q9_Join_Out {
    int64_t auction_id = 0;
    int64_t price = 0;
};

struct Q9_Key {
    int64_t auction_id = 0;
    bool operator==(const Q9_Key& other) const {
        return auction_id == other.auction_id;
    }
};

namespace std {
    template<>
    struct hash<Q9_Key> {
        size_t operator()(const Q9_Key& k) const {
            return std::hash<int64_t>{}(k.auction_id);
        }
    };
}

struct Q9_Group_Out {
    int64_t auction_id = 0;
    int64_t winning_price = 0;
    uint64_t win_id = 0;

    // Costruttori imposti dall'architettura WindFlow KeyedWindows
    Q9_Group_Out() = default;
    Q9_Group_Out(uint64_t _id) : win_id(_id) {}
    Q9_Group_Out(const Q9_Key& _key, uint64_t _id) 
        : auction_id(_key.auction_id), win_id(_id) {}
};

template <typename InputT, typename OutputT>
class Bid_Unifier {
public:
    void operator()(const InputT& in, wf::Shipper<OutputT>& shipper) {
        OutputT out;
        out.auction_id = in.auction_id;
        out.price = in.price;
        out.bid_dateTime = in.bid_dateTime;
        shipper.push(out);
    }
};

template <typename InputT, typename OutputT>
class Auction_Unifier {
public:
    void operator()(const InputT& in, wf::Shipper<OutputT>& shipper) {
        OutputT out;
        out.auction_id = in.auction_id;
        out.auction_dateTime = in.auction_dateTime;
        out.expires = in.expires;
        shipper.push(out);
    }
};

class Q9_Join_Logic {
public:
    std::optional<Q9_Join_Out> operator()(const Q9_Unified& left, const Q9_Unified& right) {
        if (right.auction_dateTime < left.bid_dateTime && left.bid_dateTime < right.expires) {
            Q9_Join_Out out;
            out.auction_id = left.auction_id;
            out.price = left.price;
            return out; 
        }
        return std::nullopt; 
    }
};

class Q9_Window_Logic {
public:
    void operator()(const Q9_Join_Out& in, Q9_Group_Out& out) {        
        // Logica di Max Aggregation
        if (in.price > out.winning_price) {
            out.winning_price = in.price;
        }
    }
};

int main(int argc, char* argv[]) {
    uint64_t q9_epoch = parse_ISO8601("2026-09-01T00:00:00.000Z");

    // --- 1. SORGENTI ---
    auto bid_src = Table_Source_Builder<Bid>(
            "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_datasets/30m_bid.csv", 
            [](const std::string& line, Bid& record, uint64_t& timestamp) {
                std::stringstream ss(line);
                std::string token;

                std::getline(ss, token, ',');
                record.auction_id = parse_BIGINT(token);

                std::getline(ss, token, ',');
                record.bidder = parse_BIGINT(token);

                std::getline(ss, token, ',');
                record.price = parse_BIGINT(token);

                std::getline(ss, token, ',');
                record.channel = parse_STRING(token);

                std::getline(ss, token, ',');
                record.url = parse_STRING(token);

                std::getline(ss, token, ',');
                record.bid_dateTime = parse_ISO8601(token);
                timestamp = parse_ISO8601(token);

                std::getline(ss, token, ',');
                record.extra = parse_STRING(token);

            }
        )
        .withName("bid_source")
        .withHeader()
        .withParallelism(2, 134217728ULL) 
        .withOrderedEventTime(q9_epoch)
        .build();

    auto auction_src = Table_Source_Builder<Auction>(
            "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_datasets/30m_auction.csv", 
            [](const std::string& line, Auction& record, uint64_t& timestamp) {
                std::stringstream ss(line);
                std::string token;

                std::getline(ss, token, ',');
                record.auction_id = parse_BIGINT(token);

                std::getline(ss, token, ',');
                record.item_name = parse_STRING(token);

                std::getline(ss, token, ',');
                record.description = parse_STRING(token);

                std::getline(ss, token, ',');
                record.initial_bid = parse_BIGINT(token);

                std::getline(ss, token, ',');
                record.reserve = parse_BIGINT(token);

                std::getline(ss, token, ',');
                record.auction_dateTime = parse_ISO8601(token);
                timestamp = parse_ISO8601(token);

                std::getline(ss, token, ',');
                record.expires = parse_ISO8601(token);

                std::getline(ss, token, ',');
                record.seller = parse_BIGINT(token);

                std::getline(ss, token, ',');
                record.category = parse_BIGINT(token);

            }
        )
        .withName("auction_source")
        .withHeader()
        .withParallelism(2, 8388608ULL) //8MB
        .withOrderedEventTime(q9_epoch)
        .build();

    Bid_Unifier<Bid, Q9_Unified> b_unifier;
    auto bid_unifier = wf::FlatMap_Builder(b_unifier)
        .withName("bid_unify")
        .withParallelism(2)
        .build();

    Auction_Unifier<Auction, Q9_Unified> a_unifier;
    auto auction_unifier = wf::FlatMap_Builder(a_unifier)
        .withName("auction_unify")
        .withParallelism(2)
        .build();

    Q9_Join_Logic join_logic;
    auto interval_join = wf::Interval_Join_Builder(join_logic)
        .withName("q9_interval_join")
        .withParallelism(2)
        .withKeyBy([](const Q9_Unified& in) -> int64_t { return in.auction_id; })
        .withKPMode()
        .withBoundaries(std::chrono::hours(-24), std::chrono::hours(1))
        .build();

    Q9_Window_Logic win_logic;
    auto window_group = wf::Keyed_Windows_Builder(win_logic)
        .withName("q9_native_keyed_windows")
        .withParallelism(2)
        .withKeyBy([](const Q9_Join_Out& in) -> Q9_Key { return Q9_Key{in.auction_id}; })
        .withTBWindows(std::chrono::hours(48), std::chrono::hours(48))
        .build();

    
    auto sink = Table_Sink_Builder<Q9_Group_Out>(
            "q9_output",
            [](const Q9_Group_Out& record, std::ostream& os) {
                os << record.auction_id << "," << record.winning_price;
            }
        )
        .withName("q9_sink")
        .withHeader("auction_id,winning_price")
        .withParallelism(2)
        .build();

    // --- 6. TOPOLOGIA ---
    wf::PipeGraph topology("q9", wf::Execution_Mode_t::DEFAULT, wf::Time_Policy_t::EVENT_TIME);

    auto& pipe_1 = topology.add_source(bid_src).add(bid_unifier);
    auto& pipe_2 = topology.add_source(auction_src).add(auction_unifier);
    
    std::vector<wf::MultiPipe*> branches = {&pipe_2, &pipe_1};
    auto* merged = wf::merge_multipipes_func(&topology, branches);
    
    merged->add(interval_join).add(window_group).add_sink(sink);

    topology.run();
    return 0;
}