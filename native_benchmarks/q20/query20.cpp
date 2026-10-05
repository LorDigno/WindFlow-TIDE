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

// 1. Struct Unificato per la Interval Join
struct Q20_Unified {
    int64_t auction_id = 0; 
    
    // Campi Bid
    int64_t bidder = 0;
    int64_t price = 0;
    std::string channel;
    std::string url;
    uint64_t bid_dateTime = 0;
    std::string extra;
    
    // Campi Auction
    std::string item_name;
    std::string description;
    int64_t initial_bid = 0;
    int64_t reserve = 0;
    uint64_t auction_dateTime = 0;
    uint64_t expires = 0;
    int64_t seller = 0;
    int64_t category = 0;
};

// 2. Struct di Output Proiettato
struct Q20_Out {
    int64_t auction_id = 0;
    int64_t bidder = 0;
    int64_t price = 0;
    std::string channel;
    std::string url;
    uint64_t bid_dateTime = 0;
    std::string extra;
    std::string item_name;
    std::string description;
    int64_t initial_bid = 0;
    int64_t reserve = 0;
    uint64_t auction_dateTime = 0;
    uint64_t expires = 0;
    int64_t seller = 0;
    int64_t category = 0;
};

// --- LOGICA DI JOIN NATIVA ---
class Q20_Join_Logic {
public:
    std::optional<Q20_Out> operator()(const Q20_Unified& left, const Q20_Unified& right) {
        Q20_Out out;
        out.auction_id = left.auction_id;
        
        // Popolamento campi dal ramo sx (Auction)
        out.item_name = left.item_name;
        out.description = left.description;
        out.initial_bid = left.initial_bid;
        out.reserve = left.reserve;
        out.auction_dateTime = left.auction_dateTime;
        out.expires = left.expires;
        out.seller = left.seller;
        out.category = left.category;
        
        // Popolamento campi dal ramo dx (Bid)
        out.bidder = right.bidder;
        out.price = right.price;
        out.channel = right.channel;
        out.url = right.url;
        out.bid_dateTime = right.bid_dateTime;
        out.extra = right.extra;
        
        return out;
    }
};

