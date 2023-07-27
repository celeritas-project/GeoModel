# Troubleshooting


## GeoModelVisualization (GMEX)

### "OpenGL context could not be created"

If, after starting `gmex`, you get a window that tells you that was not possible to open an "OpenGL context", that can happen for two reasons:

* try to restart your machine. Seriously! :-) Sometimes, after updates to the graphical packages (those can happen behind the scene, without you to notice them), the system needs to be restarted. I experienced that personally: `gmex` was failing to start, when I was trying a new version of our 3D engine, Coin3D, and I was trying to find for the cause in the code of Coin3D itself... But then my OpenGL-accelerated terminal emulator, Alacritty, had stopped working as well; and that gave me a hint that something global was happening... So, I Googled, and I found a suggestion to resart the machine to get the graphics drivers sync between each others. Try that first, it can help! ;-)
* your OpenGL installation / support is not correct / fully-installed. In that case, you should look for instructions to install proper OpenGL support on your system.

### GMEX crashes on the latest Ubuntu

The GeoModelExplorer (GMEX) application appears to crash when run on the latest version of the Ubuntu "Wayland" window manager.

The problem is that the SoQt/Coin third-party packages we use in GMEX have not been ported to work to the latest Wayland yet. As far as we know, all applications that use the Coin/SoQt graphics layers as their graphics interface crash on the latest Wayland. 

The Coin3D community is aware of the problem but there are no fixes, yet.

Therefore, we strongly suggest to use the alternative X11 window manager when running GEMX (or all other SoQt/Coin3D-based applications) on Ubuntu, for the time being.



## Packages

### macOS / Homebrew

#### Cleaning your environment

**PACKAGES**

Sometimes, cleaning the system from old packages helps to solve `brew`-related issues.

Also, please clean your system from old packages before installing new packages, or new versions of the packages. This will fix installation issues, in case `brew` does not succeed to correctly upgrade the installed packages.

You can see the list of all `brew` packages installed on your system with: 

```
brew list
```

Then, you can clean the system by removing the GeoModel packages. 
To do so, just type `brew remove` and the name of the packages that need to be removed (you might be asked to use `brew uninstall` for specific cases; in that case, you will get a clear error message from `brew` about that and you should merely follow the on-screen instructions).

If you see packages like `geomodelcore` in the output of `brew list`, that means that you have old versions of the GeoModel packages installed. Please remove them and update to the latest packages; for example, by using (please remove from the commands below the packages that are not present on your system, otherwise `brew` will complain):

```bash
brew remove geomodelexplorer geomodelio geomodelcore geomodelvisualization
brew remove soqt-bb coin-bb simage
```

Also, if you see packages like `geomodel`, please remove them before trying to compile GeoModel from sources.
You can clean your system from recent versions of the packages by running:

```
brew remove geomodel geomodelvisualization
```

You can also remove the graphical libraries by running:

```
brew remove coin-geomodel soqt-geomodel simage-geomodel
```


**TAPS**

*Taps* are remote repositories used to store `brew` formulas. GeoModel has a Tap called `atlas/geomodel`, which is stored on the CERN GitLab server:
https://gitlab.cern.ch/GeoModelDev/packaging/homebrew-geomodel

In order to have a clean environment, you should remove old GeoModel related Taps.

At first, check the Taps that are installed on your systems; please, type:

```
brew tap
```

You should see at least:

```
atlas/geomodel
homebrew/cask
homebrew/core
homebrew/services
```

plus any other tap you installed on your own for other reasons.

In particular, if you see taps like the ones here below:

```
ric-bianchi/coin3d
ric-bianchi/vp1light
jfb3615/repo
atlas/graphics
```

those are old repositories containing old formulas, which conflict with the latest version of the formulas hosted in the `atlas/geomodel` tap.

Please remove those taps by typing:

```
brew untap ric-bianchi/coin3d  ric-bianchi/vp1light jfb3615/repo atlas/graphics
```


#### Error: SHA256 mismatch

If you see an output similar to the one below:

```bash
==> Installing geomodelio from atlas/geomodel
==> Downloading https://gitlab.cern.ch/GeoModelDev/GeoModelIO/-/archive/3.2.0/GeoModelIO-3.2.0.zip
Already downloaded: /Users/user/Library/Caches/Homebrew/downloads/0cc80cc8d2a1e3066725c5ee623c23c7f329a086ea892fdc7cc6dd5b226694e2--GeoModelIO-3.2.0.zip
==> Verifying 0cc80cc8d2a1e3066725c5ee623c23c7f329a086ea892fdc7cc6dd5b226694e2--GeoModelIO-3.2.0.zip checksum
Error: SHA256 mismatch
Expected: 5dcbedff90f88088af8fc6c94b16f73d1e43d02dbf49ebd2087cff3423ab20bd
  Actual: 050860303807f186a36d7caaf386be5ed0af710d3d44994184b391781a666dc3
 Archive: /Users/user/Library/Caches/Homebrew/downloads/0cc80cc8d2a1e3066725c5ee623c23c7f329a086ea892fdc7cc6dd5b226694e2--GeoModelIO-3.2.0.zip
To retry an incomplete download, remove the file above.
```

It means you had a file downloaded by brew at an earlier stage, and the hash (the *signature*) mismatches. Try to remove the old file from your system, before running the same command again.

In this example, try to remove:

```
rm /Users/user/Library/Caches/Homebrew/downloads/0cc80cc8d2a1e3066725c5ee623c23c7f329a086ea892fdc7cc6dd5b226694e2--GeoModelIO-3.2.0.zip
```

and then retry to install.


#### Moving from macOS <= 11 (BigSur) to macOS >= 12 (Monterey) on Apple M1 chip

On macOS <= 11, the Homebrew package manager installs everything under `usr/local`. On the contrary, starting from macOS 12 and on machines with the new Apple M1 chip, Homebrew installs all packages under the new, dedicated folder `/opt/homebrew`.

As a result, when migrating your Apple M1 machine from macOS 11 to 12, you might have relics in your `usr/local` folder, which can interfer with the compilation and/or installation of the GeoModel packages.

To be sure to have a clean environment, where Homebrew can install the GeoModel packages without problems, when moving your Apple M1 system to macOS 12 please make sure you don't have any GeoModel relics in `usr/local`. You can check this by running:

```sh
% ls /usr/local/lib/libGeo*
```

If you have old GeoModel-related libraries installed, you should see an output like the one below:

```sh
/usr/local/lib/libGeoGenericFunctions.4.2.6.dylib	/usr/local/lib/libGeoModelDBManager.dylib		/usr/local/lib/libGeoModelWrite.4.2.6.dylib
/usr/local/lib/libGeoGenericFunctions.4.2.8.dylib	/usr/local/lib/libGeoModelJSONParser.4.2.6.dylib	/usr/local/lib/libGeoModelWrite.4.2.8.dylib
/usr/local/lib/libGeoGenericFunctions.4.2.9.dylib	/usr/local/lib/libGeoModelJSONParser.4.2.8.dylib	/usr/local/lib/libGeoModelWrite.4.2.9.dylib
/usr/local/lib/libGeoGenericFunctions.4.dylib		/usr/local/lib/libGeoModelJSONParser.4.dylib		/usr/local/lib/libGeoModelWrite.4.dylib
/usr/local/lib/libGeoGenericFunctions.dylib		/usr/local/lib/libGeoModelJSONParser.dylib		/usr/local/lib/libGeoModelWrite.dylib
/usr/local/lib/libGeoMaterial2G4.4.2.6.dylib		/usr/local/lib/libGeoModelKernel.4.2.6.dylib		/usr/local/lib/libGeoModelXMLParser.4.2.6.dylib
/usr/local/lib/libGeoMaterial2G4.4.dylib		/usr/local/lib/libGeoModelKernel.4.2.8.dylib		/usr/local/lib/libGeoModelXMLParser.4.2.8.dylib
/usr/local/lib/libGeoMaterial2G4.dylib			/usr/local/lib/libGeoModelKernel.4.2.9.dylib		/usr/local/lib/libGeoModelXMLParser.4.dylib
/usr/local/lib/libGeoModel2G4.4.2.6.dylib		/usr/local/lib/libGeoModelKernel.4.dylib		/usr/local/lib/libGeoModelXMLParser.dylib
/usr/local/lib/libGeoModel2G4.4.dylib			/usr/local/lib/libGeoModelKernel.dylib			/usr/local/lib/libGeoModelXml.4.2.6.dylib
/usr/local/lib/libGeoModel2G4.dylib			/usr/local/lib/libGeoModelRead.4.2.6.dylib		/usr/local/lib/libGeoModelXml.4.2.8.dylib
/usr/local/lib/libGeoModelDBManager.4.2.6.dylib		/usr/local/lib/libGeoModelRead.4.2.8.dylib		/usr/local/lib/libGeoModelXml.4.dylib
/usr/local/lib/libGeoModelDBManager.4.2.8.dylib		/usr/local/lib/libGeoModelRead.4.2.9.dylib		/usr/local/lib/libGeoModelXml.dylib
/usr/local/lib/libGeoModelDBManager.4.2.9.dylib		/usr/local/lib/libGeoModelRead.4.dylib
/usr/local/lib/libGeoModelDBManager.4.dylib		/usr/local/lib/libGeoModelRead.dylib
```

