# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Find the header and source files.
file( GLOB SOURCES src/*.cxx )
file( GLOB HEADERS GDMLInterface/*.h )

# Create the library.
add_library( GDMLtoGM SHARED ${HEADERS} ${SOURCES} )
target_link_libraries( GDMLtoGM PUBLIC XercesC::XercesC
    ExpressionEvaluator GeoModelXMLParser GeoModelCore::GeoModelKernel )
target_include_directories( GDMLtoGM PUBLIC
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> )
source_group( "GDMLInterface" FILES ${HEADERS} )
source_group( "src" FILES ${SOURCES} )
set_target_properties( GDMLtoGM PROPERTIES
   VERSION ${PROJECT_VERSION}
   SOVERSION ${PROJECT_VERSION_MAJOR} )

# Set up an alias with the same name that you would get by "finding" a pre-built
# version of the library.
add_library( GeoModelTools::GDMLtoGM ALIAS GDMLtoGM )

# Install the library.
install(TARGETS GDMLtoGM
    EXPORT ${PROJECT_NAME}-export
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT Runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT          Runtime
            NAMELINK_COMPONENT Development   # Requires CMake 3.12
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT Development
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GDMLtoGM
            COMPONENT Development
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install( FILES ${HEADERS}
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GDMLInterface
   COMPONENT Development )

# Create and install the gdml2gm executable.
add_executable( gdml2gm gdml2gm/gdml2gm.cxx )
target_link_libraries( gdml2gm PRIVATE
    GeoModelXMLParser GeoModelCore::GeoModelKernel GeoModelIO::GeoModelWrite GeoModelIO::GeoModelDBManager GDMLtoGM )
install( TARGETS gdml2gm
   EXPORT ${PROJECT_NAME}-export
   LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
   COMPONENT Runtime )
