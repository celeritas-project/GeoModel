# Copyright (C) 2023 CERN for the benefit of the ATLAS collaboration

# Important note: the library name must match that used in the plugin creator function
# i.e. if we have "libFooBar.so", then there must be a "createFooBar" `extern "C"` function
# to return a new-d instance of the plugin.
add_library(MinCeleritasUserActionPlugin MODULE
  MinCeleritas/Celeritas.hh
  MinCeleritas/Celeritas.cc
  MinCeleritas/EventAction.hh
  MinCeleritas/RunAction.hh
  MinCeleritas/TrackingAction.hh
  UserActionPlugin.cc)

# Link explicitly to Celeritas's accel and accel_final targets if the latter exists. We don't use celeritas_target_link_libraries
# as this will only link in the _final (containing device code) library in an executable.
# Target is only present with device builds, so protect with genex.
target_link_libraries(MinCeleritasUserActionPlugin Celeritas::accel $<TARGET_NAME_IF_EXISTS:Celeritas::accel_final> FullSimLight::FullSimLight ${Geant4_LIBRARIES})

# Configure the FSL Json file
configure_file(celer-testem3.fsl.json celer-testem3.fsl.json @ONLY)

# TODO: Add installation commands
