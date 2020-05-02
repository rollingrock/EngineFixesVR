# EngineFixesVR
Port of SSE Engine Fixes from Skyrim SE for Skyrim VR.  https://www.nexusmods.com/skyrimspecialedition/mods/17230

# Current working patches:
- Max STDIO patch
- Enabled AchievementsWithMods
- Form Caching
- TreeLODReference Caching

# Current stable fixes:
- PatchMemoryAccessErrors
- PatchLipSync
- PatchGHeapLeakDetectionCrash
- PatchAnimationLoadSignedCrash
- PatchBSLightingAmbientSpecular
- PatchCalendarSkipping
- PatchConjurationEnchantAbsorbs
- PatchEquipShoutEventSpam
- PatchRemovedSpellBook

# Install

To install drop in your SKSE/Plugins the dll and ini file.   Configure the ini file if you want to disable features.

Also the "(Part 2) Engine Fixes - skse64 Preloader and TBB Lib" zip file needs to be extracted to your main SkyrimVR folder or the plugin will not load correctly

# Credits

All I have done here is fix offsets and update some code to match the VR binary.   All code design and original ideas credit should go to aers and the people they list in the mod page credits:

- aers - original SSE mod author
- Nukem -  more stuff than I can mention
- Sniffleman/Ryan - Misc Fixes + CommonLibSSE 
- meh321 - research into tree LOD function (SSE fixes), bugfixes LE & port permissions
- sheson - skse plugin preloader for LE alongside meh
- himika - scatter table implementation from libskyrim (LE), plus tons of research function/variable names
- kassent - useful information from the source code of various skse plugins 
- Kole6738 - cosave cleaner idea+code
- LStewieAL - Things ported from here

# Tools used

Ghidra

Cheat Engine

Reclass

Visual Studio

SKSE Source
