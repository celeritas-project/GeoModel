# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: HelloGeoRead2G4
# author: Riccardo Maria BIANCHI @ CERN - Nov, 2018
# update: Riccardo Maria BIANCHI @ CERN - Jul, 2023
################################################################################

cmake_minimum_required(VERSION 3.16...3.26)

project(HelloGeoRead2G4)

# Find includes in current dir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find the needed dependencies, when building individually
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR) 
  message(STATUS "Compiling the example '${PROJECT_NAME}' as a standalone project...")
  find_package( GeoModelCore REQUIRED  )
  find_package( GeoModelIO REQUIRED ) 
  find_package( GeoModelG4 REQUIRED ) 
  find_package( Eigen3 REQUIRED ) 
endif()

## External dependencies.
find_package( Geant4 REQUIRED )
# include Geant4 headers
include(${Geant4_USE_FILE})

# Populate a CMake variable with the sources
set( SRCS HelloGeoReadG4.cpp )

# Tell CMake to create the helloworld executable
add_executable( hellogeoRead2G4 ${SRCS} )

# Link all needed libraries
target_link_libraries( hellogeoRead2G4 GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelRead GeoModelCore::GeoModelKernel GeoModel2G4 Eigen3::Eigen ${Geant4_LIBRARIES})
