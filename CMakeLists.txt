cmake_minimum_required(VERSION 3.30)
project(SerialReader C)

set(CMAKE_C_STANDARD 11)

set(headers
    SerialReader.h
)

set(sources
    SerialReader.c
)

add_executable(SerialReader ${headers} ${sources})
