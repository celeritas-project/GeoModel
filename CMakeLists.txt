# Set up the project.
cmake_minimum_required( VERSION 3.10 )

# set minimum C++b standard
set  (CMAKE_CXX_STANDARD 14)

project( "GeoModelVisualization" VERSION 3.2.0 LANGUAGES CXX )
set    ( CMAKE_BUILD_TYPE RELEASE )

set    (OpenGL_GL_PREFERENCE GLVND)


# Make the module directory visible to CMake.
list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

# Use the GNU install directory names.
include( GNUInstallDirs )

# Set up the "optional" dependencies.
include( SetupJSON )


# Project's dependencies.
find_package( GeoModelCore 3.2.0 REQUIRED )
find_package( GeoModelIO   3.2.0 REQUIRED )
find_package( Qt5 COMPONENTS Core Gui Widgets Sql )
# Set sub-packages to build.
add_subdirectory(VP1GeometryPlugin)
add_subdirectory(VP1GeometrySystems)
add_subdirectory(GXClashPointSystems)
add_subdirectory(GeoModelExplorer)
add_subdirectory(VP1Gui)
add_subdirectory(VP1Base)
add_subdirectory(VP1HEPVis)

add_subdirectory(VP1GuideLineSystems)