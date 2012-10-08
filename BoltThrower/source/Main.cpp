// Bolt Thrower for the wii - Paul Overy - 2010
// reminder to myself - To turn on/off visual indicators of whitespace in Visual Studio, use the keyboard chord (Ctrl-R, Ctrl-W) 

#include <gccore.h>
#include <sys/dir.h>
#include "config.h"
#include "Singleton.h"
#include "WiiManager.h"
#include "debug.h"
#include "GameDisplay.h"
#include "URIManager.h"
#include "UpdateManager.h"
#include "camera.h"

#include "Thread.h"
#include "util.h"

#ifndef BUILD_FINAL_RELEASE
#warning *** DONT FORGET TO CHANGE THIS DEFINE FOR RELEASE BUILDS ***
#endif


Thread MyThread;
#include "HashString.h"
extern "C" {  extern void __exception_setreload(int t); }
int main(int /* argc */, char**  argv ) 
{	
	__exception_setreload(6);
	WiiManager& rWiiManager( Singleton<WiiManager>::GetInstanceByRef() );

	if (argv[0]!=NULL)
		rWiiManager.m_ExePath = WiiFile::GetPathFromFullFileName( argv[0] ) ;
	else
		rWiiManager.m_ExePath = "";

	rWiiManager.InitWii();  

	rWiiManager.InitGameResources();   // NOTE: a thread is started in here, just after loading the small font & cog graphics

	rWiiManager.FinalInitManagers();   // dependancy on call to InitWii that sets up GXRModeObj
	
#if (0)
	//-------------------------------------------------
	// Downloads the "LatestVersion.XML" containing the update information
	rWiiManager.GetUpdateManager()->DoUpdate(s_MasterFileLatestVersion);
	//-------------------------------------------------
#endif
#if (1)
	//-------------------------------------------------
	// Check for any missing downloads like extra music - from DownloadFiles section in the "configurtion.xml" file
	rWiiManager.m_MusicStillLeftToDownLoad = rWiiManager.GetUpdateManager()->DownloadFilesListedInConfiguration(true); // true so SCAN ONLY
	//-------------------------------------------------
#endif

	MyThread.Stop();


	rWiiManager.FinalInitGameResources_NOT_DISPLAY_THREAD_SAFE();  // don't forget to turn off display thread before calling this

	//
	// Main loop
	//
	rWiiManager.MainLoop();

	//
	// Display exit message using display thread
	//
	MyThread.Start(ThreadData::QUIT);
	Util::SleepForMilisec(1500);
	MyThread.Stop();
	rWiiManager.UnInitWii();


	return 0;
}


// Buzz words for web search
//
//// Interpolation methods
//
//double LinearInterpolate(
//   double y1,double y2,
//   double mu)
//{
//   return(y1*(1-mu)+y2*mu);
//}
//
//Linear interpolation results in discontinuities at each point. 
//Often a smoother interpolating function is desirable, perhaps the simplest is cosine interpolation. 
//A suitable orientated piece of a cosine function serves to provide a smooth transition between adjacent segments. 
//
//double CosineInterpolate(
//   double y1,double y2,
//   double mu)
//{
//   double mu2;
//
//   mu2 = (1-cos(mu*PI))/2;
//   return(y1*(1-mu2)+y2*mu2);
//}



//
////Wiilight stuff  
//static vu32 *_wiilight_reg = (u32*)0xCD0000C0;  
//void wiilight(int enable) 
//{            
// // Toggle wiilight (thanks Bool for wiilight source)     
//  u32 val = (*_wiilight_reg&~0x20);      
//  if (enable && Settings.wiilight) 
//  val |= 0x20;      *_wiilight_reg=val; 
//}    
//



/* 
Bolt Thrower - Games design notes

* Players can guide asteroids, by placing proximity mines or any other method like ramming
* Offensive weapons
	- Effector – uses highly-efficient & sophisticated technology, doubles up as a defensive weapon 
* Defensive weapons
	- Chaff dispenser - cheap and easy to resource
* Background effect - Comets with dust trail

* Mechanisms of War
	- A material defeat happens when the ability to fight is lost
	- A mental defeat happens when the will to fight is lost
	- Reduced ability affects the will to fight, and reduced will affects the ability to fight
* Decisive action (quick), or by attrition (slow)
	- Decisive action is meant to defeat the enemy in a relatively short period of maximum effort, which in itself contributes to defeating the enemy by rapid destruction, and by shocking some of its forces and neutralizing others. It's a good way to defeat a stronger enemy or to defeat the enemy with minimal losses, but sometimes it's simply not possible, depending on the situation. 
	- Attrition is meant to defeat the enemy by gradually eroding its resources (and/or morale) at a rate higher than its rate of recovery, and of course at the same time by not being eroded even faster at your side. A war of attrition is a slow and often very bloody business, but if decisive action is not possible it's the only way left. The best (and bloodiest) example of attrition is World War 1 where millions of lives were lost and the only benefit was that the enemy was running out of soldiers a bit faster. 
	(A blockade is a common way of attrition. It sometimes has the potential of being so successful in damaging the enemy that it might quickly become a decisive action that will suffocate the enemy's war potential and end the war.  )
	
	Defeat can be achieved either by attacking and defeating the enemy's strong points (which might save the need to also attack the rest of its force) or weak points (which saves the need to attack its strong points).
	
	Detecting the enemy's weak points or detecting an enemy key strong point worthy of being attacked is one of the main goals of the various types of intelligence units, from plain combat observers to codebreakers and spies. 

	The enemy is often generally aware of its weaknesses, and if knows early enough which one will be attacked it can often reinforce it and neutralize the attack. That's why surprise and deception are often so important in attack and even in defense, and why secrecy is so important in not letting the enemy gather information about your side, your weaknesses, and your intentions. 

* The four mechanisms of defeat
	- The ability to fight is defeated either by destruction
	- or by paralysis. 
	- The will to fight is defeated either by loss of interest in the cause 
	- or by loss of faith in the ability to achieve it. 

'The Mechanisms Of Defeat': see (http://www.2worldwar2.com/mechanisms-of-defeat.htm)
	
*/
