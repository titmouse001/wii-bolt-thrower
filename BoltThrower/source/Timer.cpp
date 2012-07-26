#include <gccore.h>
#include "ogc\lwp_watchdog.h"
#include "timer.h"
#include "Util.h"

Timer::Timer()
{
	SetTimerSeconds(0);
}

bool Timer::IsTimerDone()
{
	return (Util::timer_gettime() >= GetTimerTicks() );
}

void Timer::SetTimerSeconds(u32 t) 
{ 
	SetTimerTicks( Util::timer_gettime() + secs_to_ticks(t) );
}

void Timer::SetTimerMillisecs(u32 t) 
{ 
	SetTimerTicks( Util::timer_gettime() + millisecs_to_ticks(t) );
}

u32 Timer::GetTimerSeconds() 
{ 
	return ticks_to_secs(m_Timer - Util::timer_gettime());
}

u64 Timer::GetTimerMicrosecs() 
{ 
	return ticks_to_microsecs(m_Timer - Util::timer_gettime());
}


void Timer::ResetTimer() 
{ 
	SetTimerTicks( Util::timer_gettime() );
}