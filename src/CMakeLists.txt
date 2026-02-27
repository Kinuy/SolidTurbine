#add_subdirectory(core)
#add_subdirectory(io)
#add_subdirectory(math)
#add_subdirectory(noise)
#add_subdirectory(airfoil)

# Main executable
add_executable(solidturbine 
    main.cpp
    Airfoil.cpp
    AirfoilAeroCharacteristic.cpp
    AirfoilGeometry.cpp
    AirfoilGeometryData.cpp
    AirfoilGeometryFileListData.cpp
    AirfoilGeometryFileListParser.cpp
    AirfoilGeometryInterpolationFactory.cpp
    AirfoilGeometryParser.cpp
    AirfoilPerformanceData.cpp
    AirfoilPerformanceFileListData.cpp
    AirfoilPerformanceFileListParser.cpp
    AirfoilPerformanceParser.cpp
    AirfoilPolarData.cpp
    AirfoilPolarInterpolationFactory.cpp
    AkimaSplineInterpolationStrategy.cpp
    Blade.cpp
    BladeBodyManager.cpp
    BladeGeometryData.cpp
    BladeGeometryParser.cpp
    BladeInterpolator.cpp
    BladeSection.cpp
    BoolParser.cpp
    BoundaryLayer.cpp
    Configuration.cpp
    ConfigurationParser.cpp
    ConfigurationSchema.cpp
    CubicSplineInterpolationStrategy.cpp
    CustomMultiValueParser.cpp
    DataExporter.cpp
    DefaultPathResolver.cpp
    DoubleParser.cpp 
    DXFCore.cpp
    DXFDocument.cpp
    DXFEntities.cpp
    # DXFBlade3D.cpp
    # DXFDocument.cpp
    # DXFEntityBase.cpp
    # DXFEntityFactory.cpp
    # DXFFileWriter.cpp
    # DXFFormatter.cpp
    # DXFHandleGenerator.cpp
    # DXFLine.cpp
    # DXFPoint.cpp
    # DXFPolyLine.cpp
    # DXFText.cpp
    ExporterFactory.cpp
    FilePathParser.cpp
    FileReader.cpp
    FileSystemDirectoryManager.cpp
    FlexiblePathResolver.cpp
    HorizontalTurbine.cpp
    IntParser.cpp
    IValueParser.cpp
    LiftingSurface.cpp
    LiftingSurfaceCreator.cpp
    LiftingSurfaceParameters.cpp
    LiftingSurfaceSolver.cpp
    LinearInterpolationStrategy.cpp
    LineParser.cpp
    MathUtility.cpp
    MathUtilities.cpp
    MonotonicCubicSplineInterpolationStrategy.cpp
    OperationAnalyzer.cpp
    OperationDataLogger.cpp
    OperationSimulationService.cpp
    RangeParser.cpp
    RLWakeModel.cpp
    StandardFileWriter.cpp
    StringParser.cpp
    Surface.cpp
    SurfaceCreator.cpp
    SurfaceWriter.cpp
    VTKFileWriter.cpp
    Wake.cpp
    TurbineGeometry.cpp
    BrentsRootFinder.cpp
    LossModels.cpp
    BladedBinaryReader.cpp
    BilinearTurbSimInterpolator.cpp
    TurbSimManager.cpp
    FlowCalculator.cpp
    OperationSolver.cpp
    NingSolver.cpp
    EmpiricalWakeInduction.cpp
    AEPCalculator.cpp
    VariableSpeedController.cpp
    StandardPitchSchedule.cpp
    BEMPostprocessor.cpp
    TecplotSimulationExporter.cpp
    DataFormat.cpp
    DataWriter.cpp
    FileOutputTarget.cpp
    TecplotFormatter.cpp
    TecplotBlade3DExporter.cpp
)

# Collect all .cpp files in src directory
#file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

#add_executable(solidturbine ${SOURCES})



target_include_directories(solidturbine PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

#target_link_libraries(solidturbine PRIVATE
#    bladenoise_core
#    bladenoise_io
#    bladenoise_math
#    bladenoise_noise
#    bladenoise_airfoil
#    Eigen3::Eigen
#)

# Also add Eigen3 if you use it:
target_link_libraries(solidturbine PRIVATE
    Eigen3::Eigen
)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
   target_link_libraries(solidturbine PRIVATE OpenMP::OpenMP_CXX)
endif()