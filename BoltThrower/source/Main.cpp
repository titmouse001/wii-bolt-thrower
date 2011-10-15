// Bolt Thrower for the wii - Paul Overy - 2010

// To turn on visual indicators of whitespace in Visual Studio, use the keyboard chord (Ctrl-R, Ctrl-W) 

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

#include <gccore.h>
#include "config.h"
#include "Singleton.h"
#include "WiiManager.h"
#include "Image.h"
#include "InputDeviceManager.h"
#include "GameLogic.h"
#include "Menu.h"
#include "debug.h"
#include "Util.h"
#include "Util3d.h"
#include "HashString.h"
#include <math.h>
#include "WiiFile.h"
#include <asndlib.h>
#include <gcmodplay.h>
#include "FontMAnager.h"
#include "ImageManager.h"
#include "SoundManager.h"
#include "JPEGDEC.h"
#include "Image.h"
#include "GameDisplay.h"
#include "MenuScreens.h"
//#include  "HTTP/HTTP_Util.h"
#include <network.h>
#include  "URIManager.h"
#include <ogc/machine/processor.h>
#include <ogc/lwp_watchdog.h>
#include "ogc/lwp.h"
#include <ogcsys.h>
#include <unistd.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include  "vessel.h"
#include "oggplayer/oggplayer.h"
#include  "SetupGame.h"
#include  "util.h"
#include "TinyXML/TinyXML.h"

#include "MessageBox.h"
//extern void OggTest();

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


#include "CullFrustum\Vec3.h"
#include "CullFrustum\FrustumR.h"

// TODO: main is getting nasty - needs a big refactor

void QUICKHACK_UpdateMessage(string Message,string Heading);
void CheckForUpdate();

string MesageHack = "";

extern "C" {  extern void __exception_setreload(int t); }
int main(int /* argc */, char** /* argv */) 
{	


	__exception_setreload(6);
	WiiManager& rWiiManager( Singleton<WiiManager>::GetInstanceByRef() );

	rWiiManager.InitWii();

	rWiiManager.ProgramStartUp();


	rWiiManager.GetCamera()->InitialiseCamera(); // 3D View


	MesageHack = rWiiManager.GetText("Version")  + s_ReleaseVersion + " - " + s_DateOfRelease;
	CheckForUpdate();



	rWiiManager.GetSetUpGame()->MainLoop();

	rWiiManager.GetGameDisplay()->DisplaySimpleMessage(rWiiManager.GetText("QuitMessage"));
	rWiiManager.UnInitWii();
	return 0;
}


