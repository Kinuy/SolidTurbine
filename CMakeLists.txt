cmake_minimum_required(VERSION 3.16)
project(SolidTurbine 
    VERSION 1.0.0 
    DESCRIPTION "Wind Turbine Performance Prediction Software"
    LANGUAGES CXX
)

# C++17 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)




# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include(FetchContent)

FetchContent_Declare(
    Eigen3
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG 5.0.1  # or latest stable version
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Eigen3)


# Option to build tests
option(BUILD_TESTS "Build unit tests" OFF)

# Add subdirectories
add_subdirectory(src)

# Configure include directories for the solidturbine target
#target_include_directories(solidturbine
#    PUBLIC
#        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
#        $<INSTALL_INTERFACE:include>
#)

# Install targets
install(TARGETS solidturbine
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install headers
install(DIRECTORY include/ 
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

target_link_options(solidturbine PRIVATE
    -static-libgcc
    -static-libstdc++
)

target_compile_options(solidturbine PRIVATE -Wno-psabi)


# Install data files
#install(DIRECTORY data/ 
#    DESTINATION share/solidturbine/data
#)
