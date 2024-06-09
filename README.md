# kdoom - DOOM for the Amazon Kindle

WIP port of DOOM to the Kindle (seriously, why hasn't anyone done this yet?)

## Todo

* ~~Scale properly to the screen~~
* ~~Figure out the optimal e-ink waveform for displaying the game~~
* Proper touch input
* Test on something other than my (BomberFish's) Paperwhite 4
* ~~Kobo?~~ (probably never happening)

## Setup

You will need:
* A jailbroken Kindle. You can check if your firmware is jailbreakable [here](https://wiki.mobileread.com/wiki/Kindle_Firmware).
* KUAL installed. Downloads can be found [here](https://www.mobileread.com/forums/showthread.php?t=225030).

Setup is easy as pie (?)
1. Connect your Kindle via USB.
2. Copy your DOOM .wad to the root of the storage device.
3. Copy the kual extension folder to the extensions/ folder.
4. Enjoy! ;)

## Building from source

You will need:
* Everything from the setup guide
* A Linux PC. macOS might work on a good day but you're on your own.
* A compatible toolchain. See below.

Just run the following:
* `git clone https://github.com/MercuryWorkshop/kdoom --recurse-submodules`
* `make package`

After that, just follow the installation instructions in "Setup".

## Getting a toolchain

Amazon changed the architecture the OS uses in a recent software update. You will need a compatible toolchain to build the binaries.

### ARM soft float (FW <= 5.16.2.1.1)

* [Source](https://github.com/koreader/koxtoolchain)
* [Pre-built](https://mega.co.nz/#!yK4mRAyB!CvvSNWmzX4SSDlgtZp2f82dhJvcfB1Zoznwh4FiT4YY)

### ARM hard float (FW > 5.16.2.1.1)

* [Source](https://github.com/notmarek/koxtoolchain)
* [Pre-built](https://fw.notmarek.com/khf/hf-tc.tar.gz)

> [!IMPORTANT]  
> You will need to append `ARMHF=1` to all `make` commands when building for newer firmware versions.

## Credits

* Maxime Vincent - fbDOOM (upstream source)
* NiLuJe - FBInk (e-ink drawing library)
* Id Software (the game, duh)
