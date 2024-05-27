# DOOM for Amazon Kindle

WIP port of DOOM to the Kindle (seriously, why hasn't anyone done this yet?)

## Todo

* Scale properly to the screen
* Input (either touch or kb idrc)

## Setup

You will need:
* A jailbroken Kindle. You can check if your firmware is jailbreakable here: https://wiki.mobileread.com/wiki/Kindle_Firmware
* KUAL installed. Downloads are on the MobileRead forums: https://www.mobileread.com/forums/showthread.php?t=225030

Setup is easy as pie (?)
1. Connect your Kindle via USB.
2. Copy your DOOM .wad to the root of the storage device.
3. Copy the kual extension folder to the extensions/ folder.
4. Enjoy! ;)

## Building

You will need:
* Everything from the setup guide
* A Linux PC. macOS might work on a good day but you're on your own.
* A compatible toolchain. See https://www.mobileread.com/forums/showthread.php?t=348710 (sources and prebuilts)

Just run the following after cloning the repo:
* git submodule update --init --recursive
* make package

After that, copy the extension folder (./kual/doom) to the kual extensions folder on your Kindle.
