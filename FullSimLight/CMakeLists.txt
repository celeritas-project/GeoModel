# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# CMake settings
cmake_minimum_required(VERSION 3.16...3.26)

#Set up the project. Check if we build it with GeoModel or individually
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # I am top-level project.
  # Make the root module directory visible to CMake.
  list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake )
  # get global GeoModel version
  include( GeoModel-version ) 
  # set the project, with the version taken from the GeoModel parent project
  project( FullSimLight VERSION ${GeoModel_VERSION} LANGUAGES CXX )
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
  find_package( GeoModelIO REQUIRED ${GeoModel_VERSION} )
  find_package( GeoModelG4 REQUIRED ${GeoModel_VERSION} )
  # Print Build Info on screen
  include( PrintBuildInfo )
else()
  # I am called from other project with add_subdirectory().
  message( STATUS "Building ${PROJECT_NAME} as part of the root GeoModel project.")
  # Set the project
  project( FullSimLight VERSION ${GeoModel_VERSION} LANGUAGES CXX )
endif()

## External dependencies.
#----------------------------------------------------------------------------
# Find Geant4 package, batch mode only executable (i.e. no need ui and vis).
find_package(Geant4 REQUIRED)
message( STATUS "Found Geant4: ${Geant4_INCLUDE_DIR}")

# Support for the HepMC3 exchange format
# This is OFF by default, but it can be enbled by the user if used/needed;
# for that, of course, HepMC3 must be installed on the system.
option(GEOMODEL_USE_HEPMC3 "Build GeoModel tools with support for the HepMC3 exchancge format (Note: HepMC3 must be installed on the target machine)" OFF )
if(GEOMODEL_USE_HEPMC3)
  find_package(HepMC3 REQUIRED) # required by default, but it can be disabled 
endif()

# Support for Pythia event generator
# This is OFF by default, but it can be enbled by the user if used/needed;
# for that, of course, Pythia must be installed on the system.
option(GEOMODEL_USE_PYTHIA "Build GeoModel tools with support for the Pythia event generator  (Note: Pythia must be installed on the target machine)" OFF )
if(GEOMODEL_USE_PYTHIA)
  find_package(Pythia REQUIRED) # if support for Pythia is enabled, it must be installed.
endif()

# Support for Celeritas
# NB: a dummy find as it's only used in Plugins/GPUPlugins, but CMake/FSL project
# structure will fail generation with an error about no compile features for
# the CUDA compiler, which is unknown. TODO: understand/resolve this
find_package(Celeritas 0.3 QUIET)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

if(NOT HepMC3_FOUND)
  file(GLOB hepmc3src ${PROJECT_SOURCE_DIR}/src/HepMC3*.cc)
  list(REMOVE_ITEM sources ${hepmc3src})
endif()

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
set(OUTPUT bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${OUTPUT})

# Plugin Interface Library
add_library(FullSimLight INTERFACE)
add_library(FullSimLight::FullSimLight ALIAS FullSimLight)
target_include_directories( FullSimLight INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# OBJECT library for the shared functionality
add_library(FullSimLight_obj OBJECT ${sources} ${headers})
target_compile_definitions(FullSimLight_obj 
  PUBLIC
    G4SHAREDIR="${Geant4_INCLUDE_DIR}/../../share"
    FULLSIMLIGHTSHAREDIR="${CMAKE_INSTALL_PREFIX}/share/FullSimLight"
    G4VERSION="${Geant4_VERSION}"
    $<$<BOOL:${Pythia_FOUND}>:USE_PYTHIA>
    $<$<BOOL:${HepMC3_FOUND}>:USE_HEPMC3>)
target_include_directories(FullSimLight_obj
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${HEPMC3_INCLUDE_DIR})
target_link_libraries(FullSimLight_obj
  PUBLIC
    nlohmann_json::nlohmann_json
    GeoModel2G4
    GeoModelCore::GeoModelKernel
    GeoModelIO::GeoModelRead
    GeoModelIO::GeoModelWrite
    ${HEPMC3_LIB}
    ${Geant4_LIBRARIES}
    $<$<BOOL:${Pythia_FOUND}>:Pythia::Pythia>
  PRIVATE
    FullSimLight::FullSimLight)

# Primary executables
add_executable(fullSimLight fullSimLight.cc)
target_link_libraries(fullSimLight PRIVATE FullSimLight_obj)

add_executable(gmclash geoModelClash.cc)
target_link_libraries(gmclash PRIVATE FullSimLight_obj)

add_executable(gmmasscalc geoModelMassCalculator.cc)
target_link_libraries(gmmasscalc PRIVATE FullSimLight_obj)

add_executable(gmgeantino geantinoMaps.cc)
target_link_libraries(gmgeantino PRIVATE FullSimLight_obj)

add_executable(gm2gdml geoModel2GDML.cc)
target_link_libraries(gm2gdml PRIVATE FullSimLight_obj)

add_executable(fillHistogramExample fillHistogramExample.cc src/Histo.cc include/Histo.hh)
target_include_directories(fillHistogramExample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(fillHistogramExample ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------
# Add sub-projects and targets
add_subdirectory(Plugins)

# Add profiling test targets
if(GEOMODEL_BUILD_FULLSIMLIGHT_PROFILING)
  if(NOT Pythia_FOUND)
    message(FATAL_ERROR "FullSimLight needs to be built with Pythia support for profiling with perf")
  endif()
  add_subdirectory(perf)
endif()

#----------------------------------------------------------------------------
# Copy all scripts to the build/OUTPUT directory. This is so that, after
# install, we can run the executable directly because it relies on these
# scripts being in the current working directory.
#
set(FULLSIMLIGHT_SCRIPTS
  geantino.g4
  geantino_Pixel.g4
  geantino_SCT.g4
  geantino_TRT.g4
  geantino_LAr.g4
  geantino_Tile.g4
  geantino_Muon.g4
  macro.g4
  pythia.g4
  drawMagField.C
  drawGeantinoMaps.C
  )

foreach(_script ${FULLSIMLIGHT_SCRIPTS})
  configure_file(
    ${_script}
    ${CMAKE_BINARY_DIR}/share/${_script}
    COPYONLY
    )
endforeach()

#----------------------------------------------------------------------------
# Install the executables to 'bin/' directory under the
# CMAKE_INSTALL_PREFIX
#
install(TARGETS fullSimLight gmclash gmmasscalc fillHistogramExample gmgeantino gm2gdml
  DESTINATION ${OUTPUT})
install(FILES ${FULLSIMLIGHT_SCRIPTS} DESTINATION share/FullSimLight)

# Set up the packaging of the project using CPack.

include ( GNUInstallDirs )
include( WriteBasicConfigVersionFile )

write_basic_config_version_file(
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion )

install(
  FILES
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}ConfigVersion.cmake
  COMPONENT Development
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME} )

#Create and install the description of the libraries.
install( EXPORT ${PROJECT_NAME}-export
  FILE ${PROJECT_NAME}Targets.cmake
  COMPONENT Development
  NAMESPACE "${PROJECT_NAME}::"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME} )

# Install the hand-written project configuration.
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FullSimLightConfig.cmake.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake
  @ONLY )
install(
  FILES
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake
  COMPONENT Development
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME} )


install( TARGETS FullSimLight
  EXPORT ${PROJECT_NAME}-export
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} )

   
install(FILES FullSimLight/FSLSensitiveDetectorPlugin.h 
              FullSimLight/MagFieldPlugin.h 
              FullSimLight/FSLPhysicsListPlugin.h
    	      FullSimLight/FSLUserActionPlugin.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/FullSimLight
  COMPONENT Development )
