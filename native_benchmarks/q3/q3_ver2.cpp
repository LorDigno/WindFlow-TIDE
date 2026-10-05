#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <chrono>
#include <map>
#include <unordered_set>
#include <windflow.hpp>
#include <windflow_table_api.hpp>
#include "nexmark_streams.hpp"

// Struct Unificato per la Interval Join
struct Q3_Unified {
    int64_t person_id = 0; // Join Key
    std::string name;
    std::string city;
    std::string state;
    int64_t auction_id = 0;
};

// Struct di Output Proiettato
struct Q3_Out {
    std::string name;
    std::string city;
    std::string state;
    int64_t auction_id = 0;
};

int main() {
    // Variabile di epoch per la normalizzazione dei timestamp
    uint64_t q3_epoch = parse_ISO8601("2026-09-01T00:00:00.000Z");

    // ==========================================
    // RAMO 1: PERSON
    // ==========================================
    auto from_person = Table_Source_Builder<Person>( 
        "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_datasets/70m_person.csv",
        [](const std::string& line, Person& record, uint64_t& timestamp) {
            std::stringstream ss(line);
            std::string token;

            std::getline(ss, token, ','); record.person_id = parse_BIGINT(token);
            std::getline(ss, token, ','); record.name = parse_STRING(token);
            std::getline(ss, token, ','); record.email_address = parse_STRING(token);
            std::getline(ss, token, ','); record.credit_card = parse_STRING(token);
            std::getline(ss, token, ','); record.city = parse_STRING(token);
            std::getline(ss, token, ','); record.state = parse_STRING(token);
            std::getline(ss, token, ','); record.person_dateTime = parse_ISO8601(token); 
            timestamp = parse_ISO8601(token); // Metadato di sistema
            std::getline(ss, token, ','); record.extra = parse_STRING(token);
        }
    )
    .withName("person_source")
    .withHeader()
    .withParallelism(3, 8388608ULL)  
    .withOrderedEventTime(q3_epoch)
    .build();

    auto person_filter = Where_Builder<Person>(
        [](const Person& in) -> bool {
            return in.state == "OR" || in.state == "ID" || in.state == "CA";
        }
    )
    .withName("person_filter")
    .withParallelism(3)
    .build();

    auto person_unifier = Select_Builder<Person, Q3_Unified>(
        [](const Person& in) -> Q3_Unified {
            Q3_Unified out;
            out.person_id = in.person_id;
            out.name = in.name;
            out.city = in.city;
            out.state = in.state;
            return out;
        }
    )
    .withName("person_unifier")
    .withParallelism(3)
    .build();

    // ==========================================
    // RAMO 2: AUCTION
    // ==========================================
    auto from_auction = Table_Source_Builder<Auction>( 
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
            timestamp = parse_ISO8601(token); // Metadato di sistema
            std::getline(ss, token, ','); record.expires = parse_ISO8601(token);
            std::getline(ss, token, ','); record.seller = parse_BIGINT(token);
            std::getline(ss, token, ','); record.category = parse_BIGINT(token);
        }
    )
    .withName("auction_source")
    .withHeader()
    .withParallelism(3, 8388608ULL) 
    .withOrderedEventTime(q3_epoch)
    .build();

    auto auction_filter = Where_Builder<Auction>(
        [](const Auction& in) -> bool {
            return in.category == 10;
        }
    )
    .withName("auction_filter")
    .withParallelism(2)
    .build();

    auto auction_unifier = Select_Builder<Auction, Q3_Unified>(
        [](const Auction& in) -> Q3_Unified {
            Q3_Unified out;
            out.person_id = in.seller;      // Rename implicito per l'unificazione della chiave
            out.auction_id = in.auction_id;
            return out;
        }
    )
    .withName("auction_unifier")
    .withParallelism(2)
    .build();

    // ==========================================
    // JOIN E SINK
    // ==========================================
    
    // Intervallo in microsecondi (31 giorni)
    int64_t bound_us = 31LL * 24LL * 60LL * 60LL * 1000000LL; 

    auto join_op = wf::Interval_Join_Builder(
        [](const Q3_Unified& left, const Q3_Unified& right) -> std::optional<Q3_Out> {
            Q3_Out out;
            out.name = left.name;
            out.city = left.city;
            out.state = left.state;
            out.auction_id = right.auction_id;
            return out;
        }
    )
    .withName("q3_native_interval_join")
    .withParallelism(3)
    .withKeyBy([](const Q3_Unified& in) -> int64_t { return in.person_id; })
    .withKPMode()
    // Intervallo asimmetrico: da 0 (esatto momento di matching) a +31 giorni nel futuro
    .withBoundaries(
        std::chrono::microseconds(0), 
        std::chrono::microseconds(bound_us)
    )
    .build();

    auto sink_op = Table_Sink_Builder<Q3_Out>(
        "q3_output",
        [](const Q3_Out& record, std::ostream& os) {
            os << record.name << "," << record.city << "," << record.state << "," << record.auction_id;
        }
    )
    .withName("q3_sink")
    .withHeader("name,city,state,auction_id")
    .withParallelism(3)
    .build();

    // --- 5. COSTRUZIONE TOPOLOGIA ---
    wf::PipeGraph topology("q3_topology", wf::Execution_Mode_t::DEFAULT, wf::Time_Policy_t::EVENT_TIME);

    // Composizione pipeline a stadi isolati
    auto& pipe_1 = topology.add_source(from_person).add(person_filter).add(person_unifier);
    auto& pipe_2 = topology.add_source(from_auction).add(auction_filter).add(auction_unifier);
    
    // Fusione e join
    std::vector<wf::MultiPipe*> branches = {&pipe_1, &pipe_2};
    auto* merged_pipe = wf::merge_multipipes_func(&topology, branches);
    
    merged_pipe->add(join_op).add_sink(sink_op);

    topology.run();
    return 0;
}