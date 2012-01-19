#ifndef Config_H
#define Config_H

#include <string>
#include <stdlib.h>
#include <stdio.h>

static const std::string s_ReleaseVersion("0.60");
static const std::string s_DateOfRelease("Oct 2011");
static const float s_fVersion( atof( s_ReleaseVersion.c_str() ) );

#define OPTION (3)  

// (1) WII SEND
// (2) WII EMULATOR ... dolphin emu has been improved, opt 2 is not really needed now
// (3) WII NATIVE ... *** MAKE SURE YOU USE OPTION 3 FOR THE FINAL RELEASE ***

//--------------------------------------------------------------------------------------------------------------
// This config changes the game path dir so its no longer relative to the executable.
// This allows you to launch ‘wiiload.exe’ on the PC that in turn pipes (wirelessly) the compiled 
// code *.dol file over to the Wii.  The Wii's homebrew channel needs to be running to recieve. 
//--------------------------------------------------------------------------------------------------------------

#if (OPTION==1)
#define LAUNCH_VIA_WIISEND 
#elif (OPTION==2)
#define LAUNCH_VIA_WII_EMULATOR 
#else
#define LAUNCH_VIA_WII	
#define BUILD_FINAL_RELEASE
#endif

#define ENABLE_SOUND

#endif