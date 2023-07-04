# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# CMake settings
cmake_minimum_required(VERSION 3.16...3.26)

# Dummy call to 'project()', needed to set 'PROJECT_SOURCE_DIR'
project( "Plugins" )

#Set up the project. Check if we build it with GeoModel or individually
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    # I am built as a top-level project.
    # Make the root module directory visible to CMake.
    list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake )
    # get global GeoModel version
    include( GeoModel-version ) 
    # set the project, with the version taken from the GeoModel parent project
    project( "ATLAS-Extensions" VERSION ${GeoModel_VERSION} LANGUAGES CXX )
    # Define color codes for CMake messages
    include( cmake_colors_defs )
    # Warn the users about what they are doing
    message(STATUS "${BoldGreen}Building ${PROJECT_NAME} individually, as a top-level project.${ColourReset}")
    # Set default build and C++ options
    include( configure_cpp_options )
    set( CMAKE_FIND_FRAMEWORK "LAST" CACHE STRING
         "Framework finding behaviour on macOS" )
    # Set up how the project handle some of its dependenices. Either by picking them
    # up from the environment, or building them itself.

    # Find the base GeoModel packages, which must be installed on the target system already
    find_package( GeoModelCore REQUIRED )
else()
    # I am called from other project with add_subdirectory().
    message( STATUS "Building ${PROJECT_NAME} as part of the root GeoModel project.")
    # Set the project
    project( "ATLAS-Extensions" VERSION ${GeoModel_VERSION} LANGUAGES CXX )
endif()


# Use the GNU install directory names.
include( GNUInstallDirs )

# Set up the build of the libraries of the project.
add_subdirectory(HitsPlugin)
add_subdirectory(Examples/UserActionPlugins)
add_subdirectory(TracksPlugin)
add_subdirectory(Examples/PhysicsListPlugins/FSLTestPhysListPlugins)
add_subdirectory(Examples/EventGeneratorPlugins/FSLExamplePrimaryGeneratorPlugin)
add_subdirectory(Examples/SensitiveDetectorPlugins/SDPlugin)
add_subdirectory(Examples/MagneticFieldPlugins/UniformMagneticFieldPlugin)

add_subdirectory(GPUPlugins)
