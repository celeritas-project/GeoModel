#include "Celeritas.hh"

#include <G4Threading.hh>
#include <accel/AlongStepFactory.hh>
#include <celeritas/field/UniformFieldData.hh>
#include <celeritas/io/ImportData.hh>
#include <accel/LocalTransporter.hh>
#include <accel/SetupOptions.hh>
#include <accel/SharedParams.hh>

#include <memory>

using namespace celeritas;

// Global shared setup options
SetupOptions& CelerSetupOptions()
{
  static SetupOptions options = [] {
    // Construct setup options the first time CelerSetupOptions is invoked
    SetupOptions so;

    // Set along-step factory
    so.make_along_step = celeritas::UniformAlongStepFactory();

    so.max_num_tracks = 1024;
    so.max_num_events = 10000;
    so.initializer_capacity = 1024 * 128;
    so.secondary_stack_factor = 3.0;
    so.ignore_processes = {"CoulombScat"};

    // Use Celeritas "hit processor" to call back to Geant4 SDs.
    so.sd.enabled = true;

    // Only call back for nonzero energy depositions: this is currently a
    // global option for all detectors, so if any SDs extract data from tracks
    // with no local energy deposition over the step, it must be set to false.
    so.sd.ignore_zero_deposition = true;

    // Using the pre-step point, reconstruct the G4 touchable handle.
    so.sd.locate_touchable = true;

    // Save diagnostic information
    so.output_file = "celeritas-tilecal.json";

    // Pre-step time is used
    so.sd.pre.global_time = true;
    return so;
  }();
  return options;
}

// Shared data and GPU setup
SharedParams& CelerSharedParams()
{
  static SharedParams sp;
  return sp;
}

// Thread-local transporter
LocalTransporter& CelerLocalTransporter()
{
  static G4ThreadLocal LocalTransporter lt;
  return lt;
}

// Thread-local offload interface
SimpleOffload& CelerSimpleOffload()
{
  static G4ThreadLocal SimpleOffload so;
  return so;
}
