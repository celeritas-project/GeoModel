//---------------------------------------------------------------------------//
//! \file Celeritas.hh
//! \brief Minimal interface to Celeritas SimpleOffload for Geant4 applications
//---------------------------------------------------------------------------//
#pragma once

#include <accel/SimpleOffload.hh>

namespace celeritas
{
class LocalTransporter;
struct SetupOptions;
class SharedParams;
}  // namespace celeritas

// Global shared setup options
celeritas::SetupOptions& CelerSetupOptions();
// Shared data and GPU setup
celeritas::SharedParams& CelerSharedParams();
// Thread-local transporter
celeritas::LocalTransporter& CelerLocalTransporter();
// Thread-local offload
celeritas::SimpleOffload& CelerSimpleOffload();
