
cmake_minimum_required(VERSION 3.16)
project(WindFlowGeneratedQueries CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

find_package(Threads REQUIRED)

include_directories(
    "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_queries/discard_baselines/../../include"
    "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_queries/discard_baselines/../../include/WindFlow/wf"
    "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_queries/discard_baselines/../../include/fastflow"
    "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_queries/discard_baselines/../../include/Table_WindFlow"
    "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_queries/discard_baselines/person"
)
            
# --- Target per Query: discard_p ---
add_executable(discard_p discard_p_main.cpp)
target_link_libraries(discard_p PRIVATE Threads::Threads pthread)
