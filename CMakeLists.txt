project(learn-rabbitmq-c CXX)

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_executable(consumer-c consumer-c.cpp error.h error.cpp)
target_link_libraries(consumer-c rabbitmq log4cplus)

add_executable(producer-c producer-c.cpp error.h error.cpp)
target_link_libraries(producer-c rabbitmq log4cplus)