# AnyRegion_Changer
An import of tona's AnyRegion Changer.  All rights go to its original owner.

# tona's README
AnyRegion Changer v1.1b
by tona

Description:

This program will let you edit any and all of the region information on your 
Wii, and allow you to install System Menu 3.2 from any region if you so
desire. The setting changes are the result of a setting change library I've
been working on for a while called sysconf.c. The System Menu Installer
uses patchmii as a simple base for web installations. It also uses some
IOS patches (by bushing/waninkoko) in the event that the system menu 
or IOS30 needs to be downgraded. 

The setting changes are non-temporary, i.e. they persist after system
reboot. You should have the Homebrew Channel installed so you can 
revert any changes pre or post reboot.

Precautions:

This software comes with NO WARRANTY WHATSOEVER. You may use this
software at your own risk. I can take no responsibility for any damage
caused by this application.

Please take note of the following:
-If setting.txt is corrupted for any reason, it is entirely possible that 
  your Wii will no longer boot. 
-If you change the GAME ("Game Region") setting, you may no longer be 
  able to boot discs!
-Be careful not to change Video mode to a mode unsupported by your
  display.
-If you change your Country code, you will be unable to receive any 
  pending gifts you have in the Wii Shop Channel. (The shop will also
  warn you of this)
  
===IMPORTANT===
-Setting the AREA setting ("Console Area Setting") to a different region 
  than your System Menu WILL cause a Semi-Brick! If for some reason 
  your System-Configured flag is unset (via some installation, or a 
  corrupted SYSCONF) your system WILL become FULLY BRICKED. 

Compiling:

To compile this, you will need libogc cvs with the patch in libogc_patches
applied.

License:

Patchmii is licensed under the GPLv2, and as such a copy of the license is
included. The non-patchmii material is under a BSD-like license.

Credits:

bushing, svpe, and everyone else for their wonderful work on patchmii.
marcan for the original conf.c
Waninkoko for his title_version check IOS patch
crediar for adding ES_OpenTitleContent to libogc
Anyone brave enough to test for me. ChipD, SoraK05, crediar.
serlex, pcg, callmebob, and dieforit, who sent me some interesting setting.txt
SoraK05 who tested a Korean->PAL Wii region change

Have fun guys :) 
If you see me on IRC, yell at me for trying to have a social life

Changelog:
1.1b:
-No longer cares whether EULA has been set or not
-Now downloads the correct IOS35 from nus, a fix for post 10/23/08
1.1:
-Minor fix to sysconf lib (Should fix some -24578 errors)
-Changed Identification Routines
-Added System Menu Region detection
-Added intro screen/extra warnings

1.0:
-Initial Release

(c) 2008 tona / the internet
