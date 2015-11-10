project(learn-rabbitmq-c CXX)

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

include_directories(~/work/my-amqpcpp/include)
link_directories(~/work/my-amqpcpp)

add_executable(consumer-c consumer-c.cpp error.h error.cpp)
target_link_libraries(consumer-c rabbitmq log4cplus)

add_executable(producer-c producer-c.cpp error.h error.cpp)
target_link_libraries(producer-c rabbitmq log4cplus)

add_executable(consumer consumer.cpp)
target_link_libraries(consumer log4cplus SimpleAmqpClient)

add_executable(producer producer.cpp)
target_link_libraries(producer log4cplus SimpleAmqpClient)

add_executable(consumer-m consumer-m.cpp)
target_link_libraries(consumer-m log4cplus amqpcpp rabbitmq)

add_executable(producer-m producer-m.cpp)
target_link_libraries(producer-m log4cplus amqpcpp rabbitmq)