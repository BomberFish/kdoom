=======================
===    DOOM for     ===
===  Amazon Kindle  ===
=======================

WIP port of DOOM to the Kindle (seriously, why hasn't anyone done this yet?)

Todo
===============

* Scale properly to the screen
* Input (either touch or kb idrc)

Setup
===============

You will need:
* A jailbroken Kindle. You can check if your firmware is jailbreakable here: https://wiki.mobileread.com/wiki/Kindle_Firmware
* KUAL installed. Downloads are on the MobileRead forums: https://www.mobileread.com/forums/showthread.php?t=225030

Setup is easy as pie (?)
1. Connect your Kindle via USB.
2. Copy your DOOM .wad to the root of the storage device.
3. Copy the kual extension folder to the extensions/ folder.
4. Enjoy! ;)

Building
===============

You will need:
* Everything from the setup guide
* A Linux PC. macOS might work on a good day but you're on your own.
* A compatible toolchain. See https://www.mobileread.com/forums/showthread.php?t=348710 (sources and prebuilts)

Just run the following after cloning the repo:
* git submodule update --init --recursive
* make package

After that, copy the extension folder (./kual/doom) to the kual extensions folder on your Kindle.

fbDOOM readme
===============

Adaptation of the original DOOM
to be easily portable to framebuffer devices with minimal depenencies.
Runs on:
* Plain desktop Linux framebuffer,
* Siglent SSA 3021X Spectrum Analyzer,
* Frosted OS

To build (example):
* cd fbdoom
* make CROSS_COMPILE=arm-linux-gnueabihf-


Original readme
===============

Here it is, at long last.  The DOOM source code is released for your
non-profit use.  You still need real DOOM data to work with this code.
If you don't actually own a real copy of one of the DOOMs, you should
still be able to find them at software stores.

Many thanks to Bernd Kreimeier for taking the time to clean up the
project and make sure that it actually works.  Projects tends to rot if
you leave it alone for a few years, and it takes effort for someone to
deal with it again.

The bad news:  this code only compiles and runs on linux.  We couldn't
release the dos code because of a copyrighted sound library we used
(wow, was that a mistake -- I write my own sound code now), and I
honestly don't even know what happened to the port that microsoft did
to windows.

Still, the code is quite portable, and it should be straightforward to
bring it up on just about any platform.

I wrote this code a long, long time ago, and there are plenty of things
that seem downright silly in retrospect (using polar coordinates for
clipping comes to mind), but overall it should still be a usefull base
to experiment and build on.

The basic rendering concept -- horizontal and vertical lines of constant
Z with fixed light shading per band was dead-on, but the implementation
could be improved dramatically from the original code if it were
revisited.  The way the rendering proceded from walls to floors to
sprites could be collapsed into a single front-to-back walk of the bsp
tree to collect information, then draw all the contents of a subsector
on the way back up the tree.  It requires treating floors and ceilings
as polygons, rather than just the gaps between walls, and it requires
clipping sprite billboards into subsector fragments, but it would be
The Right Thing.

The movement and line of sight checking against the lines is one of the
bigger misses that I look back on.  It is messy code that had some
failure cases, and there was a vastly simpler (and faster) solution
sitting in front of my face.  I used the BSP tree for rendering things,
but I didn't realize at the time that it could also be used for
environment testing.  Replacing the line of sight test with a bsp line
clip would be pretty easy.  Sweeping volumes for movement gets a bit
tougher, and touches on many of the challenges faced in quake / quake2
with edge bevels on polyhedrons.

Some project ideas:

Port it to your favorite operating system.

Add some rendering features -- transparency, look up / down, slopes,
etc.

Add some game features -- weapons, jumping, ducking, flying, etc.

Create a packet server based internet game.

Create a client / server based internet game.

Do a 3D accelerated version.  On modern hardware (fast pentium + 3DFX)
you probably wouldn't even need to be clever -- you could just draw the
entire level and get reasonable speed.  With a touch of effort, it should
easily lock at 60 fps (well, there are some issues with DOOM's 35 hz
timebase...).  The biggest issues would probably be the non-power of two
texture sizes and the walls composed of multiple textures.


I don't have a real good guess at how many people are going to be
playing with this, but if significant projects are undertaken, it would
be cool to see a level of community cooperation.  I know that most early
projects are going to be rough hacks done in isolation, but I would be
very pleased to see a coordinated 'net release of an improved, backwards
compatable version of DOOM on multiple platforms next year.

Have fun.

John Carmack
12-23-97