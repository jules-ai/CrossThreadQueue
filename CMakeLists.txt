cmake_minimum_required(VERSION 3.1.0)
set(CMAKE_CXX_STANDARD 14)
project(ctQueue)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/built/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/built/")

#include_directories(
#
#)

add_executable(${PROJECT_NAME} example.cpp cross_thread_queue.hpp)
target_link_libraries(${PROJECT_NAME} PUBLIC -pthread)

install(TARGETS ${PROJECT_NAME}
       ARCHIVE DESTINATION lib
       LIBRARY DESTINATION lib
       RUNTIME DESTINATION lib)