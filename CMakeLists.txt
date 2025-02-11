cmake_minimum_required(VERSION 3.30)
project(FT12SerialFrameReceiver C)

set(CMAKE_C_STANDARD 11)

set(headers
        FT12SerialFrameReceiver.h
)

set(sources
        FT12SerialFrameReceiver.c
)

add_executable(FT12SerialFrameReceiver ${headers} ${sources})
add_executable(TestThreads TestThreads.c TestThreads.h)