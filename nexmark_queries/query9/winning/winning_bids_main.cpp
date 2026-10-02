#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <chrono>
#include <windflow.hpp>
#include <windflow_table_api.hpp>
#include "winning_bids_structs.hpp"

int main(int argc, char* argv[]) {
    //variabile di epoch per la normalizzazione dei timestamp
    uint64_t winning_bids_epoch = parse_ISO8601("2026-09-01T00:00:00.000Z");

    //-----     OPERATOR BUILDERS   -----

    auto from_1_op = Table_Source_Builder<source_winning_bids_from_4>( "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_datasets/30m_bid.csv",
    [](const std::string& line, source_winning_bids_from_4& record, uint64_t& timestamp) {
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
    .withName("winning_bids_from_4")
    .withHeader()
    .withParallelism(3, 67108864ULL)
    .withOrderedEventTime(winning_bids_epoch)
    .build();

    auto from_2_op = Table_Source_Builder<source_winning_bids_from_5>( "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_datasets/30m_auction.csv",
    [](const std::string& line, source_winning_bids_from_5& record, uint64_t& timestamp) {
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
    .withName("winning_bids_from_5")
    .withHeader()
    .withParallelism(3, 16777216ULL)
    .withOrderedEventTime(winning_bids_epoch)
    .build();

    auto left_unifier_3_op = Select_Builder<source_winning_bids_from_4, source_winning_bids_from_4_unified_source_winning_bids_from_5>(
        [](const source_winning_bids_from_4& in) -> source_winning_bids_from_4_unified_source_winning_bids_from_5 {
    source_winning_bids_from_4_unified_source_winning_bids_from_5 out;
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
    .withName("left_unifier_3_op")
    .withParallelism(3)
    .build();

    auto right_unifier_4_op = Select_Builder<source_winning_bids_from_5, source_winning_bids_from_4_unified_source_winning_bids_from_5>(
        [](const source_winning_bids_from_5& in) -> source_winning_bids_from_4_unified_source_winning_bids_from_5 {
    source_winning_bids_from_4_unified_source_winning_bids_from_5 out;
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
    .withName("right_unifier_4_op")
    .withParallelism(3)
    .build();

    auto join_5_op = Table_Interval_Join_Builder<source_winning_bids_from_4_unified_source_winning_bids_from_5, source_winning_bids_from_4_unified_source_winning_bids_from_5, winning_bids_join_interval_3_key_struct>(
    [](const source_winning_bids_from_4_unified_source_winning_bids_from_5& left, const source_winning_bids_from_4_unified_source_winning_bids_from_5& right) -> std::optional<source_winning_bids_from_4_unified_source_winning_bids_from_5> {
    if( !(((right.auction_dateTime < left.bid_dateTime) && (left.bid_dateTime < right.expires)))){
        return std::nullopt;
    }

    source_winning_bids_from_4_unified_source_winning_bids_from_5 out{};
    out.auction_id = left.auction_id;
    out.bidder = left.bidder;
    out.price = left.price;
    out.channel = left.channel;
    out.url = left.url;
    out.bid_dateTime = left.bid_dateTime;
    out.extra = left.extra;
    out.item_name = right.item_name;
    out.description = right.description;
    out.initial_bid = right.initial_bid;
    out.reserve = right.reserve;
    out.auction_dateTime = right.auction_dateTime;
    out.expires = right.expires;
    out.seller = right.seller;
    out.category = right.category;
    return out;
},
    -86400000000,
    3600000000
)
    .withName("winning_bids_join_interval_3")
    .withParallelism(3)
    .withKeyBy([](const source_winning_bids_from_4_unified_source_winning_bids_from_5& in) -> winning_bids_join_interval_3_key_struct {
    winning_bids_join_interval_3_key_struct out;
    out.auction_id = in.auction_id;
    return out;
})
    .build_keyed();


    auto window_group_6_op = Windowed_Group_Builder<source_winning_bids_from_4_unified_source_winning_bids_from_5, winning_bids_window_group_by_2_struct_out, winning_bids_join_interval_3_key_struct>(
    [](const source_winning_bids_from_4_unified_source_winning_bids_from_5& in, winning_bids_window_group_by_2_struct_out& out) -> void {
    out.auction_id = in.auction_id;

    auto MAX_price_tmp = in.price; 
if( MAX_price_tmp > out.MAX_price ){
    out.MAX_price = MAX_price_tmp;
}
}
)
    .withName("winning_bids_window_group_by_2")
    .withTBWindow(172800000000ULL, 172800000000ULL)
    .withParallelism(3)
    .withKeyBy([](const source_winning_bids_from_4_unified_source_winning_bids_from_5& in) -> winning_bids_join_interval_3_key_struct {
    winning_bids_join_interval_3_key_struct out;
    out.auction_id = in.auction_id;
    return out;
})
    .build_keyed();


    auto select_7_op = Select_Builder<winning_bids_window_group_by_2_struct_out, winning_bids_select_1_struct_out>(
        [](const winning_bids_window_group_by_2_struct_out& in) -> winning_bids_select_1_struct_out {
    winning_bids_select_1_struct_out out;
    out.auction_id = in.auction_id;
    out.winning_price = in.MAX_price;
    return out;
}
    )
    .withName("select_7_op")
    .withParallelism(3)
    .build();

    auto sink_8_op = Table_Sink_Builder<winning_bids_select_1_struct_out>("winning_bids",
    [](const winning_bids_select_1_struct_out& record, std::ostream& os) {
 
    os << record.auction_id << ",";
 
    os << record.winning_price;
}
)
    .withName("winning_bids_sink_6")
    .withParallelism(3)
    .withHeader("auction_id,winning_price")
    .build();

    //-----     PIPES AND TOPOLOGY  ------
    wf::PipeGraph topology(
        "winning_bids", 
        wf::Execution_Mode_t::DEFAULT
        , wf::Time_Policy_t::EVENT_TIME 
    );

    auto& pipe_1 = topology.add_source(from_1_op).add(left_unifier_3_op);

    auto& pipe_2 = topology.add_source(from_2_op).add(right_unifier_4_op);

    std::vector<wf::MultiPipe*> pipe_0_branches = {&pipe_1, &pipe_2};
auto* pipe_0_pointer = wf::merge_multipipes_func(&topology, pipe_0_branches);
auto& pipe_0 = (*pipe_0_pointer).add(join_5_op).add(window_group_6_op).add(select_7_op).add_sink(sink_8_op);

    topology.run();
    return 0;
}