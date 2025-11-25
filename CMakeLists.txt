# First line of CMakeLists.txt must be the following line:
# feel free to change the version number to whatever version you have installed on your machine.
cmake_minimum_required(VERSION 3.15)
# Declare the name of this project (for logging purposes, etc.)
project(DeltaStation VERSION 0.2.0)

# Specify which language/version used for this project (C++20 is used here)
set(CMAKE_CXX_STANDARD 20)
set(ImGui_INSTALL_DIR libs/imgui)

# Declare an executable to be built, and explicitly add every compilation unit (AKA every .cpp file) to this executable
add_executable(ds
        src/main.cpp
        src/RS-FEC.h
        src/InputParameters.cpp
        src/InputParameters.h
        src/BufferParser.cpp
        src/BufferParser.h
        src/common.h
        src/IOSerial.cpp
        src/IOSerial.h
        src/Dashboard.cpp
        src/Dashboard.h
        src/DebugReader.cpp
        src/DebugReader.h
        src/Window.cpp
        src/Window.h

        libs/serialib/lib/serialib.cpp

        libs/implot/implot.cpp
        libs/implot/implot.h
        libs/implot/implot_internal.h
        libs/implot/implot_items.cpp

        ${ImGui_INSTALL_DIR}/imgui.cpp
        ${ImGui_INSTALL_DIR}/imgui_demo.cpp
        ${ImGui_INSTALL_DIR}/imgui_draw.cpp
        ${ImGui_INSTALL_DIR}/imgui_tables.cpp
        ${ImGui_INSTALL_DIR}/imgui_widgets.cpp

        ${ImGui_INSTALL_DIR}/backends/imgui_impl_glfw.cpp
        ${ImGui_INSTALL_DIR}/backends/imgui_impl_opengl3.cpp
        src/Config.cpp
        src/Config.h

        src/expr/Lexer.cpp
        src/expr/Lexer.h
        src/expr/Parser.cpp
        src/expr/Parser.h

        src/Graph.cpp
        src/Graph.h
)

if (MSVC_IDE)
    set_target_properties(ds PROPERTIES ARCHIVE_OUTPUT_DIRECTORY $<1:${PROJECT_SOURCE_DIR}/build>)
    set_target_properties(ds PROPERTIES LIBRARY_OUTPUT_DIRECTORY $<1:${PROJECT_SOURCE_DIR}/build>)
    set_target_properties(ds PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:${PROJECT_SOURCE_DIR}/build>)
endif()

find_package(OpenGL REQUIRED)
add_subdirectory(libs/glfw)

# Specify include directories for a certain target. A target is a library, executable, or anything that gets compiled by
# CMake. As the `ds` target is a compiled executable, it is also a target. PRIVATE means that if another
# target were to depend on `ds`, it would not inherit the same include directories or compile flags.
target_include_directories(ds PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/libs/serialib/lib
        ${CMAKE_SOURCE_DIR}/libs/implot
        ${CMAKE_SOURCE_DIR}/libs/tomlplusplus/include
        ${ImGui_INSTALL_DIR}
)

target_link_libraries(ds PRIVATE glfw OpenGL::GL)

target_compile_options(ds PRIVATE -g)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
