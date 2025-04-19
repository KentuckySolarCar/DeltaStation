# First line of CMakeLists.txt must be the following line:
# feel free to change the version number to whatever version you have installed on your machine.
cmake_minimum_required(VERSION 3.15)
# Declare the name of this project (for logging purposes, etc.)
project(DeltaStation)

# Specify which language/version used for this project (C++20 is used here)
set(CMAKE_CXX_STANDARD 20)

# Declare an executable to be built, and explicitly add every compilation unit (AKA every .cpp file) to this executable
add_executable(ds libs/serialib/lib/serialib.cpp src/main.cpp
        src/RS-FEC.h
        src/InputParameters.cpp
        src/InputParameters.h
        src/BufferParser.cpp
        src/BufferParser.h
        src/common.h
        src/Reader.cpp
        src/Reader.h
        src/Dashboard.cpp
        src/Dashboard.h
        src/DebugReader.cpp
        src/DebugReader.h
)

find_package(Boost 1.40.0 COMPONENTS filesystem system iostreams REQUIRED)

target_include_directories(ds PRIVATE "${CMAKE_SOURCE_DIR}/libs/gnuplot-iostream")

# Specify include directories for a certain target. A target is a library, executable, or anything that gets compiled by
# CMake. As the `ds` target is a compiled executable, it is also a target. PRIVATE means that if another
# target were to depend on `ds`, it would not inherit the same include directories or compile flags.
target_include_directories(ds PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/libs/serialib/lib)
target_link_libraries(ds ${Boost_LIBRARIES})

