
cmake_minimum_required(VERSION 3.16)
project(WindFlowGeneratedQueries CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

find_package(Threads REQUIRED)

include_directories(
    "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_queries/query12/../../include"
    "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_queries/query12/../../include/WindFlow/wf"
    "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_queries/query12/../../include/fastflow"
    "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_queries/query12/../../include/Table_WindFlow"
    "/disc1/homes/lorenzoni/WindFlow-Table-API/nexmark_queries/query12/ingress"
)
            
# --- Target per Query: ingress_time_windows ---
add_executable(ingress_time_windows ingress_time_windows_main.cpp)
target_link_libraries(ingress_time_windows PRIVATE Threads::Threads pthread)
