# ATLTileCalTB Plugin

Minimal fullsimlight-ization of [celeritas-project/atlastilecal-integration](https://github.com/celeritas-project/atlas-tilecal-integration/)
including scoring.

# Setup and Run
See the main [GPUPlugins README](../README.md) for setup and build instructions, the
code here has no additional requirements.

A basic FullSimLight config file to [load and run this plugin](celer-atltilecaltb.fsl.json) is provided, and is configured
by CMake into the build directory so that it can be run directly from there as:

```console
$ ./bin/fullSimLight -c ./FullSimLight/Plugins/GPUPlugins/ATLTileCalTB/celer-atltilecaltb.fsl.json
```

This file's `g4ui_commands` section simply copies [`single.mac`](https://github.com/celeritas-project/atlas-tilecal-integration/blob/main/single.mac)
from [celeritas-project/atlastilecal-integration](https://github.com/celeritas-project/atlas-tilecal-integration/).
It can be used as a template to create more complex workflows, and has been tested
both in CPU-only and CPU-GPU modes.

# Conversion for use as FullSimLight Plugin
## Required Code Adaptions
FullSimLight plugins only allow control of User Action construction and creation
and link up of Sensitive Detectors. There is no control of the Detector Construction
or Action Initialization components of Geant4, and therefore the plugins must workaround:

1. Connecting up the user actions that `ATLTileCalTB` relies on: 

   ```cpp
   void ATLTileCalTBActInitialization::BuildForMaster() const
   {
     auto EventAction = new ATLTileCalTBEventAction(nullptr);
     SetUserAction(new ATLTileCalTBRunAction(EventAction));

     CelerSimpleOffload().BuildForMaster(&CelerSetupOptions(), &CelerSharedParams());
   }

   void ATLTileCalTBActInitialization::Build() const
   {
     auto PrimaryGenAction = new ATLTileCalTBPrimaryGenAction();
     auto EventAction = new ATLTileCalTBEventAction(PrimaryGenAction);

     SetUserAction(PrimaryGenAction);
     SetUserAction(new ATLTileCalTBRunAction(EventAction));
     SetUserAction(EventAction);
     SetUserAction(new ATLTileCalTBStepAction(EventAction));

     CelerSimpleOffload().Build(&CelerSetupOptions(), &CelerSharedParams(), &CelerLocalTransporter());
     SetUserAction(new ATLTileCalTBTrackingAction());
   }
   ```

   This is done in [`ATLTileCalTBUserActionPlugin.cc`](ATLTileCalTBUserActionPlugin.cc) via

   - The constructor of the plugin class calls `CelerSimpleOffload().Build...` as appropriate for
     the thread the plugin is constructed on. FullSimLight instantiates the plugin class in `FSLActionInitialization::Build()`
     and in `FSLActionInitialization::BuildForMaster()` so the cases are distinguishable.
   - Where the `ATLTileCalTB` actions depend on each other, function-local statics are used
     to avoid duplicating instances.

2. Connecting Logical Volumes to their Sensitive Detectors:

   ```cpp
   void ATLTileCalTBDetConstruction::ConstructSDandField()
   {
     // Sensitive detectors
     //
     auto caloSD = new ATLTileCalTBSensDet("caloSD", "caloHitsCollection");
     G4SDManager::GetSDMpointer()->AddNewDetector(caloSD);

     // Assign to logical volumes
     //
     auto LVStore = G4LogicalVolumeStore::GetInstance();
     for (auto volume : *LVStore) {
       if (volume->GetName() == "Tile::Scintillator") volume->SetSensitiveDetector(caloSD);
     }

     // No fields involved
   }
   ```

   This is done in [`ATLTileCalTBSDPlugin.cc`](ATLTilCalTBSDPlugin.cc) via:

   - Using the constructor/`addLogicalVolumeName` member function to store the LV names that
     should use the SD
   - Implementing `getHitCollectionName` to return the hits collection string id
   - Implementing `getSensitiveDetector` to return a new instance.

   FullSimLight will use this information in its internal `ConstructSDandField` (it
   is basically identical to the above `ATLTileCalSD` implementation).

In addition, the design and interface
of FullSimLight plugins means that the User Action plugin is in one library, and Sensitive Detector

## Required Build/Link Adaptions
At present, FullSimLight's plugin mechanism only supports one plugin class of one type 
per plugin library. This means that we:

1. Need one plugin library per Sensitive Detector type, even if they are all for the same experiment
2. Cannot put Sensitive Detector and User Action plugins into the _same_ plugin

ATLTileCalTB's Actions and Sensitive Detectors are somewhat coupled through use of
shared functionality for things like geometry constants, and the original implementation
is thus a single application/library. The workaround adopted for use in FullSimLight is to:

1. Build a final, device linked, shared library from the implementation classes
2. Build a User Action Plugin module library for _only_ the plugin class that privately links to that library
3. The same thing but for the Sensitive Detctor plugin class

All device code should thus be hidden inside the underlying shared library, albeit it gets loaded more than
once per process. Basic running as demonstrated above functions without warning or error so far as explored.

*TODO**: A hack, but see if we can only build _one_ library containing _all_ the plugins, but use softlinks
to alias for each plugin. Longer term, better plugin mechanism!
