# kdoom - DOOM for the Amazon Kindle

WIP port of DOOM to the Kindle (seriously, why hasn't anyone done this yet?)

## Todo

* ~~Scale properly to the screen~~
* Input (either touch or kb idrc)
* Test on something other than my (BomberFish's) Paperwhite 4

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
* A compatible toolchain. See [here](https://www.mobileread.com/forums/showthread.php?t=348710) (sources and prebuilts)

Just run the following:
* `git clone https://github.com/MercuryWorkshop/kdoom --recurse-submodules`
* `make package`

After that, just follow the installation instructions in "Setup".

## Credits

* Maxime Vincent - fbDOOM (upstream source)
* NiLuJe - FBInk (e-ink drawing library)
* Id Software (the game, duh)
