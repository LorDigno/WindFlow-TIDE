cmake_minimum_required(VERSION 3.16)
project(WindFlowNativeBenchmarks CXX)

# Ottimizzazioni massime per il benchmark
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Threading (richiesto da WindFlow e FastFlow)
find_package(Threads REQUIRED)

# Risaliamo di un livello per raggiungere TableAPI/include
set(TABLE_API_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(INC_ROOT "${TABLE_API_ROOT}/include")

# Inclusioni
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}              # Per nexmark_streams.hpp
    ${INC_ROOT}                              # Cartella include globale
    ${INC_ROOT}/Table_WindFlow               # Builder della Table API
    ${INC_ROOT}/WindFlow                     # WindFlow root
    ${INC_ROOT}/WindFlow/wf                  # Per <windflow.hpp>
    ${INC_ROOT}/fastflow                     # Per <ff/...>
)

# --- Target per la Query 2 (specific_id) ---
add_executable(specific_id query2.cpp)
target_link_libraries(specific_id PRIVATE Threads::Threads pthread)

# --- Target per la Query 3 (selling_in_states) ---
add_executable(selling_in_states query3.cpp)
target_link_libraries(selling_in_states PRIVATE Threads::Threads pthread)
