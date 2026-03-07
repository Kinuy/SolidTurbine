# ─────────────────────────────────────────────────────────────────────────────
# SolidTurbine — src/CMakeLists.txt
#
# Layout
# ──────
#   src/
#     main.cpp                          ← entry point
#     *.cpp                             ← SolidTurbine domain sources (flat)
#     bladenoise/                       ← bladenoise submodule (OBJECT library)
#       CMakeLists.txt
#       core/     math/   io/
#       noise/    airfoil/  potential/
#
# include/
#   *.h                                 ← SolidTurbine flat headers
#   bladenoise/
#     core/   io/   math/   noise/   airfoil/   potential/
#
# The bladenoise OBJECT library is compiled independently and its object files
# are linked directly into the solidturbine executable — no .lib is installed.
# ─────────────────────────────────────────────────────────────────────────────

add_subdirectory(bladenoise)

# ── SolidTurbine domain sources ───────────────────────────────────────────────
# Add all .cpp files in src/ (except main.cpp and the bladenoise subtree)
file(GLOB SOLIDTURBINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
list(FILTER SOLIDTURBINE_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

# ── Executable ────────────────────────────────────────────────────────────────
add_executable(solidturbine
    main.cpp
    ${SOLIDTURBINE_SOURCES}

    # ── SolidTurbine ↔ bladenoise bridge layer ────────────────────────────────
    # These files are in src/ alongside the other domain sources.
    # Listed explicitly because they depend on both sides of the boundary.
    BladeNoiseConfigBuilder.cpp
    SectionNoiseCalculator.cpp
    BEMSectionNoiseAdapter.cpp
    TecplotNoiseExporter.cpp
)

# Pull in bladenoise object files
target_link_libraries(solidturbine PRIVATE
    $<TARGET_OBJECTS:bladenoise>
    Eigen3::Eigen
)

# bladenoise PUBLIC include dirs propagate automatically via OBJECT library
target_include_directories(solidturbine PRIVATE
    ${PROJECT_SOURCE_DIR}/include     # flat SolidTurbine headers + bladenoise/ tree
    ${CMAKE_CURRENT_SOURCE_DIR}       # src/ itself (for bridge layer headers)
)

# ── Warnings ──────────────────────────────────────────────────────────────────
target_compile_options(solidturbine PRIVATE
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>: -Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>: /W4>
)
