# ProStreet Open Limit Adjuster

Experiment for Need for Speed: ProStreet that removes or increases certain memory limits

## Installation

- Make sure you have v1.3 of the game, as this is the only version this plugin is compatible with. (exe size of 6029312 bytes)
- Patch your game executable with the [4GB patch](https://ntcore.com/4gb-patch/).
- Plop the files into your game folder, edit `NFSPSOpenLimitAdjuster_gcp.toml` to change the options to your liking.
- Enjoy, nya~ :3

Please note that this mod is a proof of concept and does not increase the limit of actual cars in a race, only vehicle counts in general

## Building

Building is done on an Arch Linux system with CLion and vcpkg being used for the build process. 

Before you begin, clone [nya-common](https://github.com/gaycoderprincess/nya-common), [nya-common-nfsps](https://github.com/gaycoderprincess/nya-common-nfsps) and [CwoeeMenuLib](https://github.com/gaycoderprincess/CwoeeMenuLib) to folders next to this one, so they can be found.

Required packages: `mingw-w64-gcc`

You should be able to build the project now in CLion.
