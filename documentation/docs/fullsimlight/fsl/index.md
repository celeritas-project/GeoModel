# fsl

fsl is a GUI for FullSimLight introduced in GeoModel release 4.3.0. It allows users to configure their simulations in FullSimLight easily and without conflicts by providing a series of tabs which offer various configuration options. It can be ran from the command line through the fsl command.

```bash
./fsl
```

 Compatability of the simulation is ensured by the fsl interface by enabling and disabling options based on the selections made. Once the user has configured the simulation to their desire, they can save the configuration in a json file that can be used to run FullSimLight through the -c flag. 
 
```bash
./fullSimLight -c /path/to/config.json
```
 
Alternatively the user can run FullSimLight at any time within fsl itself with the current configuration. fsl also allows users to load in previously saved configurations through the **Open Configuration** option. 
One can also load in a configuration by running fsl with the -c flag.

```bash
./fsl -c /path/to/config.json
```


## Main Tab


{{ imgutils_image_caption('fsl-main.png', 
   alt='fsl', 
   cap='fsl main tab',
   width ='700px',
   urlFix=False) 
}}

The main tab allows configuration of the following parameters.

- Geometry Input
- Physics List
- Number of Threads
- Number of Events
- G4UI Verbosity Commands

It also contains the view button which shows the current configuration on the main display. The user can also run FullSimLight in the main display at any time with the current configuration by clicking on the FullSimLight button in the main tab. fsl provides similar buttons to run gmex and gmclash once a geometry input has been selected through the interface.


## Generator Tab


{{ imgutils_image_caption('fsl-gen.png', 
   alt='fsl', 
   cap='fsl generator tab',
   width ='700px',
   urlFix=False) 
}}

The generator tab shown above contains a list of event generators that can be selected for event generation. The options provided are

- Particle Gun
- Pythia
- HepMC3 File
- Generator Plugin

### 1. Particle Gun

To use a particle gun, the user must specify the particle type and momentum vector.

### 2. Pythia

To use Pythia one can select one of the pre-customized options which are

- ttbar
- higgs
- minbias

Alternatively one can also provide a custom file. 

### 3. HepMC3 File

HepMC3 files containing events can be used in FullSimLight in the both the standard Asciiv3 format (introduced in HepMC3) as well as the old Ascii format (used in HepMC and HepMC2).

### 4. Generator Plugin

In order to generate events one can also specify a plugin. Visit the Plugin Support page for more details.

## Magnetic Field Tab


{{ imgutils_image_caption('fsl-mag.png', 
   alt='fsl', 
   cap='fsl magnetic field tab',
   width ='700px',
   urlFix=False) 
}}

To set a Magnetic Field, there are two options

- Fixed Axial
- MAgnetic Field Plugin


### 1. Fixed Axial

Sets a constant Magnetic field in the z-direction at the specified magnitude. 

### 2. Magnetic Field Plugin

In order to generate a Magnetic Field one can also specify a plugin. Visit the Plugin Support page for more details. A custom ATLAS Magnetic Field Plugin comes with the GeoModel package ready to use. This is automatically contained in the atlas-conf.json configuration file provided by GeoModel. ***Warning: This Plugin requires the ATLAS Magnetic Field Map to be installed in a standard location. This Map is available to all ATLAS users on request.***


## Regions Tab


{{ imgutils_image_caption('fsl-reg.png', 
   alt='fsl', 
   cap='fsl regions tab',
   width ='700px',
   urlFix=False) 
}}

Regions can be added through the regions tab in FSL as shown above. For each region one must specify 

- Region name
- Root Logical Volumes
- Electron cut (GeV)
- Proton cut (GeV)
- Positron cut (GeV)
- Gamma cut (GeV)

A list of ATLAS specific regions comes in the atlas-conf.json condfiguration file provided by GeoModel.

## Sensitive Detectors Tab


{{ imgutils_image_caption('fsl-sen.png', 
   alt='fsl', 
   cap='fsl sensitive detector tab',
   width ='700px',
   urlFix=False) 
}}

In order to add Sensitive Detectors one can create plugins which can then be added to the simulation through the menu shown above. Visit the Plugin Support page for more details.


## User Actions Tab


{{ imgutils_image_caption('fsl-user.png', 
   alt='fsl', 
   cap='fsl user action tab',
   width ='700px',
   urlFix=False) 
}}

In order to add User Actions one can create plugins which can then be added to the simulation through the menu shown above. Visit the Plugin Support page for more details.