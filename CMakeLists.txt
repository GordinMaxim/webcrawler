project(WebCrawler)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -lcurl")
add_executable(${PROJECT_NAME} ${SRC_LIST})

