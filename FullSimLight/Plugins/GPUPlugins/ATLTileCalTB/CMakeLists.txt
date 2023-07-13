# Copyright (C) 2023 CERN for the benefit of the ATLAS collaboration

add_library(ATLTileCalTB SHARED
  #ATLTileCalTB/ATLTileCalTBActInitialization.cc
  #ATLTileCalTB/ATLTileCalTBActInitialization.hh
  ATLTileCalTB/ATLTileCalTBConstants.hh
  #ATLTileCalTB/ATLTileCalTBDetConstruction.cc
  #ATLTileCalTB/ATLTileCalTBDetConstruction.hh
  ATLTileCalTB/ATLTileCalTBEventAction.cc
  ATLTileCalTB/ATLTileCalTBEventAction.hh
  ATLTileCalTB/ATLTileCalTBGeometry.cc
  ATLTileCalTB/ATLTileCalTBGeometry.hh
  ATLTileCalTB/ATLTileCalTBHit.cc
  ATLTileCalTB/ATLTileCalTBHit.hh
  ATLTileCalTB/ATLTileCalTBPrimaryGenAction.cc
  ATLTileCalTB/ATLTileCalTBPrimaryGenAction.hh
  ATLTileCalTB/ATLTileCalTBRunAction.cc
  ATLTileCalTB/ATLTileCalTBRunAction.hh
  ATLTileCalTB/ATLTileCalTBSensDet.cc
  ATLTileCalTB/ATLTileCalTBSensDet.hh
  ATLTileCalTB/ATLTileCalTBStepAction.cc
  ATLTileCalTB/ATLTileCalTBStepAction.hh
  ATLTileCalTB/ATLTileCalTBTrackingAction.cc
  ATLTileCalTB/ATLTileCalTBTrackingAction.hh
  ATLTileCalTB/Celeritas.cc
  ATLTileCalTB/Celeritas.hh)

  # Link explicitly to Celeritas's accel and accel_final targets if the latter exists. We don't use celeritas_target_link_libraries
# as this will only link in the _final (containing device code) library in an executable.
# Target is only present with device builds, so protect with genex.
target_link_libraries(ATLTileCalTB PUBLIC Celeritas::accel $<TARGET_NAME_IF_EXISTS:Celeritas::accel_final> ${Geant4_LIBRARIES})
target_include_directories(ATLTileCalTB PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)


# Plugins
add_library(ATLTileCalTBUserActionPlugin MODULE ATLTileCalTBUserActionPlugin.cc)
target_link_libraries(ATLTileCalTBUserActionPlugin PRIVATE ATLTileCalTB FullSimLight::FullSimLight)

add_library(ATLTileCalTBSDPlugin MODULE ATLTileCalTBSDPlugin.cc)
target_link_libraries(ATLTileCalTBSDPlugin PRIVATE ATLTileCalTB FullSimLight::FullSimLight)


# Configure the FSL Json file
configure_file(celer-atltilecaltb.fsl.json celer-atltilecaltb.fsl.json @ONLY)

# TODO: Add installation commands
