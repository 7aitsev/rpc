cmake_minimum_required(VERSION 3.13)
project(rpc)

set(CMAKE_CXX_STANDARD 14)

add_executable(sample_server sample_server.cpp abstract_rpc.hpp json_rpc.cpp json_rpc.hpp)

find_package(Boost REQUIRED COMPONENTS thread system)
target_include_directories(sample_server PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(sample_server ${Boost_LIBRARIES})

add_executable(sample_client sample_client.cpp json_rpc.cpp json_rpc.hpp client.hpp)
target_include_directories(sample_client PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(sample_client ${Boost_LIBRARIES})