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
set(EIGEN_BUILD_DOC       OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING         OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_PKGCONFIG OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(Eigen3)


# ── OpenMP ───────────────────────────────────────────────────────────────────
find_package(OpenMP)

option(BUILD_TESTS "Build unit tests" OFF)

# Add subdirectories
add_subdirectory(src)

# ── Per-target flags (must come after add_subdirectory so target exists) ──────
# Warnings are set per-target inside src/CMakeLists.txt; only GCC ABI and
# static-runtime flags live here because they apply to the final link step.
if(OpenMP_CXX_FOUND)
    target_link_libraries(solidturbine PRIVATE OpenMP::OpenMP_CXX)
endif()

if(NOT MSVC)
    target_link_options(solidturbine PRIVATE
        -static-libgcc
        -static-libstdc++
    )
    target_compile_options(solidturbine PRIVATE -Wno-psabi)
endif()

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
