SolidTurbine
Software for wind turbine aerodynamic analysis and blade design,
featuring advanced interpolation methods, configurable data processing, and robust
file I/O capabilities.

Key Features:

Data input:
Parsing of

- 2D Blade geometry data
- 2D airfoil geometry data as a set of geometry files
- Airfoil polar data as a set of performance files

Data processing:

- Interpolation of airfoil geometry and performance (3d interpolation-> AoA,Reynolds,Mach) data on any given blade section
  so that overall blade performance analysis can be performed.

Data output (wip):

- 3D Geometry data (DXF format) for further 3D geometry processing in CAD tools
- 2D performance data of simulation results (tbd)

TODOs:

- implement output 3D Data
- implement steady state solver
- implement turbine controller