and:

```sh
$> ls /usr/local/lib/libGM*
/usr/local/lib/libGMXPlugin.4.2.6.dylib	/usr/local/lib/libGMXPlugin.4.2.8.dylib	/usr/local/lib/libGMXPlugin.4.dylib	/usr/local/lib/libGMXPlugin.dylib
```

and:

```sh
$> ls /usr/local/lib/libGX*
/usr/local/lib/libGXBase.4.2.9.dylib			/usr/local/lib/libGXGeometrySystems.4.2.9.dylib		/usr/local/lib/libGXGuideLineSystems.4.2.9.dylib
/usr/local/lib/libGXBase.4.dylib			/usr/local/lib/libGXGeometrySystems.4.dylib		/usr/local/lib/libGXGuideLineSystems.4.dylib
/usr/local/lib/libGXBase.dylib				/usr/local/lib/libGXGeometrySystems.dylib		/usr/local/lib/libGXGuideLineSystems.dylib
/usr/local/lib/libGXClashPointSystems.4.2.9.dylib	/usr/local/lib/libGXGui.4.2.9.dylib			/usr/local/lib/libGXHEPVis.4.2.9.dylib
/usr/local/lib/libGXClashPointSystems.4.dylib		/usr/local/lib/libGXGui.4.dylib				/usr/local/lib/libGXHEPVis.4.dylib
/usr/local/lib/libGXClashPointSystems.dylib		/usr/local/lib/libGXGui.dylib				/usr/local/lib/libGXHEPVis.dylib
```

You can check also the `/usr/local/bin` folder as well, for relics of GeoModel executables:

```sh
$> ls /usr/local/bin/gm*    
/usr/local/bin/gmcat		/usr/local/bin/gmex		/usr/local/bin/gmstatistics
$> ls /usr/local/bin/full*. # for fullSimLight    
```

Therefore, before installing or compiling GeoModel or GeoModel-dependent packages, clean the `usr/local` folder, first.

To clean it, you can run, for example:

**WARNING:** Before running the commands below, which are provided as examples only, please double check that the `*` does not pick up other, valuable, non-GeoModel libraries you might have installed in `/usr/local`!!!

**NOTE:** Depending on your installation, you might need to run the commands below as `sudo` (as in the examples) or not.

```sh
sudo rm -f /usr/local/lib/libGeo*
sudo rm -f /usr/local/lib/libGM*
sudo rm -f /usr/local/lib/libGX*
sudo rm -rf /usr/local/lib/libTFPersistification.*
sudo rm -rf /usr/local/lib/libExpressionEvaluator.*
sudo rm -rf /usr/local/lib/libGDMLtoGM.*
sudo rm -rf /usr/local/lib/gxplugins
sudo rm -rf /usr/local/lib/cmake/GeoModel*
sudo rm -rf /usr/local/lib/Geant4-11.0.1
sudo rm -rf /usr/local/lib/libG4*

sudo rm -f /usr/local/bin/gm*
sudo rm -f /usr/local/bin/gdml2gm
sudo rm -f /usr/local/bin/fullSimLight 
sudo rm -f /usr/local/bin/geant4*

sudo rm -rf /usr/local/share/GeoModelXml/
sudo rm -rf /usr/local/share/gmex
sudo rm -rf /usr/local/share/Geant4-11.0.1

sudo rm -rf /usr/local/include/ExpressionEvaluator*
sudo rm -rf /usr/local/include/Geo*
sudo rm -rf /usr/local/include/GDMLInterface
sudo rm -rf /usr/local/include/GXClashPointSystems/
sudo rm -rf /usr/local/include/TFPersistification/
sudo rm -rf /usr/local/include/VP1*
sudo rm -rf /usr/local/include/Geant4 

```