int main(int argc, char* argv[]) {
    uint64_t q20_epoch = parse_ISO8601("2026-09-01T00:00:00.000Z");

    // --- 1. SORGENTI (Chunk allineati a 1MB per stabilità watermark) ---
    auto auction_src = Table_Source_Builder<Auction>(
            "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_datasets/70m_auction.csv", 
            [](const std::string& line, Auction& record, uint64_t& timestamp) {
                std::stringstream ss(line);
                std::string token;
                std::getline(ss, token, ','); record.auction_id = parse_BIGINT(token);
                std::getline(ss, token, ','); record.item_name = parse_STRING(token);
                std::getline(ss, token, ','); record.description = parse_STRING(token);
                std::getline(ss, token, ','); record.initial_bid = parse_BIGINT(token);
                std::getline(ss, token, ','); record.reserve = parse_BIGINT(token);
                std::getline(ss, token, ','); record.auction_dateTime = parse_ISO8601(token);
                timestamp = record.auction_dateTime; 
                std::getline(ss, token, ','); record.expires = parse_ISO8601(token);
                std::getline(ss, token, ','); record.seller = parse_BIGINT(token);
                std::getline(ss, token, ','); record.category = parse_BIGINT(token);
            }
        )
        .withName("auction_source")
        .withHeader()
        .withParallelism(3, 8388608ULL)
        .withOrderedEventTime(q20_epoch)
        .build();

    auto bid_src = Table_Source_Builder<Bid>(
            "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_datasets/70m_bid.csv", 
            [](const std::string& line, Bid& record, uint64_t& timestamp) {
                std::stringstream ss(line);
                std::string token;
                std::getline(ss, token, ','); record.auction_id = parse_BIGINT(token);
                std::getline(ss, token, ','); record.bidder = parse_BIGINT(token);
                std::getline(ss, token, ','); record.price = parse_BIGINT(token);
                std::getline(ss, token, ','); record.channel = parse_STRING(token);
                std::getline(ss, token, ','); record.url = parse_STRING(token);
                std::getline(ss, token, ','); record.bid_dateTime = parse_ISO8601(token);
                timestamp = record.bid_dateTime;
                std::getline(ss, token, ','); record.extra = parse_STRING(token);
            }
        )
        .withName("bid_source")
        .withHeader()
        .withParallelism(3, 67108864ULL)
        .withOrderedEventTime(q20_epoch)
        .build();

    // --- 2. OPERATORI ISOLATI ---
    
    // A. Filtro puro sul ramo Auction
    auto auction_filter = Where_Builder<Auction>(
        [](const Auction& in) -> bool {
            return in.category == 10;
        }
    )
    .withName("auction_filter")
    .withParallelism(3)
    .build();

    // B. Proiezione sul ramo Auction (scatta solo sui dati sopravvissuti)
    auto auction_unifier = Select_Builder<Auction, Q20_Unified>(
        [](const Auction& in) -> Q20_Unified {
            Q20_Unified out;
            out.auction_id = in.auction_id;
            out.item_name = in.item_name;
            out.description = in.description;
            out.initial_bid = in.initial_bid;
            out.reserve = in.reserve;
            out.auction_dateTime = in.auction_dateTime;
            out.expires = in.expires;
            out.seller = in.seller;
            out.category = in.category;
            return out;
        }
    )
    .withName("auction_unifier")
    .withParallelism(3)
    .build();

    // C. Proiezione sul ramo Bid (non necessita di filtro per questa query)
    auto bid_unifier = Select_Builder<Bid, Q20_Unified>(
        [](const Bid& in) -> Q20_Unified {
            Q20_Unified out;
            out.auction_id = in.auction_id;
            out.bidder = in.bidder;
            out.price = in.price;
            out.channel = in.channel;
            out.url = in.url;
            out.bid_dateTime = in.bid_dateTime;
            out.extra = in.extra;
            return out;
        }
    )
    .withName("bid_unifier")
    .withParallelism(3)
    .build();

    // --- 3. INTERVAL JOIN NATIVA ---
    Q20_Join_Logic join_logic;
    auto interval_join = wf::Interval_Join_Builder(join_logic)
        .withName("q20_native_interval_join")
        .withParallelism(3)
        .withKeyBy([](const Q20_Unified& in) -> int64_t { return in.auction_id; })
        .withKPMode()
        .withBoundaries(std::chrono::minutes(-1), std::chrono::hours(12))
        .build();

    // --- 4. SINK ---
    auto sink = Table_Sink_Builder<Q20_Out>("q20_output",
            [](const Q20_Out& r, std::ostream& os) {
                os << r.auction_id << "," << r.bidder << "," << r.price << "," 
                   << r.channel << "," << r.url << "," << r.bid_dateTime << "," 
                   << r.extra << "," << r.item_name << "," << r.description << "," 
                   << r.initial_bid << "," << r.reserve << "," << r.auction_dateTime << "," 
                   << r.expires << "," << r.seller << "," << r.category;
            }
        )
        .withName("q20_sink")
        .withHeader("auction_id,bidder,price,channel,url,bid_dateTime,extra,item_name,description,initial_bid,reserve,auction_dateTime,expires,seller,category")
        .withParallelism(3)
        .build();

    // --- 5. TOPOLOGIA ---
    wf::PipeGraph topology("q20", wf::Execution_Mode_t::DEFAULT, wf::Time_Policy_t::EVENT_TIME);

    // Connessione con scomposizione logica
    auto& pipe_1 = topology.add_source(auction_src).add(auction_filter).add(auction_unifier);
    auto& pipe_2 = topology.add_source(bid_src).add(bid_unifier);
    
    std::vector<wf::MultiPipe*> branches = {&pipe_1, &pipe_2};
    auto* merged = wf::merge_multipipes_func(&topology, branches);
    
    merged->add(interval_join).add_sink(sink);

    topology.run();
    return 0;
}