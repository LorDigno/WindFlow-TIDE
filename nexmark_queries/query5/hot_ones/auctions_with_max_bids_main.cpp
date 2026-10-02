#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <chrono>
#include <windflow.hpp>
#include <windflow_table_api.hpp>
#include "auctions_with_max_bids_structs.hpp"

int main(int argc, char* argv[]) {
    //variabile di epoch per la normalizzazione dei timestamp
    uint64_t auctions_with_max_bids_epoch = parse_ISO8601("2026-09-01T00:00:00.000Z");

    //-----     OPERATOR BUILDERS   -----

    auto from_1_op = Table_Source_Builder<source_bids_counter_per_auction_from_6>( "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_datasets/10m_bid.csv",
    [](const std::string& line, source_bids_counter_per_auction_from_6& record, uint64_t& timestamp) {
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
    .withName("bids_counter_per_auction_from_6")
    .withHeader()
    .withParallelism(3, 33554432ULL)
    .withOrderedEventTime(auctions_with_max_bids_epoch)
    .build();

    auto window_group_2_op = Windowed_Group_Builder<source_bids_counter_per_auction_from_6, bids_counter_per_auction_window_group_by_5_struct_out, bids_counter_per_auction_window_group_by_5_key_struct>(
    [](const source_bids_counter_per_auction_from_6& in, bids_counter_per_auction_window_group_by_5_struct_out& out) -> void {
    out.auction_id = in.auction_id;

    out.COUNT += 1;
}
)
    .withName("bids_counter_per_auction_window_group_by_5")
    .withTBWindow(43200000000ULL, 43200000000ULL)
    .withParallelism(3)
    .withKeyBy([](const source_bids_counter_per_auction_from_6& in) -> bids_counter_per_auction_window_group_by_5_key_struct {
    bids_counter_per_auction_window_group_by_5_key_struct out;
    out.auction_id = in.auction_id;
    return out;
})
    .build_keyed();


    auto select_3_op = Select_Builder<bids_counter_per_auction_window_group_by_5_struct_out, bids_counter_per_auction_select_4_struct_out>(
        [](const bids_counter_per_auction_window_group_by_5_struct_out& in) -> bids_counter_per_auction_select_4_struct_out {
    bids_counter_per_auction_select_4_struct_out out;
    out.auction_id = in.auction_id;
    out.bids_counter = in.COUNT;
    return out;
}
    )
    .withName("select_3_op")
    .withParallelism(3)
    .build();

    auto select_4_op = Select_Builder<bids_counter_per_auction_select_4_struct_out, bids_counter_per_auction_query_4_select_3_struct_out>(
        [](const bids_counter_per_auction_select_4_struct_out& in) -> bids_counter_per_auction_query_4_select_3_struct_out {
    bids_counter_per_auction_query_4_select_3_struct_out out;
    out.auction_id = in.auction_id;
    out.maxxxxxx = in.bids_counter;
    return out;
}
    )
    .withName("select_4_op")
    .withParallelism(3)
    .build();

    auto from_5_op = Table_Source_Builder<source_bids_counter_per_auction_from_6>( "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_datasets/10m_bid.csv",
    [](const std::string& line, source_bids_counter_per_auction_from_6& record, uint64_t& timestamp) {
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
    .withName("bids_counter_per_auction_from_6")
    .withHeader()
    .withParallelism(3, 33554432ULL)
    .withOrderedEventTime(auctions_with_max_bids_epoch)
    .build();

    auto window_group_6_op = Windowed_Group_Builder<source_bids_counter_per_auction_from_6, bids_counter_per_auction_window_group_by_5_struct_out, bids_counter_per_auction_window_group_by_5_key_struct>(
    [](const source_bids_counter_per_auction_from_6& in, bids_counter_per_auction_window_group_by_5_struct_out& out) -> void {
    out.auction_id = in.auction_id;

    out.COUNT += 1;
}
)
    .withName("bids_counter_per_auction_window_group_by_5")
    .withTBWindow(43200000000ULL, 43200000000ULL)
    .withParallelism(3)
    .withKeyBy([](const source_bids_counter_per_auction_from_6& in) -> bids_counter_per_auction_window_group_by_5_key_struct {
    bids_counter_per_auction_window_group_by_5_key_struct out;
    out.auction_id = in.auction_id;
    return out;
})
    .build_keyed();


    auto select_7_op = Select_Builder<bids_counter_per_auction_window_group_by_5_struct_out, bids_counter_per_auction_select_4_struct_out>(
        [](const bids_counter_per_auction_window_group_by_5_struct_out& in) -> bids_counter_per_auction_select_4_struct_out {
    bids_counter_per_auction_select_4_struct_out out;
    out.auction_id = in.auction_id;
    out.bids_counter = in.COUNT;
    return out;
}
    )
    .withName("select_7_op")
    .withParallelism(3)
    .build();

    auto window_group_8_op = Windowed_Group_Builder<bids_counter_per_auction_select_4_struct_out, max_bid_count_window_group_by_8_struct_out>(
    [](const bids_counter_per_auction_select_4_struct_out& in, max_bid_count_window_group_by_8_struct_out& out) -> void {

    auto MAX_bids_counter_tmp = in.bids_counter; 
if( MAX_bids_counter_tmp > out.MAX_bids_counter ){
    out.MAX_bids_counter = MAX_bids_counter_tmp;
}
}
)
    .withName("max_bid_count_window_group_by_8")
    .withTBWindow(43200000000ULL, 43200000000ULL)
    .build();


    auto select_9_op = Select_Builder<max_bid_count_window_group_by_8_struct_out, max_bid_count_select_7_struct_out>(
        [](const max_bid_count_window_group_by_8_struct_out& in) -> max_bid_count_select_7_struct_out {
    max_bid_count_select_7_struct_out out;
    out.maxxxxxx = in.MAX_bids_counter;
    return out;
}
    )
    .withName("select_9_op")
    .withParallelism(3)
    .build();

    auto left_unifier_10_op = Select_Builder<bids_counter_per_auction_query_4_select_3_struct_out, bids_counter_per_auction_query_4_select_3_struct_out>(
        [](const bids_counter_per_auction_query_4_select_3_struct_out& in) -> bids_counter_per_auction_query_4_select_3_struct_out {
    bids_counter_per_auction_query_4_select_3_struct_out out;
    out.auction_id = in.auction_id;
    out.maxxxxxx = in.maxxxxxx;
    return out;
}
    )
    .withName("left_unifier_10_op")
    .withParallelism(3)
    .build();

    auto right_unifier_11_op = Select_Builder<max_bid_count_select_7_struct_out, bids_counter_per_auction_query_4_select_3_struct_out>(
        [](const max_bid_count_select_7_struct_out& in) -> bids_counter_per_auction_query_4_select_3_struct_out {
    bids_counter_per_auction_query_4_select_3_struct_out out;
    out.maxxxxxx = in.maxxxxxx;
    return out;
}
    )
    .withName("right_unifier_11_op")
    .withParallelism(3)
    .build();

    auto join_12_op = Table_Interval_Join_Builder<bids_counter_per_auction_query_4_select_3_struct_out, bids_counter_per_auction_query_4_select_3_struct_out, max_bid_count_select_7_struct_out>(
    [](const bids_counter_per_auction_query_4_select_3_struct_out& left, const bids_counter_per_auction_query_4_select_3_struct_out& right) -> std::optional<bids_counter_per_auction_query_4_select_3_struct_out> {

    bids_counter_per_auction_query_4_select_3_struct_out out{};
    out.auction_id = left.auction_id;
    out.maxxxxxx = left.maxxxxxx;
    return out;
},
    -3600000000,
    46800000000
)
    .withName("auctions_with_max_bids_join_interval_2")
    .withParallelism(3)
    .withKeyBy([](const bids_counter_per_auction_query_4_select_3_struct_out& in) -> max_bid_count_select_7_struct_out {
    max_bid_count_select_7_struct_out out;
    out.maxxxxxx = in.maxxxxxx;
    return out;
})
    .build_keyed();


    auto sink_13_op = Table_Sink_Builder<bids_counter_per_auction_query_4_select_3_struct_out>("auctions_with_max_bids",
    [](const bids_counter_per_auction_query_4_select_3_struct_out& record, std::ostream& os) {
 
    os << record.auction_id << ",";
 
    os << record.maxxxxxx;
}
)
    .withName("auctions_with_max_bids_sink_9")
    .withParallelism(3)
    .withHeader("auction_id,maxxxxxx")
    .build();

    //-----     PIPES AND TOPOLOGY  ------
    wf::PipeGraph topology(
        "auctions_with_max_bids", 
        wf::Execution_Mode_t::DEFAULT
        , wf::Time_Policy_t::EVENT_TIME 
    );

    auto& pipe_1 = topology.add_source(from_1_op).add(window_group_2_op).add(select_3_op).add(select_4_op).add(left_unifier_10_op);

    auto& pipe_2 = topology.add_source(from_5_op).add(window_group_6_op).add(select_7_op).add(window_group_8_op).add(select_9_op).add(right_unifier_11_op);

    std::vector<wf::MultiPipe*> pipe_0_branches = {&pipe_1, &pipe_2};
auto* pipe_0_pointer = wf::merge_multipipes_func(&topology, pipe_0_branches);
auto& pipe_0 = (*pipe_0_pointer).add(join_12_op).add_sink(sink_13_op);

    topology.run();
    return 0;
}