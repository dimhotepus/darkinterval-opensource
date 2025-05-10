DARK INTERVAL Source Code

1. Contents

This repository contains a copy of Dark Interval's development repository.
It includes everything needed to build the client, server, and shader DLLs of the mod.

2. Building

The build process for the most part is the same as with Source SDK Base 2013 Singleplayer.
One notable point of difference is that Dark Interval got rid of VPC mechanism, instead
shipping the ready solution and project files. This was done largely due to problems and 
annoyances VPC was causing when working with Visual Studio 2015 (which remains the primary
software used by Dark Interval devs).

For building Source SDK Base 2013 Singleplayer, see 
https://developer.valvesoftware.com/wiki/Setting_up_Source_SDK_Base_2013_Singleplayer
Namely, building it requires v120_xp toolset. It is intended that you
do not update/retarget the projects to a toolset other than v120_xp
(however, you can do that if you know what you're doing).

The build was tested on VS 2013 and VS 2015.

After successfully building the solution, the DLLs should appear in 
game/client/_compiled_bin
game/server/_compiled_bin
materialsystem/_compiled_bin
and the bin/ folder of the repository, next to src/.

3. Notes

The first point of difference is that this repository is not forked from Valve's SDK 2013 repo.
This was done for two reasons: to reduce the bloat (as Valve's repo used to include both
MP and SP code together, plus a number of unnecessary libraries and rarely used source code),
and to reduce possible interference from Valve's updates. Since then, Valve has split their repo
so the SP branch is available without MP bloat, but this was done only very recently, whereas 
DI has been in development for years.
As such, Dark Interval repo could be considered "standalone" in that it does not tie into Valve's
Github repo. Still, it is subject to Valve's original license which is included.

Now on the content differences. 
There was a lot of effort being put into separating DI changes and documenting them at least briefly.
All the DI changes use the preprocessor block #ifdef DARKINTERVAL or #ifndef DARKINTERVAL.
There're some exceptions made for brevity (as in not being ifdef'd), including:
* nearly all Xbox/Xbox 360 code has been stripped away;
* nearly all CS/TF2/HL2DM/DOD:S/L4D code included in Valve's SDK, has been stripped away;
* the shader sources do not use #ifdef DARKINTERVAL due to the way they are compiled;
* all instances of gpGlobals->curtime have been replaced with the macro CURTIME (see shareddefs.h);
* checks for sv_cheats and FCVAR_CHEAT flags have been stripped from most
convars;

Also:
* numerous console commands and variables have been disabled (ifndef'd out) in cases
where they weren't likely to ever be needed to change;
* VR related SDK code has been ifndef'd out, and its files removed - to re-enable it
you'll need to copy them over from Valve's SDK;
* with the project being strictly singleplayer, teams have been ifndef'd out and 
the files removed, such as team.h, c_team.h and others.
To re-enable it for whichever reason, those files will also need to be copied over
from Valve's SDK;
* numerous files were renamed for consistency, such as combine_mine.cpp -> npc_combine_mine.cpp,
explosion.cpp/h -> env_explosion.cpp/h, and all the file names in the game code have been
made lowercase whenever they were not, with all the header includes also being in lowercase;
* several of HL2 classes have been moved from their own files into 'encompassing' files,
namely _weapons_darkinterval.cpp;
* numerous unused files have been removed, such as server/sdk, client/sdk, shared/sdk, old 
entities (cut HL1 and HL2 weapons, NPCs, items that were included in the SDK);

If you are interested in the 'live ragdolls' code, you can find all of it by searching for the comment
'DI NEW: Live Ragdolls Experiment'.

4. On the leak code

There are a few instances of 2003 leak code being used. Specifically, npc_antlionguard.cpp includes
a number of cut AI routines copied over from 2003 (they are commented as such). view.cpp and viewrender.h
include copied engine code for skybox drawing as a fix for skybox culling at high FOV.

Also, a large portion of Portal's source code is included as well, as it is intended to be used in future
Dark Interval installments. It is all put inside of game/server/portal, game/client/portal and game/shared/portal folders and 
separated inside the solution, so it is easy to ignore it.

5. Third party sources

Dark Interval is built using selective fixes and features from the following sources:
Transmissions: Element 120 - the HEV locator code
https://github.com/shokunin000/te120
Estranged: Act 1 - the menu override
https://github.com/alanedwardes/Estranged-Act-1
Mapbase - numerous fixes such as model decals, flashlight improvements, camera/monitor improvements
https://github.com/mapbase-source
Alien Swarm - shader sources such as flowmapped water, parallax, object motion blur
https://github.com/Source-SDK-Archives/source-sdk-alien-swarm

6. Usage and crediting

First and foremost Dark Interval code, being based on Valve's SDK code, is subject to included Valve's license.
As for the custom DI code, you are free to use it in your non-commercial projects if you give credit to
the original source. You can make forks and redistribute this code if you credit and link the original source.
