#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <map>
#include <unordered_set>
#include <windflow.hpp>
#include <windflow_table_api.hpp>
#include "nexmark_streams.hpp"

//struct di output della select
struct Q2_Out {
    int64_t auction_id = 0;
    int64_t price = 0;
};

//flatmap con operator fusion di select e where
template <typename InputT, typename OutputT>
class Q2_FlatMap_Functor {
public:
    Q2_FlatMap_Functor() = default;

    void operator()(const InputT& in, wf::Shipper<OutputT>& shipper) {
        
        //filtro
        if (in.auction_id == 1007 || 
            in.auction_id == 1020 || 
            in.auction_id == 2001 || 
            in.auction_id == 2019 || 
            in.auction_id == 2087) {
            
            //select
            OutputT out;
            out.auction_id = in.auction_id;
            out.price = in.price;
            
            shipper.push(out);
        }
    }
};

int main() {
    //variabile di epoch per la normalizzazione dei timestamp
    uint64_t specific_id_epoch = parse_ISO8601("2026-09-01T00:00:00.000Z");

    auto from_1_op = Table_Source_Builder<Bid>( 
            "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_datasets/70m_bid.csv",
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
        .withName("sprcific_id_bid_source")
        .withHeader()
        .withParallelism(5, 33554432ULL)  
        .withOrderedEventTime(specific_id_epoch)
        .build();

    Q2_FlatMap_Functor<Bid, Q2_Out> flatmap_logic;
    auto q2_op = wf::FlatMap_Builder(flatmap_logic)
        .withName("q2_specific_id_filter_select")
        .withParallelism(5) 
        .build();

    auto sink_4_op = Table_Sink_Builder<Q2_Out>(
            "specific_id",
            [](const Q2_Out& record, std::ostream& os) {
                os << record.auction_id << ",";
                os << record.price;
            }
        )
        .withName("specific_id_sink")
        .withParallelism(5)
        .withHeader("auction_id,price")
        .build();

    wf::PipeGraph topology(
        "specific_id", 
        wf::Execution_Mode_t::DEFAULT
        , wf::Time_Policy_t::EVENT_TIME 
    );

    auto& pipe_0 = topology.add_source(from_1_op).add(q2_op).add_sink(sink_4_op);

    topology.run();
    return 0;
}