//#include <gccore.h>
#include <math.h>
#include "Util.h"
#include "config.h"
#include "ogc\lwp_watchdog.h"
#include "ogc\system.h"
#include "ogcsys.h"
#include "wiiuse\wpad.h"
#include <sstream>
#include <iomanip>

#include "Util_google_analytics.h"
using namespace std;

int Util_GA::CreateHash(string domain) 
{		
	int hash(1);		
	if (!domain.empty()) 
	{			
		hash = 0;			
		for (int index = domain.length() - 1; index >= 0; index--) 
		{				
			int Character = domain[index];				
			hash = (hash << 6 & 268435455) + Character + (Character << 14);			
			int code(hash & 266338304);			
			hash = code != Character ? hash ^ code >> 21 : hash;		
		}		
	}	
	return hash;	
}


string Util_GA::CreateHashString(string domain)
{
	std::ostringstream  osstream;
	osstream << Util_GA::CreateHash(domain);
	return osstream.str();
}


string Util_GA::GetUnixTimeNow()
{
	std::ostringstream  osstream;
	osstream << time(NULL);
	return osstream.str();
}

string Util_GA::GetRandom9DigitDecimalAsString()
{
	std::stringstream ss;
	ss <<  1+(rand()%9);
	ss <<  1+(rand()%9);
	ss <<  1+(rand()%9);
	ss <<  1+(rand()%9);
	ss <<  1+(rand()%9);
	ss <<  1+(rand()%9);
	ss <<  1+(rand()%9);
	ss <<  1+(rand()%9);
	ss <<  1+(rand()%9);
	return ss.str();
}
