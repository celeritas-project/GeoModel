# MinCeleritas Plugin

Smallest possible example of enabling Celeritas offload from Geant4 as a
FullSimLight User Action Plugin. 

See the main [GPUPlugins README](../README.md) for setup and build instructions, the
code here has no additional requirements.

A basic FullSimLight config file to [load and run this plugin](celer-testem3.fsl.json) is provided, 
and is configured by CMake into the build directory so that it can be run directly from there as:

```console
$ ./bin/fullSimLight -c ./FullSimLight/Plugins/GPUPlugins/MinCeleritas/celer-testem3.fsl.json
```

This uses a GDML file for the canonical TestEM3 50 layer sampling calorimeter as the geometry.
No scoring is provided, and explicitly disabled in the [Celeritas `SetupOptions`](MinCeleritas/Celeritas.cc).
Other GDML files can of course be explored by adapting the `Geometry` parameter in the input JSON file.
