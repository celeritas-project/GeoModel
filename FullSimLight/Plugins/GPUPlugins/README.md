# Celeritas and AdePT plugins for FullSimLight

Implementations of FullSimLight plugins for demoing and testing use of 
[Celeritas](https://github.com/celeritas-project/celeritas) and [AdePT](https://github.com/apt-sim/adept)
for offloading e-/e+/g transport to GPUs.

## Build and Run
The project and all plugins are implemented to build as a subproject in GeoModel to keep things simple.
It's therefore best to get/build/run it together with GeoModel to keep everything synced.
The minimal set of options to build GeoModel, FullSimLight and these Plugins are:

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
with the [MinCeleritas plugin](./MinCeleritas). After building, running FullSimLight with this:

```console
$ ./bin/fullSimLight -c ./FullSimLight/Plugins/GPUPlugins/MinCeleritas/celer-testem3.fsl.json
```

can be used as a basic smoke test.

## Implemented Plugins
- [MinCeleritas](./MinCeleritas/README.md)
  - Smallest possible example to demonstrate Celeritas tracking-based offload in a FullSimLight plugin
  - Example shows use of TestEM3 sampling calorimeter geometry
  - No scoring
- [ATLTileCalTB](./ATLTileCalTB/README.md)
  - Implementation of [ATLAS TileCal Test Beam application](https://github.com/celeritas-project/atlas-tilecal-integration/)
    as a FullSimLight plugin
  - Supplies "no beamline" geometry
  - Scoring using Celeritas and host-side sensistive detectors

## TODOs
- Spack environment file
- Implement Sensitive Detectors for TestEM3
- Use `celeritas::SetupOptionsMessenger` to allow override of hardcoded values in [`src/Celeritas.cc`](src/Celeritas.cc)
- Implement AdePT plugin
