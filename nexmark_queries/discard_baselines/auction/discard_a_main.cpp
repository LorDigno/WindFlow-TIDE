#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <chrono>
#include <windflow.hpp>
#include <windflow_table_api.hpp>
#include "discard_a_structs.hpp"

int main(int argc, char* argv[]) {
    //variabile di epoch per la normalizzazione dei timestamp
    uint64_t discard_a_epoch = parse_ISO8601("2026-09-01T00:00:00.000Z");

    //-----     OPERATOR BUILDERS   -----

    auto from_1_op = Table_Source_Builder<source_discard_a_from_2>( "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_datasets/70m_auction.csv",
    [](const std::string& line, source_discard_a_from_2& record, uint64_t& timestamp) {
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
    .withName("discard_a_from_2")
    .withHeader()
    .withParallelism(2, 67108864ULL)
    .withOrderedEventTime(discard_a_epoch)
    .build();

    auto sink_2_op = Table_Sink_Builder<source_discard_a_from_2>("discard_a",
    [](const source_discard_a_from_2& record, std::ostream& os) {
 
    os << record.auction_id << ",";
 
    os << record.item_name << ",";
 
    os << record.description << ",";
 
    os << record.initial_bid << ",";
 
    os << record.reserve << ",";
 
    os << reformat_ISO8601(record.auction_dateTime) << ",";
 
    os << reformat_ISO8601(record.expires) << ",";
 
    os << record.seller << ",";
 
    os << record.category;
}
)
    .withName("discard_a_sink_3")
    .withParallelism(2)
    .withHeader("auction_id,item_name,description,initial_bid,reserve,auction_dateTime,expires,seller,category")
    .build();

    //-----     PIPES AND TOPOLOGY  ------
    wf::PipeGraph topology(
        "discard_a", 
        wf::Execution_Mode_t::DEFAULT
        , wf::Time_Policy_t::EVENT_TIME 
    );

    auto& pipe_0 = topology.add_source(from_1_op).add_sink(sink_2_op);

    topology.run();
    return 0;
}