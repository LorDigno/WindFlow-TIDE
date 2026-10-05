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


// Struct Unificato obbligatorio per l'Interval Join
struct Q3_Unified {
    int64_t person_id = 0; // Join Key
    std::string name;
    std::string city;
    std::string state;
    int64_t auction_id = 0;
};

// Struct della Chiave
struct Q3_Key {
    int64_t person_id = 0;
};

// Struct di Output Proiettato
struct Q3_Out {
    std::string name;
    std::string city;
    std::string state;
    int64_t auction_id = 0;
};

// Ramo 1: Filter su Person e unificazione
template <typename InputT, typename OutputT>
class Person_Filter_Unifier {
public:
    Person_Filter_Unifier() = default;

    void operator()(const InputT& in, wf::Shipper<OutputT>& shipper) {
        if (in.state == "OR" || in.state == "ID" || in.state == "CA") {
            OutputT out;
            out.person_id = in.person_id;
            out.name = in.name;
            out.city = in.city;
            out.state = in.state;
            shipper.push(out);
        }
    }
};

// Ramo 2: Filter su Auction e unificazione
template <typename InputT, typename OutputT>
class Auction_Filter_Unifier {
public:
    Auction_Filter_Unifier() = default;

    void operator()(const InputT& in, wf::Shipper<OutputT>& shipper) {
        if (in.category == 10) {
            OutputT out;
            out.person_id = in.seller;      // Rename implicito
            out.auction_id = in.auction_id;
            shipper.push(out);
        }
    }
};

// 4. Logica Nativa per wf::Interval_Join_Builder (restituisce std::optional)
class Q3_Join_Logic {
public:
    std::optional<Q3_Out> operator()(const Q3_Unified& left, const Q3_Unified& right) {
        Q3_Out out;
        // Accesso posizionale: left conterrà i dati dal primo ramo (Person), right dal secondo (Auction)
        out.name = left.name;
        out.city = left.city;
        out.state = left.state;
        out.auction_id = right.auction_id;
        return out;
    }
};

int main() {
    //variabile di epoch per la normalizzazione dei timestamp
    uint64_t selling_in_states_epoch = parse_ISO8601("2026-09-01T00:00:00.000Z");

    auto from_person = Table_Source_Builder<Person>( 
        "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_datasets/70m_person.csv",
        [](const std::string& line, Person& record, uint64_t& timestamp) {
            std::stringstream ss(line);
            std::string token;

            std::getline(ss, token, ',');
            record.person_id = parse_BIGINT(token);

            std::getline(ss, token, ',');
            record.name = parse_STRING(token);

            std::getline(ss, token, ',');
            record.email_address = parse_STRING(token);

            std::getline(ss, token, ',');
            record.credit_card = parse_STRING(token);

            std::getline(ss, token, ',');
            record.city = parse_STRING(token);

            std::getline(ss, token, ',');
            record.state = parse_STRING(token);

            std::getline(ss, token, ',');
            record.person_dateTime = parse_ISO8601(token);
            timestamp = parse_ISO8601(token);

            std::getline(ss, token, ',');
            record.extra = parse_STRING(token);

        }
        )
        .withName("person_source")
        .withHeader()
        .withParallelism(2, 16777216ULL)    
        .withOrderedEventTime(selling_in_states_epoch)
        .build();

    auto from_auction = Table_Source_Builder<Auction>( 
        "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_datasets/70m_auction.csv",
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
        .withOrderedEventTime(selling_in_states_epoch)
        .build();

    //flatmap 
    Person_Filter_Unifier<Person, Q3_Unified> p_logic;
    auto person_flatmap_op = wf::FlatMap_Builder(p_logic)
        .withName("person_filter_unify")
        .withParallelism(2)
        .build();

    Auction_Filter_Unifier<Auction, Q3_Unified> a_logic;
    auto auction_flatmap_op = wf::FlatMap_Builder(a_logic)
        .withName("auction_filter_unify")
        .withParallelism(2)
        .build();

    //interval join
    Q3_Join_Logic join_logic;
    
    //inytervallo in microsecondi (+/- 31 giorni)
    int64_t bound_us = 31LL * 24LL * 60LL * 60LL * 1000000LL; 
 
    auto join_op = wf::Interval_Join_Builder(join_logic)
        .withName("q3_native_interval_join")
        .withParallelism(2)
        .withKeyBy([](const Q3_Unified& in) -> int64_t { return in.person_id; })
        .withKPMode()
        .withBoundaries(
            std::chrono::microseconds(-bound_us), 
            std::chrono::microseconds(bound_us)
        )
        .build();

    //sink
    auto sink_op = Table_Sink_Builder<Q3_Out>(
            "q3_output",
            [](const Q3_Out& record, std::ostream& os) {
                os << record.name << "," << record.city << "," << record.state << "," << record.auction_id;
            }
        )
        .withName("q3_sink")
        .withHeader("name,city,state,auction_id")
        .withParallelism(2)
        .build();

    // --- 5. COSTRUZIONE TOPOLOGIA ---
    wf::PipeGraph topology("q3_topology", wf::Execution_Mode_t::DEFAULT, wf::Time_Policy_t::EVENT_TIME);

    auto& pipe_1 = topology.add_source(from_person).add(person_flatmap_op);
    auto& pipe_2 = topology.add_source(from_auction).add(auction_flatmap_op);
    
    // Risoluzione dei rami
    std::vector<wf::MultiPipe*> branches = {&pipe_1, &pipe_2};
    auto* merged_pipe = wf::merge_multipipes_func(&topology, branches);
    
    // Chiusura del grafo
    merged_pipe->add(join_op).add_sink(sink_op);

    topology.run();
    return 0;
}