void CheckForUpdate()
{
	URLManager* pURLManager( new URLManager );

	// download missing music
	WiiManager& rWiiManager( Singleton<WiiManager>::GetInstanceByRef() );

	rWiiManager.GetCamera()->SetCameraView(0,0) ;

	for ( vector<FileInfo>::iterator Iter( rWiiManager.GetOggInfoBegin()); Iter !=  rWiiManager.GetOggInfoEnd() ; ++Iter )
	{
		string MusicPath(WiiFile::GetGamePath() + Iter->DownloadDir );

		string URI_FilePath ( Iter->LogicName.c_str() );
		//printf( "URI: %s", URI_FilePath.c_str() );
		string FileName ( MusicPath + WiiFile::GetFileNameWithoutPath( URI_FilePath ) );

		if ( !(WiiFile::CheckFileExist(FileName)) )
		{
				rWiiManager.GetGameDisplay()->DisplaySmallSimpleMessage("downloading and caching " + FileName);
			//printf("____missing... downloading and caching____ %s",FileName.c_str());
			pURLManager->SaveURI(URI_FilePath , MusicPath );
		}
	}

	
	//Check version number


	// pURLManager->SaveURI("http://wii-bolt-thrower.googlecode.com/hg/LatestVersion.xml",WiiFile::GetGamePath() );
	MemoryInfo* pData(pURLManager->GetFromURI("http://wii-bolt-thrower.googlecode.com/hg/LatestVersion.xml"));

	//// TEST CODE - usefull when testing from emulator (put file on SD rather than http site)
	//FILE* pFile( WiiFile::FileOpenForRead( WiiFile::GetGamePath() +  "LatestVersion.xml" )  );
	//fseek (pFile , 0, SEEK_END);
	//uint FileSize( ftell (pFile) );
	//rewind(pFile); 
	//u8* ptestdata = (u8*) malloc (sizeof(char) * FileSize);
	//size_t TestSize = fread (ptestdata,1,FileSize,pFile);

	TiXmlDocument doc;
	if ( doc.LoadMem( (char*)pData->GetData(), pData->GetSize() ) )   // from file test 
//	if ( doc.LoadMem( (char*) ptestdata, TestSize ) )
	{
		TiXmlHandle docHandle( &doc );
		TiXmlHandle Data( docHandle.FirstChild( "Data" ) );
		if (Data.Element() != NULL)  // check for valid xml root
		{
			TiXmlElement* pElem=Data.FirstChild("LatestReleaseAvailable").Element();
			if (pElem)
			{
				double fLatestReleaseAvailable( atof(pElem->GetText()) );
				printf("LatestReleaseAvailable = %f", fLatestReleaseAvailable );
				if ( fLatestReleaseAvailable > s_fVersion )
				{
					//printf("New Version Available %f",s_fVersion );
					QUICKHACK_UpdateMessage((string)"visit http://wiibrew.org/wiki/BoltThrower for download (sorry no auto update yet)",
						pElem->GetText() + (string)" Available");

					MesageHack = (string) "visit http://wiibrew.org/wiki/BoltThrower " + pElem->GetText() + " is available";
				}
				else
				{
					QUICKHACK_UpdateMessage((string)"No update available \n \n You are running latest version" , "Information");
				}
			}

			TiXmlElement* Updates =  Data.FirstChild( "Updates" ).FirstChildElement().ToElement();
			for( TiXmlElement* pElement(Updates); pElement!=NULL; pElement=pElement->NextSiblingElement() )
			{
				string Key(pElement->Value());
				if (Key=="AddFile") 
				{
					string a = pElement->Attribute("URI");
					string b= pElement->Attribute("Path");
					printf("URI=%s PATH=%s", a.c_str(), b.c_str() );
				}
			}
		}
	}



	delete pURLManager;
}


void QUICKHACK_UpdateMessage(string Message,string Heading)
{
	WiiManager& rWiiManager( Singleton<WiiManager>::GetInstanceByRef() );

	//rWiiManager.GetCamera()->SetCameraView( 0, 0, -(579.4f));
	rWiiManager.GetMessageBox()->SetUpMessageBox(Heading, Message  );				
	rWiiManager.GetCamera()->SetCameraView(0,0) ;
	do
	{
		WPAD_ScanPads();
		static float spin=0.0f;
		spin+=0.01f;
		GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
		Util3D::Trans(rWiiManager.GetScreenWidth()/2.0f, rWiiManager.GetScreenHeight()/2.0f);

		// image is 1024x1024  ((100-n)% more zoomed in)
		rWiiManager.GetSpaceBackground()->DrawImageXYZ(0,0, 0.95f * (579.4f * (579.4f/1024.0f)) ,255);

		rWiiManager.GetMessageBox()->DisplayMessageBox(); //DoMessageBox();
		GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);
		rWiiManager.SwapScreen();  // to clear zbuffer keep GX_SetZMode on until after this call 
		GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
	} while( (WPAD_ButtonsUp(0) & (WPAD_BUTTON_A | WPAD_BUTTON_B) )== 0 );

}


//	printf("   START URLManager  ");
//	URLManager* pURLManager( new URLManager );
	//pURLManager->SaveURI("http://www.dr-lex.be/software/download/mp3tones.zip", Util::GetGamePath() );
	//pURLManager->SaveURI("http://vba-wii.googlecode.com/files/Visual%20Boy%20Advance%20GX%202.2.5.zip");
	//pURLManager->SaveURI("http://www.fnordware.com/superpng/pngtest8rgba.png");
	//pURLManager->SaveURI("http://he3.magnatune.com/all/03-Law%20of%20One-Indidginus.ogg", Util::GetGamePath());
	
//printf("----\n");
	
	//rWiiManager.GetSoundManager()->LoadSound(
			//Util::GetGamePath() + "03-Law of One-Indidginus.ogg", "03-Law of One-Indidginus");
	//		Util::GetGamePath() + "Law.ogg", "03-Law of One-Indidginus");

//	delete pURLManager;



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
