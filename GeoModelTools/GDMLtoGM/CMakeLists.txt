# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Find the header and source files.
file( GLOB SOURCES src/*.cxx )
file( GLOB HEADERS GDMLInterface/*.h )

# Create the library.
add_library( GDMLtoGM SHARED ${HEADERS} ${SOURCES} )
target_link_libraries( GDMLtoGM PUBLIC XercesC::XercesC
   ExpressionEvaluator GeoModelXMLParser GeoModelKernel )
target_include_directories( GDMLtoGM PUBLIC
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> )
source_group( "GDMLtoGM" FILES ${HEADERS} )
source_group( "src" FILES ${SOURCES} )
set_target_properties( GDMLtoGM PROPERTIES
   VERSION ${PROJECT_VERSION}
   SOVERSION ${PROJECT_VERSION_MAJOR} )
if( GEOMODEL_USE_BUILTIN_XERCESC )
   add_dependencies( GDMLtoGM XercesC )
endif()

# Install the library.
install( TARGETS GDMLtoGM
   EXPORT ${PROJECT_NAME}-export
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   COMPONENT Runtime
   NAMELINK_SKIP )
install( TARGETS GDMLtoGM
   EXPORT ${PROJECT_NAME}-export
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   COMPONENT Development
   NAMELINK_ONLY )
install( FILES ${HEADERS}
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GDMLInterface
   COMPONENT Development )