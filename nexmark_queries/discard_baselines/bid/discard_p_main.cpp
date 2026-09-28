#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <chrono>
#include <windflow.hpp>
#include <windflow_table_api.hpp>
#include "discard_p_structs.hpp"

int main(int argc, char* argv[]) {
    //variabile di epoch per la normalizzazione dei timestamp
    uint64_t discard_p_epoch = parse_ISO8601("2026-09-01T00:00:00.000Z");

    //-----     OPERATOR BUILDERS   -----

    auto from_1_op = Table_Source_Builder<source_discard_p_from_2>( "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_datasets/70m_bid.csv",
    [](const std::string& line, source_discard_p_from_2& record, uint64_t& timestamp) {
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
    .withName("discard_p_from_2")
    .withHeader()
    .withParallelism(2, 134217728ULL)
    .withOrderedEventTime(discard_p_epoch)
    .build();

    auto sink_2_op = Table_Sink_Builder<source_discard_p_from_2>("discard_p",
    [](const source_discard_p_from_2& record, std::ostream& os) {
 
    os << record.auction_id << ",";
 
    os << record.bidder << ",";
 
    os << record.price << ",";
 
    os << record.channel << ",";
 
    os << record.url << ",";
 
    os << reformat_ISO8601(record.bid_dateTime) << ",";
 
    os << record.extra;
}
)
    .withName("discard_p_sink_3")
    .withParallelism(2)
    .withHeader("auction_id,bidder,price,channel,url,bid_dateTime,extra")
    .build();

    //-----     PIPES AND TOPOLOGY  ------
    wf::PipeGraph topology(
        "discard_p", 
        wf::Execution_Mode_t::DEFAULT
        , wf::Time_Policy_t::EVENT_TIME 
    );

    auto& pipe_0 = topology.add_source(from_1_op).add_sink(sink_2_op);

    topology.run();
    return 0;
}