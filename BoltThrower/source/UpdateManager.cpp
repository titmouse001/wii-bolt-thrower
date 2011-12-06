#include <wiiuse/wpad.h>
#include "Singleton.h"
#include "WiiManager.h"
#include "TinyXML/TinyXML.h"
#include "UpdateManager.h"
#include "GameDisplay.h"
#include "URIManager.h"
#include "messageBox.h"
#include "Util3D.h"
#include "Util.h"
#include "WiiFile.h"
#include "image.h"
#include "Config.h"
#include <sys/dir.h>

#include <sstream>
#include <time.h>
#include <ctype.h>
#include <string.h>

#include "debug.h"

#include "Util_google_analytics.h"


string CreateString()
{
	// This was worked out using a network packet analyzer - I used WireShark 
	// Corners have been cut in places, and the code just says each  visit is a new one. i.e. fudged the same DataTime x3 

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
	GA_Parameters.push_back("&utmdt=LatestVersion.xml%20-%20wii-bolt-thrower%20-%20Wii%20Bolt%20Thrower%20-%20developed%20under%20the%20Wii%20homebrew%20platform%20-%20Google%20Project%20Hosting");
	GA_Parameters.push_back("&utmhid=" + Util_GA::GetRandom9DigitDecimalAsString() );
	GA_Parameters.push_back("&utmr=0");
	GA_Parameters.push_back("&utmp=%2Fp%2Fwii-bolt-thrower%2Fsource%2Fbrowse%2FLatestVersion.xml");
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
							 "." + DateTime +	// Timestamp when cookie was set
							 ".1"						// Session number
							 ".1"						// Campaign number
							 ".utmcsr%3Dwii-bolt-thrower.googlecode.com%7Cutmccn%3D(referral)%7Cutmcmd%3Dreferral%7Cutmcct%3D%2Fhg%2F%3B");
	GA_Parameters.push_back("&utmu=qAAg~");

	string BuildGetRequest("http://www.google-analytics.com/__utm.gif?");
	for (vector<string>::iterator Iter(GA_Parameters.begin()) ; Iter!=GA_Parameters.end(); ++Iter)
		BuildGetRequest += *Iter;

	return BuildGetRequest;
}





UpdateManager::UpdateManager():
	m_ReleaseNotes("-"),
	m_LatestReleaseAvailable("-"),
	m_MessageVersionReport("-")
{
}

void UpdateManager::Init()
{
//	WiiManager& rWiiManager( Singleton<WiiManager>::GetInstanceByRef() );

}

void UpdateManager::DoUpdate()
{
	if ( CheckForUpdate() )
	{
		if (DisplayUpdateMessage())
		{
			//update
			UpdateApplicationFiles();

			WiiManager& rWiiManager( Singleton<WiiManager>::GetInstanceByRef() );
			rWiiManager.GetGameDisplay()->DisplaySimpleMessage("Update complete, please reload",-3.14f/34.0f);
			rWiiManager.UnInitWii();
			exit(0);
		}
	}
}

void  UpdateManager::UpdateApplicationFiles( )
{
	URLManager* pURLManager( new URLManager );
	WiiManager& rWiiManager( Singleton<WiiManager>::GetInstanceByRef() );

	for ( vector<FileInfo>::iterator Iter( m_ApplicationSpecificUpdatesForDownloadFileInfoContainer.begin()); 
		Iter !=  m_ApplicationSpecificUpdatesForDownloadFileInfoContainer.end() ; ++Iter )
	{
		string FullDownloadPath(WiiFile::GetGamePath() + Iter->FullDownloadPath );
		string FullDownloadPathWithoutEndSlash = FullDownloadPath.substr(0,FullDownloadPath.rfind("/"));

		string URI_FilePath ( Iter->LogicName.c_str() );
		string FileName ( FullDownloadPath + WiiFile::GetFileNameWithoutPath( URI_FilePath ) );

		bool EnableWrite = false;
		if ( Iter->OverwriteExistingFile )
		{
			EnableWrite = true;
		}
		else if ( !WiiFile::CheckFileExist(FileName) )
		{
			EnableWrite = true;
		}

		if ( EnableWrite )
		{
			if ( !(WiiFile::CheckFileExist(FullDownloadPathWithoutEndSlash)) )
			{
				mkdir(FullDownloadPathWithoutEndSlash.c_str(), 0777);
			}
			//rWiiManager.GetGameDisplay()->DisplaySmallSimpleMessage("MK DIR " + FullDownloadPathWithoutEndSlash);
			rWiiManager.GetGameDisplay()->DisplaySmallSimpleMessage("downloading " + URI_FilePath);
			if (! pURLManager->SaveURI(URI_FilePath , FullDownloadPath ) )
				rWiiManager.GetGameDisplay()->DisplaySmallSimpleMessage("Not Found "+ URI_FilePath);
		}
	}
	delete pURLManager;
}

