# Celeritas and AdePT plugins for FullSimLight

Implementations of FullSimLight plugins for demoing and testing use of 
[Celeritas](https://github.com/celeritas-project/celeritas) and [AdePT](https://github.com/apt-sim/adept)
for offloading e-/e+/g transport to GPUs.

The current implement in [`src/CelerUserActionPlugin.cc`](src/CelerUserActionPlugin.cc)
is taken from the [Celeritas ATLAS TileCal testbed](https://github.com/celeritas-project/atlas-tilecal-integration),
but does not currently include scoring or the GDML file.

## Build and Run
The plugins are currently implemented as a subproject in GeoModel to keep things simple.
It's therefore best to get/build/run it together with GeoModel to keep everything synced.
The minimal set of options to build GeoModel, FullSimLight and this Plugin are:

```console
$ cmake \
  -DGEOMODEL_BUILD_ATLASEXTENSIONS=ON
  -DGEOMODEL_BUILD_FULLSIMLIGHT=ON
  -DGEOMODEL_USE_HEPMC3=ON
  <pathtogeommodelcheckout>
```

The only software required in addition to GeoModel's requirements are Celeritas 0.3 or newer,
compiled with Geant4 and VecGeom support. So far, the plugin has been tested only with Geant4 11.0.4
and VecGeom 1.2.3.

A very simple FullSimLight configuration file and GDML file for TestEM3 are supplied
locally. After building, running FullSimLight with the Celeritas plugin and TestEM3 may be testing using

```console
$ ./bin/fullSimLight -c ./FullSimLight/Plugins/GPUPlugins/celer-testem3.fsl.json
```

## TODOs
- Spack environment file
- Implement Sensitive Detectors for TestEM3 and ATLAS TileCal
- Use `celeritas::SetupOptionsMessenger` to allow override of hardcoded values in [`src/Celeritas.cc`](src/Celeritas.cc)
- Implement AdePT plugin
