cmake_minimum_required(VERSION 3.10)

project(Mp4FrameSaver)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

include_directories(include)

add_subdirectory(spdlog)

set(SOURCES
    source/main.cpp
    source/FramePublisher.cpp
    source/FrameResizer.cpp
    source/FrameSaver.cpp
    source/Service.cpp
    source/MessageQueue.cpp
)

add_executable(mp4FrameSaver ${SOURCES})

target_include_directories(mp4FrameSaver PUBLIC ${CMAKE_CURRENT_SOURCE_DIRS}/include)

target_link_libraries(mp4FrameSaver pthread rt spdlog)

