#include <math.h>
#include "Util.h"
#include "config.h"
#include "ogc\lwp_watchdog.h"
#include "ogc\system.h"
#include "ogcsys.h"
#include <sstream>
#include <iomanip>

//-------------------------------------------------
// Reset / Power off - ok some globals... I am looking for a nicer way.
int  g_PowerOffMode(SYS_POWEROFF);  
bool g_bEnablePowerOffMode(false); 
//--------------------------------------------------

void Trigger_Reset_Callback(void)			{ g_PowerOffMode = SYS_RESTART;  g_bEnablePowerOffMode=true; } 
void Trigger_Power_Callback(void)			{ g_PowerOffMode = SYS_POWEROFF; g_bEnablePowerOffMode=true; }  
void Trigger_PadPower_Callback( s32 chan )	{ g_PowerOffMode = SYS_POWEROFF; g_bEnablePowerOffMode=true; } 

bool Util::IsPowerOff()
{
	return g_bEnablePowerOffMode;
}

void Util::StringToLower(std::string& StringValue)
{

#warning *** UPDATE THIS LINE ***

//#include <algorithm>
	// std::transform(data.begin(), data.end(), data.begin(), ::tolower); 

	for (int i=0; i<(int)StringValue.size();++i)
	{
		StringValue[i] = tolower(StringValue[i]);
	}
}


GXColor Util::Colour(u8 r,u8 g,u8 b,u8 a)
{
	GXColor Colour;
	Colour.a=a;
	Colour.b=b;
	Colour.g=g;
	Colour.r=r;

	return Colour;
}
 
//MOVE THIS  into..  WiiFile
std::string Util::GetGamePath()
{
#ifdef BUILD_FINAL_RELEASE 
	std::string TempGamePath = ""; //  Final build - use path relative to the executable
#else 
	#warning *** DONT FORGET TO REMOVE THIS IN THE FINAL BUILDS ***
	std::string TempGamePath = "sd://apps/BoltThrower/"; // debug only
#endif

	return TempGamePath;
}

u8 Util::CalculateFrameRate()
{
    static u8 FramesPerSecondCounter(0);
    static u64 LastTime;
    static u8 FPS(0);
	u64 CurrentTime( ticks_to_millisecs( Util::timer_gettime() )); // gettick() ) );

    FramesPerSecondCounter++;
    if ( (CurrentTime - LastTime) >= 1000) 
	{
        FPS = FramesPerSecondCounter;
        FramesPerSecondCounter = 0;
		LastTime = CurrentTime;
    }
    return FPS;
}


void Util::SleepForMilisec(unsigned long milisec)
{
    time_t sec = (int)(milisec / 1000);
    milisec -= (sec * 1000);
	timespec req = { sec, (milisec * 1000000L) };
    while (nanosleep(&req) == -1)
        continue;  // keep trying until it really does pause
}

// The Wii is a "Big-Endian" system

u16 Util::ENDIAN16(u16 Value)
{
	// A big-endian machine stores the most-significant byte first (at the lowest address), 
	// and a little-endian machine stores the least-significant byte first.
	// NOTE: For things like Wavs if the file starts with RIFF, then it's little endian.
	return  (((Value&0x00ff)<<8) | ((Value&0xff00)>>8));
}

u32 Util::ENDIAN32(u32 Value)
{
	return  ( ((Value&0xff000000)>>24) | 
		((Value&0x00ff0000)>>8) | 
		((Value&0x0000ff00)<<8) |  
		((Value&0x000000ff)<<24));
}

std::string Util::NumberToString(int Value)
{
	std::stringstream stream; 
	stream << std::setw( 2 ) << std::setfill( '0' )  << Value;
	return stream.str();
}

u64 Util::TicksToMicrosecs(u64 Value)
{
	return ticks_to_microsecs(Value);
}

u64 Util::timer_gettime()
{
	return gettime();
}
