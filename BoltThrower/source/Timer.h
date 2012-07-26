#ifndef Timer_H_
#define Timer_H_

#include <gccore.h>

class Timer
{
private:
	u64 GetTimerTicks() const { return m_Timer; }
	void SetTimerTicks(u64 t) { m_Timer = t; }
public:
	Timer();
	void SetTimerSeconds(u32 t);
	void SetTimerMillisecs(u32 t); 
	void ResetTimer();

	bool IsTimerDone();
	u32 GetTimerSeconds();
	u64 GetTimerMicrosecs();

	u64 m_Timer;
};

#endif