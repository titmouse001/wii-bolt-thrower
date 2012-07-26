#ifndef _UTIL_GA_H
#define _UTIL_GA_H

#include "GCTypes.h"
//#include "ogc/gx.h"
#include <string>

using namespace std;

namespace Util_GA
{
	int CreateHash(string domain);
	string CreateHashString(string domain);
	string GetUnixTimeNow();
	string GetRandom9DigitDecimalAsString();
	string CreateGoogleAnalyticsRequest(string MasterUpdateFile);
}

#endif