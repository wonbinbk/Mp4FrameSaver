cmake_minimum_required(VERSION 3.10)

project(Mp4FrameSaver)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(nlohmann_json REQUIRED)

include_directories(include)

add_subdirectory(spdlog)

set(SOURCES
    source/main.cpp
    source/FramePublisher.cpp
    source/FrameResizer.cpp
    source/FrameSaver.cpp
    source/Service.cpp
    source/MessageQueue.cpp
    source/Utils.cpp
)
add_executable(mp4FrameSaver ${SOURCES})

target_include_directories(mp4FrameSaver PUBLIC ${CMAKE_CURRENT_SOURCE_DIRS}/include)

target_link_libraries(mp4FrameSaver pthread rt ${OpenCV_LIBS} spdlog nlohmann_json::nlohmann_json)

# A small client app to set video file source for the mp4FrameSaver service
set(CLIENT_SOURCES
    source/client/main.cpp
    source/MessageQueue.cpp
)
add_executable(extractMp4Frames ${CLIENT_SOURCES})
target_include_directories(extractMp4Frames PUBLIC ${CMAKE_CURRENT_SOURCE_DIRS}/include)
target_link_libraries(extractMp4Frames rt spdlog)

