#ifndef Config_H
#define Config_H


//--------------------------------------------------------------------------------------------------------------
// NOTE 1:
// *** Running via PC Development mode ***
// Comment out ***BUILD_FINAL_RELEASE*** to run the complied code on the Wii via wireless.
//
// This works by changing the game path dir so its no longer relative to the executable.
// This allows you to launch ‘wiiload.exe’ on the PC that in turn pipes (wirelessly) the compiled 
// code *.dol file over to the Wii.  The Wii's homebrew channel needs to be running to recieve. 
//
//--------------------------------------------------------------------------------------------------------------
// NOTE 2:
// For final release make sure 'BUILD_FINAL_RELEASE' is included
//
//--------------------------------------------------------------------------------------------------------------
//
// NOTE 3:
// Running under Wii 'Dolphin' emulator - must include BUILD_FOR_EMULATOR
// This a is as a work around to run under the emu, sorry but it disables all sounds
//
//--------------------------------------------------------------------------------------------------------------

#define LAUNCH_VIA_WIISEND 
//#define LAUNCH_VIA_WII_EMULATOR 
//#define LAUNCH_VIA_WII	


#ifdef LAUNCH_VIA_WIISEND
#undef BUILD_FINAL_RELEASE
#undef BUILD_FOR_EMULATOR
#endif

#ifdef LAUNCH_VIA_WII_EMULATOR
#undef BUILD_FINAL_RELEASE
#define BUILD_FOR_EMULATOR
#endif

#ifdef LAUNCH_VIA_WII
#define BUILD_FINAL_RELEASE
#undef BUILD_FOR_EMULATOR
#endif


#ifdef BUILD_FOR_EMULATOR
#define DISABLE_SOUND
#undef BUILD_FINAL_RELEASE
#else
#define ENABLE_SOUND
#endif


#ifndef BUILD_FINAL_RELEASE
#warning *** DONT FORGET TO CHANGE THIS DEFINE FOR RELEASE BUILDS ***
#endif

#endif