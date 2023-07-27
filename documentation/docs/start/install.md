# Install the GeoModel Applications and Development kits

In this section, you will find instructions on how to **install** the latest **stable** version of GeoModel applications and development kits on different platforms.  

The GeoModel applications include:

* **gmex**, the geometry explorer, an interactive 3d geometry visualizer
* **gmstatistics**, tool to monitor the memory consumption
* **fullsimlight**, a command-line tool to run Geant4 simulation, and its graphical user interface, *fsl*.
* **gmclash**, a Geant4 based command-line tool too perform clash detection
* **gmgeantino**, a Geant4 based command-line tool to generate geantino maps
* **gmmasscalc**, a Geant4 based command-line tool to compute the mass of a given piece of detector geometry
* **gm2gdml** and **gdml2gm**, command line tools to convert GeoModel to and from GDML
* **gmcat**, a command line tool to assemble geometry from multiple inputs

The development kits include header files and precompiled libraries for the development of detector description code. On some platforms (macOS) the development environment is installed together with the prebuild applications and libraries.  


## Ubuntu 20.0.4 (focal, LTS) or 21.0.10 (impish) or 22.0.4 (jammy, LTS)

Add the geomodel repository to your source list: 

```
sudo add-apt-repository ppa:kaktusjoe/geomodel
sudo apt-get update
```

Install geomodel precompiled applications
```
sudo apt install geomodel-visualization  # to get gmex (the geometry explorer)
sudo apt install fullsimlight            # to get fullsimlight, gm2gdml, gmclash, gmgeantino
sudo apt install geomodel-tools          # to get gmcat, gdml2gm
```

Install the geomodel development kits (header files and precompiled libraries)
```
sudo apt install geomodel-core-dev
sudo apt install geomodel-tools-dev
sudo apt install geomodel-g4-dev
sudo apt install geomodel-visualization-dev
```

If you have outdated packages, these commands will replace those packages with the most recent version.

!!! warning 

    The GeoModelExplorer (GMEX) application appears to crash when run on the latest version of the Ubuntu "Wayland" window manager. The problem is that the SoQt/Coin third-party packages we use in GMEX have not been ported to work to the latest Wayland yet. As far as we know, all applications that use the Coin/SoQt graphics layers as their graphics interface crash on the latest Wayland. The Coin3D community is aware of the problem but there are no fixes, yet.

    We strongly suggest to use the alternative X11 window manager when running GEMX (or all other SoQt/Coin3D-based applications) on Ubuntu, for the time being.


## macOS

