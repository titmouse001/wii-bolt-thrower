#ifndef DEBUG_H
#define DEBUG_H

//#ifdef _DEBUG
//#define ExitPrintf			Debug::_ExitPrintf
//#else
//#define ExitPrintf			sizeof
//#endif

#define ExitPrintf			Debug::_ExitPrintf

namespace Debug 
{
	void _ExitPrintf(const char* pFormat, ...);
};

#endif

