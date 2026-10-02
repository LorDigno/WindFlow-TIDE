
cmake_minimum_required(VERSION 3.16)
project(WindFlowGeneratedQueries CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

find_package(Threads REQUIRED)

include_directories(
    "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_queries/../include"
    "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_queries/../include/WindFlow/wf"
    "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_queries/../include/fastflow"
    "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_queries/../include/Table_WindFlow"
    "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_queries/query1/to-eur"
)
            
# --- Target per Query: dol_to_eur ---
add_executable(dol_to_eur dol_to_eur_main.cpp)
target_link_libraries(dol_to_eur PRIVATE Threads::Threads pthread)