On macOS, we use the [Homebrew package manager](http://brew.sh) to handle and install the GeoModel packages. If you don't have Homebrew installed on your machine, you should first install it by following the instructions on the Homebrew website: <http://brew.sh>.  Please be attentive to any instructions printed after the installation procedure; there might be environment variables to set.

!!! note

    Do you have the newer Mac computer based upon the M1 chip?  Then you will need to set the environment variable
   
    ```
    export DYLD_LIBRARY_PATH=/usr/local/lib:/opt/homebrew/lib
    ```


### First installation 

#### Add the GeoModel tap 

If this is the first time you install the pre-compiled GeoModel packages through the Homebrew package manager, then first you have to add the dedicated GeoModel Homebrew repository to your local repositories' list. To do so, you can type:

```shell
brew tap atlas/geomodel https://gitlab.cern.ch/GeoModelDev/packaging/homebrew-geomodel.git 
brew update 
```

The first command will add to your local repositories' list the GeoModel Homebrew repository (a *tap*, in Homebrew jargon), which stores all the "recipes" (*Formulae* in Homebrew) to install the GeoModel libraries on macOS; this command is only needed the first time you install the `atlas/geomodel` tap. The second command makes sure that all latest versions are taken; this should be run all times you want to update or install a GeoModel package.

#### Install the GeoModel packages 

After that, you can install the pre-compiled GeoModel applications:

```shell
brew install geomodel-visualization # to get gmex (the geometry explorer)
brew install geomodel-fsl           # to get fullsimlight, the FSL GUI, gm2gdml, gmclash, gmgeantino, gmmasscalc
brew install geomodel-tools         # to get gmcat, gdml2gm
brew install geomodel-extensions-atlas # to get the ATLAS extensions: the LAr custom shape and the MagneticField plugin for FullSimLight
```

All the above packages also automatically install the core package `geomodel`, which contains the core libraries and the I/O layer. 

The `geomodel-fsl` package also installs the following packages: 

* `geomodel-fullsimlight`, which contain the standalone Geant4 based application for GeoModel
* `geomodel-geomodelg4`, which contains the translation layer between GeoModel and Geant4


The `geomodel-extensions-atlas` package installs the [ATLAS experiment's](https://atlas.cern) extensions to be used in FSL/FullSimLight; namely, a custom shape for the LAr electromagnetic calorimeter and a plugin to define a custom magnetic field for the experiment. The package also downloads and installs the magnetic field datafile needed by the plugin. Thos extensions are semi-specific to the ATLAS experiment, but they can certainly be used as foundation and example to build other extensions for other experiments.



The use of **pre-compiled packages** (*Bottles*, in the Homebrew jargon) **speeds up** and **simplify** the installation on the user's machine.

The pre-compiled *bottles* are provided for the latest macOS release, currently:

* 12 "Monterey" (Apple M1 chip and Intel chip)

For older macOS releases, Homebrew will try to compile the packages from source. If that fails, you will have to compile from source by yourself by following the instructions provided in the [development](../dev/index.md) section.

!!! note

    As part of the installation procedure on the mac, the open source version of the [**Qt**](https://www.qt.io/download-open-source) software framework is installed, unless it is already present on your system. The installation procedures prints further instructions; in particular, additional environment variables to define in your startup script. These instructions tend to get lost in the input.   Their details depend upon your system.  Be attentive, and follow the instructions  which are printed during the installation of **Qt**.  
   

!!! tip
    
    If you experience problems in installing GeoModel packages with brew or using them, please try to clean your system from old packages or repositories. For that, please refer to the [Troubleshooting page](../dev/troubleshooting.md).


### Update

#### Quick instructions 

If your installation is outdated, you can update the GeoModel libraries and tools to the latest version simply by running the commands here below:

```shell
brew update
brew upgrade geomodel geomodel-geomodeltools geomodel-visualization geomodel-geomodelg4 geomodel-fullsimlight geomodel-fsl geomodel-extensions-atlas
```

#### Some more details 

After the first command, `brew update`, you get a summary of the updated repositories and recipes. If there are new versions of the GeoModel tools and libraries, you should see something like the output messages here below:

```
➜  ~ brew update
Updated Homebrew from e13d04c1d to 784be839b.
Updated 5 taps (homebrew/core, homebrew/cask, homebrew/bundle, homebrew/services and atlas/geomodel).
...
==> Updated Formulae
...
atlas/geomodel/geomodel                     fbthrift                                    libvdpau                                    sdcc
atlas/geomodel/geomodel-fullsimlight        feroxbuster                                 lilv                                        serverless
atlas/geomodel/geomodel-geomodelg4          fheroes2                                    localstack                                  snakemake
atlas/geomodel/geomodel-tools               fizz                                        lua-language-server                         snort
atlas/geomodel/geomodel-visualization
...
```

You can now update the GeoModel recipes with `brew upgrade ...`. 


!!! note

    Please notice that, if you get an error like the one below:

    ```shell
    ➜  ~ brew upgrade geomodel
    Error: atlas/geomodel/geomodel not installed
    ```

    that means that, even if the GeoModel repository (`tap`) has been installed, at a certain point the `geomodel` Homebrew package has been removed (for example, to test other versions). Therefore, the `upgrade` command cannot work. You should re-install the package with `brew install geomodel`. The same with all the other GeoModel Homebrew packages.





## Installation from source code.

On any other platform (such as Centos or Fedora), you can build GeoModel from sources, following [these instructions](../dev/index.md)



## Visualize geometry data

After installinmg the tools, you can test your installation on an example ATLAS geometry file. 

First, download the geometry data file:

{!start/get-geometry-example-file.md!}

Now, you can interactively visualizing the geometry data by running:

```
gmex geometry_atlas.db
```


## Uninstall

If you want to uninstall GeoModel libraries and applications, you can follow the instructions below

### macOS

If you installed GeoModel by using Homebrew as suggested above, you can easily uninstall all the GeoModel packages with a single `brew` command:

```
brew remove geomodel geomodel-tools geomodel-visualization geomodel-geomodelg4 geomodel-fullsimlight geomodel-thirdparty-simage geomodel-thirdparty-coin geomodel-thirdparty-soqt
```






