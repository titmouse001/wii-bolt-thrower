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
#include <vector>

#include "Util_google_analytics.h"
using namespace std;

int Util_GA::CreateHash(string domain) 
{		
	int hash(1);		
	if (!domain.empty()) {			
		hash = 0;			
		for (int index = domain.length() - 1; index >= 0; index--) {				
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

string Util_GA::CreateGoogleAnalyticsRequest(string MasterUpdateFile)
{
	// reverse engineered using a network packet analyzer - I used WireShark 
	// Corners have been cut in places, as each visit is a new one. i.e. fudged the same DataTime x3 

	string DateTime = Util_GA::GetUnixTimeNow();
	vector<string> GA_Parameters; 
	GA_Parameters.push_back("utmwv=5.1.9");
	GA_Parameters.push_back("&utms=15");
	GA_Parameters.push_back("&utmn=" + Util_GA::GetRandom9DigitDecimalAsString() );
	GA_Parameters.push_back("&utmhn=code.google.com");
	GA_Parameters.push_back("&utmcs=utf-8");
	GA_Parameters.push_back("&utmsr=640x480");  // Wii resolution (mostly)
	GA_Parameters.push_back("&utmsc=32-bit");
	GA_Parameters.push_back("&utmul=en-gb");
	GA_Parameters.push_back("&utmje=1");
	GA_Parameters.push_back("&utmfl=10.3%20r183");
	
	GA_Parameters.push_back("&utmdt=" + MasterUpdateFile + ".xml%20-%20wii-bolt-thrower%20-%20Wii%20Bolt%20Thrower%20-%20developed%20under%20the%20Wii%20homebrew%20platform%20-%20Google%20Project%20Hosting");

	GA_Parameters.push_back("&utmhid=" + Util_GA::GetRandom9DigitDecimalAsString() );
	GA_Parameters.push_back("&utmr=0");
	GA_Parameters.push_back("&utmp=%2Fp%2Fwii-bolt-thrower%2Fsource%2Fbrowse%2F" + MasterUpdateFile + ".xml");
	GA_Parameters.push_back("&utmac=UA-25374250-1");
	GA_Parameters.push_back("&utmcc=__utma%3D" +
							 Util_GA::CreateHashString("code.google.com") +		// Domain hash ... i.e. 247248150 for "code.google.com"
							 "." + Util_GA::GetRandom9DigitDecimalAsString() +	// Unique Identifier ... i.e. ".233226017"
							 "." + DateTime +	// Timestamp of first visit
							 "." + DateTime +	// Timestamp of previous visit
							 "." + DateTime +	// Timestamp of current visit
							 ".1%3B%2B"					// "7;+" - Number of sessions started???
							 "__utmz%3D" +				// "__utmz="
							 Util_GA::CreateHashString("code.google.com") +	// Domain Hash  ...
							 "." + DateTime +			// Timestamp when cookie was set
							 ".1"						// Session number
							 ".1"						// Campaign number
							 ".utmcsr%3Dwii-bolt-thrower.googlecode.com%7Cutmccn%3D(referral)%7Cutmcmd%3Dreferral%7Cutmcct%3D%2Fhg%2F%3B");
	GA_Parameters.push_back("&utmu=qAAg~");

	string BuildGetRequest("http://www.google-analytics.com/__utm.gif?");
	for (vector<string>::iterator Iter(GA_Parameters.begin()) ; Iter!=GA_Parameters.end(); ++Iter)
		BuildGetRequest += *Iter;

	return BuildGetRequest;
}
