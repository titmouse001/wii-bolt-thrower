#include <wiiuse/wpad.h>

#include "Singleton.h"
#include "WiiManager.h"
#include "TinyXML/TinyXML.h"
#include "UpdateManager.h"
#include "URIManager.h"
#include "messageBox.h"
#include "Util3D.h"
#include "WiiFile.h"
#include "image.h"
#include "Config.h"

//extern string MesageHack;

UpdateManager::UpdateManager():
	m_ReleaseNotes("-"),
	m_LatestReleaseAvailable("-"),
	m_MessageVersionReport("-")
{
}

void UpdateManager::Init()
{
	WiiManager& rWiiManager( Singleton<WiiManager>::GetInstanceByRef() );
	// default message - may get overwritten later
	m_MessageVersionReport = rWiiManager.GetText("Running lastest version ")  + s_ReleaseVersion + " - " + s_DateOfRelease;
}

void UpdateManager::DoUpdate()
{
	if ( CheckForUpdate() )
	{
		DisplayUpdateMessage();
	}
}

bool UpdateManager::CheckForUpdate()
{
	bool Report( false );

	URLManager* pURLManager( new URLManager );
	WiiManager& rWiiManager( Singleton<WiiManager>::GetInstanceByRef() );
	rWiiManager.GetCamera()->SetCameraView(0,0) ;

#define TEST_FROM_FILE (1)

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
	// pURLManager->SaveURI("http://wii-bolt-thrower.googlecode.com/hg/LatestVersion.xml",WiiFile::GetGamePath() );
	MemoryInfo* pData(pURLManager->GetFromURI("http://wii-bolt-thrower.googlecode.com/hg/LatestVersion.xml"));
#endif

	TiXmlDocument doc;
#if (TEST_FROM_FILE==1)
	if ( doc.LoadMem( (char*) ptestdata, TestSize ) )
#else
	if ( doc.LoadMem( (char*)pData->GetData(), pData->GetSize() ) )   // from file test 
#endif
	{
		//Check version number
		TiXmlHandle docHandle( &doc );
		TiXmlHandle Data( docHandle.FirstChild( "Data" ) );
		if (Data.Element() != NULL)  // check for valid xml root
		{
			//-----------------------------------------------------------------------------------------
			//////TiXmlElement* Updates =  Data.FirstChild( "Updates" ).FirstChildElement().ToElement();
			//////for( TiXmlElement* pElement(Updates); pElement!=NULL; pElement=pElement->NextSiblingElement() )
			//////{
			//////	string Key(pElement->Value());
			//////	if (Key=="AddFile") 
			//////	{
			//////		string a = pElement->Attribute("URI");
			//////		string b= pElement->Attribute("Path");
			//////		printf("URI=%s PATH=%s", a.c_str(), b.c_str() );
			//////	}
			//////}
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
	delete pURLManager;
	
	return Report;
}


void UpdateManager::DisplayUpdateMessage()
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
	} while( (WPAD_ButtonsUp(0) & (WPAD_BUTTON_A | WPAD_BUTTON_B) )== 0 );

}