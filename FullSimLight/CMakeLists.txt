#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(fullSimLight)

# Compile with C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
 
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(MagneticField)

# Dependencies.
## GeoModel dependencies.
find_package( GeoModelCore 3.2.0 REQUIRED )
find_package( GeoModelIO 3.2.0 REQUIRED )
find_package( GeoModelG4 REQUIRED )
find_package( nlohmann_json QUIET )

# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
#list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
#find_package(ROOT REQUIRED COMPONENTS RIO Net)

#---Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
#include(${ROOT_USE_FILE})


## External dependencies.
#----------------------------------------------------------------------------
# Find Geant4 package, batch mode only executable (i.e. no need ui and vis).
#
find_package(Geant4 REQUIRED)
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
# Setup include directory for this project
#
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)


#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/inc/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
set(OUTPUT bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${OUTPUT})

add_executable(fullSimLight fullSimLight.cc ${sources} ${headers})
add_executable(gmclash geoModelClash.cc ${sources} ${headers})
add_executable(fillHistogramExample fillHistogramExample.cc src/Histo.cc include/Histo.hh)
add_executable(gmgeantino geantinoMaps.cc ${sources} ${headers})
#add_executable(plotGeantinoMaps plotGeantinoMaps.cc)
add_executable(testMagneticField testMagneticField.cc)
add_executable(gmtogdml geoModeltoGDML.cc ${sources} ${headers})



#----------------------------------------------------------------------------
# Link all needed libraries

target_link_libraries(fillHistogramExample ${Geant4_LIBRARIES})
target_link_libraries(testMagneticField ${Geant4_LIBRARIES} MagFieldServices MagFieldInterfaces)

if( nlohmann_json_FOUND )

target_link_libraries(fullSimLight GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelRead GeoModelCore::GeoModelKernel GeoModel2G4 ${Geant4_LIBRARIES} MagFieldServices MagFieldInterfaces nlohmann_json::nlohmann_json )

target_link_libraries(gmclash GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelRead GeoModelCore::GeoModelKernel GeoModel2G4 ${Geant4_LIBRARIES} MagFieldServices MagFieldInterfaces nlohmann_json::nlohmann_json )

target_link_libraries(gmgeantino GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelRead GeoModelCore::GeoModelKernel GeoModel2G4 ${Geant4_LIBRARIES} MagFieldServices MagFieldInterfaces nlohmann_json::nlohmann_json )

target_link_libraries(gmtogdml GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelRead GeoModelCore::GeoModelKernel GeoModel2G4 ${Geant4_LIBRARIES} MagFieldServices MagFieldInterfaces nlohmann_json::nlohmann_json )

#target_link_libraries(plotGeantinoMaps ${ROOT_LIBRARIES})

else()

message(STATUS "'nlohmann_json' not found by CMake!! Anyway, if you installed the single header file in a standard system include dir, I will be able to use it.")

target_link_libraries(gmclash GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelRead GeoModelCore::GeoModelKernel GeoModel2G4 ${Geant4_LIBRARIES} MagFieldServices MagFieldInterfaces)

target_link_libraries(fullSimLight GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelRead GeoModelCore::GeoModelKernel GeoModel2G4 ${Geant4_LIBRARIES} MagFieldServices MagFieldInterfaces)

target_link_libraries(gmgeantino GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelRead GeoModelCore::GeoModelKernel GeoModel2G4 ${Geant4_LIBRARIES} MagFieldServices MagFieldInterfaces)

target_link_libraries(gmtogdml GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelRead GeoModelCore::GeoModelKernel GeoModel2G4 ${Geant4_LIBRARIES} MagFieldServices MagFieldInterfaces)

#target_link_libraries(plotGeantinoMaps ${ROOT_LIBRARIES})

endif()

#ROOT_GENERATE_DICTIONARY(G__geantinoMaps ${headers} LINKDEF geantinoMapsLinkDef.h)


#----------------------------------------------------------------------------
# Copy all scripts to the build/OUTPUT directory. This is so that, after
# install, we can run the executable directly because it relies on these
# scripts being in the current working directory.
#
set(FULLSIMLIGHT_SCRIPTS
  geantino.g4
  macro.g4
  drawMagField.C
  )

foreach(_script ${FULLSIMLIGHT_SCRIPTS})
  configure_file(
    ${_script}
    ${CMAKE_BINARY_DIR}/share/${_script}
    COPYONLY
    )
endforeach()

#----------------------------------------------------------------------------
# Install the executable to 'bin/' directory under the
# CMAKE_INSTALL_PREFIX
#
install(TARGETS fullSimLight DESTINATION ${OUTPUT})
install(TARGETS gmclash DESTINATION ${OUTPUT})
install(TARGETS fillHistogramExample DESTINATION ${OUTPUT})
install(TARGETS gmgeantino DESTINATION ${OUTPUT})
install(TARGETS testMagneticField DESTINATION ${OUTPUT})
install(TARGETS gmtogdml DESTINATION ${OUTPUT})
install(FILES ${FULLSIMLIGHT_SCRIPTS} DESTINATION share/FullSimLight)