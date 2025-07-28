# Example CMake command line to create project build files:
#
# cmake -B build .

# Specify the minimum CMake version required
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Project name and language (C or C++)
project(pmr_allocator VERSION 1.0 LANGUAGES CXX)

# Collect all .cpp and *.h source files in the current directory
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/*.cpp" "${CMAKE_SOURCE_DIR}/*.h")

# Add an executable target
add_executable(pmr_allocator_app ${SOURCES})



