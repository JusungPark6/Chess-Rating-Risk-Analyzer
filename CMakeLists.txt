cmake_minimum_required(VERSION 3.10)
project(ChessRating)

set(CMAKE_CXX_STANDARD 11)

add_executable(ChessRating main.cpp)

find_package(CURL REQUIRED)
target_include_directories(ChessRating PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(ChessRating PRIVATE ${CURL_LIBRARIES})
