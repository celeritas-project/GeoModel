# Copyright (C) 2023 CERN for the benefit of the ATLAS collaboration

# Set up the project.
cmake_minimum_required(VERSION 3.16...3.26)

# Set up the project as standalone or subproject of GeoModel
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # I am top-level project.
  # Make the root module directory visible to CMake.
  list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake )
  # get global GeoModel version
  include( GeoModel-version )
  # set the project, with the version taken from the GeoModel parent project
  # Later find of Celeritas/AdePT will enable CUDA if it's needed
  project( GPUPlugins VERSION ${GeoModel_VERSION} LANGUAGES CXX )
  # Define color codes for CMake messages
  include( cmake_colors_defs )
  # Warn the users about what they are doing
  message(STATUS "${BoldGreen}Building ${PROJECT_NAME} individually, as a top-level project.${ColourReset}")
  # Set a default build type
  include( BuildType )
  # Set default build and C++ options
  include( configure_cpp_options )
  set( CMAKE_FIND_FRAMEWORK "LAST" CACHE STRING "Framework finding behaviour on macOS" )
  # Set up how the project handle some of its dependenices. Either by picking them
  # up from the environment, or building them itself.
  include( SetupJSON )
  include( SetupXercesC )
  # Find the base GeoModel packages, which must be installed on the target system already
  find_package( GeoModelCore REQUIRED ${GeoModel_VERSION} )
  find_package( FullSimLight REQUIRED ${GeoModel_VERSION} )
  # Print Build Info on screen
  include( PrintBuildInfo )
else()
  # I am called from other project with add_subdirectory().
  message( STATUS "Building ${PROJECT_NAME} as part of the root GeoModel project.")
  # Set the project
  project( GPUPlugins VERSION ${GeoModel_VERSION} LANGUAGES CXX )
endif()

# Minimum neccesary to run Celeritas as a plugin
add_subdirectory(MinCeleritas)

# Separate for now TileCal Plugins
add_subdirectory(ATLTileCalTB)

# TODO: Add installation commands
