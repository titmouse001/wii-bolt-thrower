#ifndef Config_H
#define Config_H

#include <string>
#include <stdlib.h>
#include <stdio.h>

static const std::string s_ReleaseVersion("0.70");
static const std::string s_DateOfRelease("July 2012");
static const float s_fVersion( atof( s_ReleaseVersion.c_str() ) );

#define OPTION (1)  

// (1) WII SEND
// (2) WII EMULATOR ... for the dolphin emu
// (3) WII NATIVE ... *** MAKE SURE YOU USE OPTION 3 FOR THE FINAL RELEASE ***

//--------------------------------------------------------------------------------------------------------------
// This config changes the game path dir so its no longer relative to the executable.
// This allows you to launch ‘wiiload.exe’ on the PC that in turn pipes (wirelessly) the compiled 
// code *.dol file over to the Wii.  The Wii's homebrew channel needs to be running to recieve. 
//--------------------------------------------------------------------------------------------------------------

#if (OPTION==1)

#define LAUNCH_VIA_WIISEND 
static const std::string  s_MasterFileLatestVersion ( "LatestVersion_FAKE" ); // <<< new testing file from now on

#elif (OPTION==2)

#define LAUNCH_VIA_WII_EMULATOR 
static const std::string  s_MasterFileLatestVersion ( "LatestVersion_FAKE" ); // <<< new testing file from now on

#else

#define LAUNCH_VIA_WII	
#define BUILD_FINAL_RELEASE

static const std::string  s_MasterFileLatestVersion ( "LatestVersion" ); // <<< release

// NOTE ABOUT LatestVersion_TESTING
// static const std::string  s_MasterFileLatestVersion ( "LatestVersion_TESTING.xml" );  // release 0.61 used this - so it needs to say on web site!!!

#endif

#define ENABLE_SOUND

#endif