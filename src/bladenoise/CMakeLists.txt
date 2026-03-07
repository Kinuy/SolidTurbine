# ─────────────────────────────────────────────────────────────────────────────
# bladenoise — compiled as an OBJECT library so its sources are merged
# directly into the solidturbine executable.  No separate .lib/.a is installed.
#
# Include paths:
#   ${PROJECT_SOURCE_DIR}/include   ← bladenoise/core/Types.h  etc.
#   ${PROJECT_SOURCE_DIR}/include   ← SolidTurbine flat headers (for bridge layer)
#
# The only external dependency is Eigen3 (already fetched by root CMakeLists).
# ─────────────────────────────────────────────────────────────────────────────
add_library(bladenoise OBJECT
    # ── core ──────────────────────────────────────────────────────────────────
    # (Types.h / Constants.h / ProjectConfig.h are header-only; no .cpp)

    # ── math ──────────────────────────────────────────────────────────────────
    math/SpecialFunctions.cpp
    math/ODEIntegrator.cpp
    math/Spline.cpp

    # ── io ────────────────────────────────────────────────────────────────────
    io/StreamlineWriter.cpp
    io/TextResultsWriter.cpp

    # ── noise ─────────────────────────────────────────────────────────────────
    noise/Directivity.cpp
    noise/TBLTENoiseSource.cpp
    noise/BluntnessNoiseSource.cpp
    noise/TurbulentInflowNoiseSource.cpp
    noise/NoiseCalculator.cpp

    # ── airfoil ───────────────────────────────────────────────────────────────
    airfoil/BPMBoundaryLayerCalculator.cpp
    airfoil/XfoilBoundaryLayerCalculator.cpp
    airfoil/BoundaryLayerFactory.cpp
    airfoil/WallProfileCalculator.cpp
    airfoil/AirfoilGeometryAnalyzer.cpp

    # ── potential ─────────────────────────────────────────────────────────────
    potential/PotentialFlowSolver.cpp
)

target_include_directories(bladenoise PUBLIC
    # bladenoise headers live under include/bladenoise/<subdir>/
    # so #include "bladenoise/core/Types.h" resolves correctly
    ${PROJECT_SOURCE_DIR}/include

    # SolidTurbine flat headers — needed by BladeNoiseConfigBuilder and bridge layer
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(bladenoise PUBLIC
    Eigen3::Eigen
)

# Suppress known false-positive warnings from bladenoise sources
# (they are third-party physics code, not SolidTurbine code)
target_compile_options(bladenoise PRIVATE
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
        -Wno-unused-parameter
        -Wno-sign-compare
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /wd4100   # unreferenced formal parameter
        /wd4244   # conversion, possible loss of data
    >
)