bool UpdateManager::CheckForUpdate()
{
	bool Report( false );

	URLManager* pURLManager( new URLManager );

	if (pURLManager->m_Initialised)
	{
		WiiManager& rWiiManager( Singleton<WiiManager>::GetInstanceByRef() );
		rWiiManager.GetCamera()->SetCameraView(0,0) ;

#define TEST_FROM_FILE (0)

#if (TEST_FROM_FILE==1)

#warning *** DONT FORGET TO CHANGE 'TEST_FROM_FILE' DEFINE FOR RELEASE BUILDS ***

		// TEST CODE - usefull when testing from emulator (from file rather than http site)
		FILE* pFile( WiiFile::FileOpenForRead( WiiFile::GetGamePath() +  "LatestVersion.xml" )  );
		fseek (pFile , 0, SEEK_END);
		uint FileSize( ftell (pFile) );
		rewind(pFile); 
		u8* ptestdata = (u8*) malloc (sizeof(char) * FileSize);
		size_t TestSize = fread (ptestdata,1,FileSize,pFile);
#else
		pURLManager->GetFromURI( CreateString() ); // google analytics
		// pURLManager->SaveURI("http://wii-bolt-thrower.googlecode.com/hg/LatestVersion.xml",WiiFile::GetGamePath() );
		MemoryInfo* pData(pURLManager->GetFromURI("http://wii-bolt-thrower.googlecode.com/hg/LatestVersion.xml"));
#endif
		TiXmlDocument doc;
#if (TEST_FROM_FILE==1)
		if ( doc.LoadMem( (char*) ptestdata, TestSize ) )
#else
		if ( ( pData!=NULL ) && 
			 ( doc.LoadMem( (char*)pData->GetData(), pData->GetSize() ) ) )   // from file test 
#endif
		{
			//Check version number
			TiXmlHandle docHandle( &doc );
			TiXmlHandle Data( docHandle.FirstChild( "Data" ) );
			if (Data.Element() != NULL)  // check for valid xml root
			{

				//-----------------------------------------------------------------------------------------
				TiXmlElement* Updates =  Data.FirstChild( "Updates" ).FirstChildElement().ToElement();
				for( TiXmlElement* pElement(Updates); pElement!=NULL; pElement=pElement->NextSiblingElement() )
				{
					string Key(pElement->Value());
					if (Key=="AddFile") 
					{		
						if ( (pElement->Attribute("URI")!=0) && (pElement->Attribute("FullDownloadPath")!=0) )
						{
							FileInfo Info( pElement->Attribute("URI"), Util::urlDecode( pElement->Attribute("URI") ) );
							Info.FullDownloadPath = pElement->Attribute("FullDownloadPath");
							Info.OverwriteExistingFile = "YES";
							if ( pElement->Attribute("OverwriteExistingFile") != 0 ) 
								Info.OverwriteExistingFile = pElement->Attribute("OverwriteExistingFile");

							m_ApplicationSpecificUpdatesForDownloadFileInfoContainer.push_back( Info );
						}
					}
				}
				//-----------------------------------------------------------------------------------------
				//string ReleaseNotesText;
				TiXmlElement* Notes =  Data.FirstChild( "ReleaseNotes" ).ToElement();
				if (Notes != NULL)
				{
					m_ReleaseNotes = Notes->GetText();
				}
				//-----------------------------------------------------------------------------------------
				TiXmlElement* pElem=Data.FirstChild("LatestReleaseAvailable").Element();
				if (pElem != NULL)
				{
					m_LatestReleaseAvailable = pElem->GetText();
					float fLatestReleaseAvailable =  atof(m_LatestReleaseAvailable.c_str());
					if ( fLatestReleaseAvailable > s_fVersion )
					{
						Report = true;
						//	DisplayUpdateMessage(m_ReleaseNotes, m_LatestReleaseAvailable + " Available");

						m_MessageVersionReport = "ver " + m_LatestReleaseAvailable + " now available, visit http://wiibrew.org/wiki/BoltThrower";
					}
				}
				//-----------------------------------------------------------------------------------------
			}
		}
	}
	delete pURLManager;

	return Report;
}


bool UpdateManager::DisplayUpdateMessage()
{
	WiiManager& rWiiManager( Singleton<WiiManager>::GetInstanceByRef() );
	rWiiManager.GetCamera()->SetCameraView(0,0) ;
	rWiiManager.GetMessageBox()->SetUpMessageBox( m_LatestReleaseAvailable + " Available", m_ReleaseNotes );			

	do
	{
		WPAD_ScanPads();
		static float spin=0.0f;
		spin+=0.01f;
		GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
		Util3D::Trans(rWiiManager.GetScreenWidth()/2.0f, rWiiManager.GetScreenHeight()/2.0f);

		// image is 1024x1024  ((100-n)% more zoomed in)
		rWiiManager.GetSpaceBackground()->DrawImageXYZ(0,0, 0.95f * (579.4f * (579.4f/1024.0f)) ,255);

		rWiiManager.GetMessageBox()->DisplayMessageBox(500,300);
		GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);
		rWiiManager.SwapScreen();  // to clear zbuffer keep GX_SetZMode on until after this call 
		GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);

		if (WPAD_ButtonsUp(0) & WPAD_BUTTON_B)
			return false; // don't update

	} while( (WPAD_ButtonsUp(0) & WPAD_BUTTON_A ) == 0 );

	return true; // update
}