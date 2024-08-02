cmake_minimum_required(VERSION 3.28)
project(cc_quote)

set(CMAKE_CXX_STANDARD 17)

add_executable(cc_quote main.cpp
        crc16.c)
