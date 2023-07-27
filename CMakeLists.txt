# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# === Preamble ===
cmake_minimum_required(VERSION 3.16...3.26)

# === Project's settings ===
include( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GeoModel-version.cmake )
project( "GeoModel" VERSION ${GeoModel_VERSION} LANGUAGES CXX )

# Make the 'cmake' module directory visible to CMake.
list( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )

# === Project wide setup ===
# Define color codes for CMake messages
include( cmake_colors_defs )
# Use the GNU install directory names.
include( GNUInstallDirs )
# Set a default build type
include( BuildType )
# Set default build and C++ options
include( configure_cpp_options )

# By default prefer not to use frameworks on macOS.
# But use it at a "LAST" resource, if no other options worked.
# For details on the behaviour of this cache variable, see:
# - https://cmake.org/cmake/help/v3.0/command/find_file.html
if( APPLE )
  set( CMAKE_FIND_FRAMEWORK "LAST" CACHE STRING "Framework finding behaviour on macOS" )
endif()

# === Externally provided content ===
# Set up how the project handle some of its dependenices. Either by picking them
# up from the environment, or building them itself.
include( SetupEigen3 )
include( SetupXercesC )
include( SetupJSON )

# Find the dependencies that the project always picks up from its environment.
find_package( SQLite3 3.7.17 )

# === Main targets built by this project ===

# switches to let users build specific packages on request
option(GEOMODEL_BUILD_ALL "Enable the build of all GeoModel sub-packages" OFF)
option(GEOMODEL_BUILD_TOOLS "Enable the build of GeoModelTools" OFF)
option(GEOMODEL_BUILD_EXAMPLES "Enable the build of GeoModelExamples" OFF)
option(GEOMODEL_BUILD_EXAMPLES_W_GEANT4 "Enable the build of GeoModelExamples, including the ones concerning the interface GeoModel->Geant4" OFF)
option(GEOMODEL_BUILD_VISUALIZATION "Enable the build of GeoModelVisualization" OFF)
option(GEOMODEL_BUILD_GEOMODELG4 "Enable the build of GeoModelG4" OFF)
option(GEOMODEL_BUILD_FULLSIMLIGHT "Enable the build of FullSimLight" OFF)
option(GEOMODEL_BUILD_FULLSIMLIGHT_PROFILING "Enable FullSimLight profiling targets" OFF)
option(GEOMODEL_BUILD_FSL "Enable the build of FSL and FullSimLight" OFF)
option(GEOMODEL_BUILD_ATLASEXTENSIONS "Build the Custom ATLAS Extensions" OFF)


if(GEOMODEL_BUILD_FSL AND GEOMODEL_BUILD_FULLSIMLIGHT)
  message(STATUS "==> NOTE: You enabled both BUILD_FSL and BUILD_FULLSIMLIGHT. Since BUILD_FSL triggers the build of the FullSimLight package as well, I set BUILD_FULLSIMLIGHT to FALSE to avoid errors of 'double inclusion'")
  set(GEOMODEL_BUILD_FULLSIMLIGHT FALSE)
endif()


if(GEOMODEL_BUILD_FULLSIMLIGHT_PROFILING)
  set(GEOMODEL_BUILD_FULLSIMLIGHT ON CACHE BOOL "Enable the build of FullSimLight" FORCE)
  include(CTest) # needs to be included at the top level
endif()

# a list to keep track of the packages we build
set(BUILT_PACKAGES "")

add_subdirectory(GeoModelCore)
list( APPEND BUILT_PACKAGES "GeoModelCore")
add_subdirectory(GeoModelIO)
list( APPEND BUILT_PACKAGES "GeoModelIO")

if(GEOMODEL_BUILD_ALL)
  set(GEOMODEL_BUILD_TOOLS         TRUE)
  set(GEOMODEL_BUILD_VISUALIZATION TRUE)
  set(GEOMODEL_BUILD_FSL           TRUE)
endif()

if(GEOMODEL_BUILD_TOOLS)
  add_subdirectory(GeoModelTools)
  list( APPEND BUILT_PACKAGES "GeoModelTools")
endif()

if(GEOMODEL_BUILD_VISUALIZATION)
  add_subdirectory(GeoModelVisualization)
  list( APPEND BUILT_PACKAGES "GeoModelVisualization")
endif()

if(GEOMODEL_BUILD_EXAMPLES)
  add_subdirectory(GeoModelExamples)
  list( APPEND BUILT_PACKAGES "GeoModelExamples")
endif()

if(GEOMODEL_BUILD_FULLSIMLIGHT)
  set(GEOMODEL_BUILD_GEOMODELG4 TRUE) # FullSimLight needs GeoModelG4
  add_subdirectory(FullSimLight)
  list( APPEND BUILT_PACKAGES "FullSimLight")
endif()

if(GEOMODEL_BUILD_FSL)
  set(GEOMODEL_BUILD_GEOMODELG4 TRUE) # FullSimLight needs GeoModelG4
  add_subdirectory(FullSimLight)# FSL needs FullSimLight
  add_subdirectory(FSL)
  list( APPEND BUILT_PACKAGES "FullSimLight")
  list( APPEND BUILT_PACKAGES "FSL")
endif()

if(GEOMODEL_BUILD_ATLASEXTENSIONS)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ATLASExtensions)
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ATLASExtensions/CMakeLists.txt)
            add_subdirectory(ATLASExtensions)
            list( APPEND BUILT_PACKAGES "ATLASExtensions")
        else()
            message(FATAL_ERROR "You enabled the build of the ATLASExtensions submodule, by setting the 'GEOMODEL_BUILD_ATLASEXTENSIONS=1' option. However, no 'CMakeLists.txt' file was found in the ATLASExtensions folder! You probably forgot to recursively clone the submodule. Run this from the source top folder: 'git submodule update --init'. Then, re-run CMake." )
        endif()
    else()
        message(FATAL_ERROR "You enabled the build of the ATLASExtensions submodule, by setting the 'GEOMODEL_BUILD_ATLASEXTENSIONS=1' option. However, no 'ATLASExtensions' folder is found in the repository! Please, check the integrity of your copy of the GeoModel repository." )
    endif()
endif()

if(GEOMODEL_BUILD_GEOMODELG4 OR GEOMODEL_BUILD_EXAMPLES_W_GEANT4)
  add_subdirectory(GeoModelG4)
  list( APPEND BUILT_PACKAGES "GeoModelG4")
endif()

# Print Build Info on screen
include( PrintBuildInfo )

# Let the users know which and how many packages they are building
list(LENGTH BUILT_PACKAGES BUILT_PACKAGES_LENGTH)
string(REPLACE ";" ", " BUILT_PACKAGES_STR "${BUILT_PACKAGES}")

message(STATUS "${BoldWhite}-----${ColourReset}")
message(STATUS "${BoldGreen}Building the following ${BUILT_PACKAGES_LENGTH} packages: ${BUILT_PACKAGES_STR}${ColourReset}")
message(STATUS "${BoldWhite}-----${ColourReset}")
