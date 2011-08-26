#ifndef DEBUG_H_
#define DEBUG_H_

#include "Debug.h"
#include "GCTypes.h"

#include <gccore.h>
#include <wiiuse/wpad.h>

#include <stdarg.h> 
#include <stdio.h> 
#include <stdlib.h> 

#include "Singleton.h"
#include "WiiManager.h"

/*!
\namespace	Debug
\brief		A namespace to hold debug functions to be used for wherever.
*/
void Debug::_ExitPrintf(const char* pFormat, ...)
{
	static const u32 BufferSize(128);

	va_list tArgs;
	va_start(tArgs, pFormat);
	char cBuffer[BufferSize+1];
	vsnprintf(cBuffer, BufferSize, pFormat, tArgs);	
	va_end(tArgs);

	if (Singleton<WiiManager>::GetInstanceByPtr()!=NULL) // we may be calling from the constructor or maybe its not even got that far
	{
		if (Singleton<WiiManager>::GetInstanceByRef().GetScreenBufferId() == 1)  // switch to buffer 0, so printf outputs to buffer ZERO
		{
			Singleton<WiiManager>::GetInstanceByRef().SwapScreen();
		}
	}
	printf("\n\n\t%s\n*** Press 'Home' *** \n\n",cBuffer);

	do 
	{
		VIDEO_WaitVSync();
		WPAD_ScanPads();
	} while( (WPAD_ButtonsUp(0) & WPAD_BUTTON_HOME)== 0 );

	printf("\nExiting\n\n");

	exit(1);

}


#endif
