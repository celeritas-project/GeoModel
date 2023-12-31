# Set up the project.
cmake_minimum_required(VERSION 3.16...3.26)

set(CMAKE_CXX_STANDARD 17)

project( "UniformMagneticFieldPlugin" )


#Set up the project. Check if we build it with GeoModel or individually
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    # I am built as a top-level project.
    # Make the root module directory visible to CMake.
    list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )
    # get global GeoModel version
    #include( GeoModelATLAS-version ) 
    # set the project, with the version taken from the GeoModel parent project
    project( "UniformMagneticFieldPlugin" VERSION 1.0 LANGUAGES CXX )
    # Define color codes for CMake messages
    include( cmake_colors_defs )
    # Use the GNU install directory names.
    include( GNUInstallDirs )
    # Set a default build type
    include( BuildType )
    # Set default build and C++ options
    include( configure_cpp_options )
    # Print Build Info on screen
    include( PrintBuildInfo )
    # Warn the users about what they are doing
    message(STATUS "${BoldGreen}Building ${PROJECT_NAME} individually, as a top-level project.${ColourReset}")
    # Set default build and C++ options
    include( configure_cpp_options )
    set( CMAKE_FIND_FRAMEWORK "LAST" CACHE STRING
         "Framework finding behaviour on macOS" )
    # GeoModel dependencies
    find_package( FullSimLight REQUIRED )
    find_package( GeoModelCore REQUIRED )
else()
    # I am called from other project with add_subdirectory().
    message( STATUS "Building ${PROJECT_NAME} as part of the root project.")
    # Set the project
    project( "UniformMagneticFieldPlugin" VERSION 1.0 LANGUAGES CXX )
endif()



# Find the header and source files.
file( GLOB SOURCES src/*.cxx )

set(PROJECT_SOURCES ${SOURCES})

# Set up the library.
add_library(UniformMagneticFieldPlugin SHARED ${SOURCES})

find_package(Geant4 REQUIRED)

message( STATUS "Found Geant4: ${Geant4_INCLUDE_DIR}")

# Use the GNU install directory names.
include( GNUInstallDirs )

add_definitions (-DMAPPATH="${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}" )

target_link_libraries ( UniformMagneticFieldPlugin PUBLIC FullSimLight::FullSimLight ${CMAKE_DL_LIBS} ${Geant4_LIBRARIES} Eigen3::Eigen)


set_target_properties( UniformMagneticFieldPlugin PROPERTIES
		       VERSION ${PROJECT_VERSION}
		       SOVERSION ${PROJECT_VERSION_MAJOR} )



install( TARGETS UniformMagneticFieldPlugin
	 LIBRARY DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/FullSimLight/MagneticFieldPlugins
	 COMPONENT Runtime
	 NAMELINK_COMPONENT Development )



