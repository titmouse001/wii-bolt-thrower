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
#include "camera.h"
#include "Util_google_analytics.h"

#include "Thread.h"

extern Thread MyThread;

UpdateManager::UpdateManager(): m_ReleaseNotes("-"), m_LatestReleaseAvailable("-"), m_MessageVersionReport("-")
{
}

void UpdateManager::Init(){
	m_pWiiManager =  Singleton<WiiManager>::GetInstanceByPtr();
}

void UpdateManager::DoUpdate(string MasterUpdateFile){

	if ( CheckForUpdate(MasterUpdateFile) )
	{
		if (UpdatePackageYesOrNo())
		{
			MyThread.m_Data.Message = "";
			MyThread.m_Data.WorkingBytesDownloaded =0;
			MyThread.m_Data.State = ThreadData::ONLINEDOWNLOAD_UPDATE;

			//update
			UpdateApplicationFiles();
			
			MyThread.m_Data.Message = "";
			MyThread.m_Data.WorkingBytesDownloaded =0;
			MyThread.m_Data.State = ThreadData::UPDATE_COMPLETE_RESET;

			Util::SleepForMilisec(15); // give that above stuff time to kick in and display
			MyThread.Stop();
	
			m_pWiiManager->UnInitWii();
			exit(0);
		}
	}
}

void  UpdateManager::UpdateApplicationFiles( ){

	URLManager* pURLManager = m_pWiiManager->GetURLManager();

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

			MyThread.m_Data.Message = "downloading "+ WiiFile::GetFileNameWithoutPath(URI_FilePath);
	
			// Download and Save 
			if ( pURLManager->SaveURI(URI_FilePath , FullDownloadPath ) )
				MyThread.m_Data.Message = "Not Found "+ WiiFile::GetFileNameWithoutPath(URI_FilePath);

		}
	}
}

bool UpdateManager::CheckForUpdate(string MasterUpdateFile)
{
	MyThread.m_Data.State = ThreadData::CHECKING_FOR_UPDATE;
	MyThread.m_Data.Message = "";

	bool Report( false );

	URLManager* pURLManager = m_pWiiManager->GetURLManager();

	if (pURLManager->m_Initialised)
	{

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
		if ( MasterUpdateFile != "LatestVersion_FAKE")
		{
			m_ReleaseNotes = "";
			m_MessageVersionReport = ""; // setup later in this section
			// Tally online
			pURLManager->GetFromURI( Util_GA::CreateGoogleAnalyticsRequest(MasterUpdateFile) ); // google analytics

			MyThread.m_Data.Message = MasterUpdateFile+ ".xml : tally";

		}
		else
		{	m_ReleaseNotes = "FAKE";  // fake test running
			m_MessageVersionReport = "FAKE"; // fake test running
		}

		// pURLManager->SaveURI("http://wii-bolt-thrower.googlecode.com/hg/LatestVersion.xml",WiiFile::GetGamePath() );
		MemoryInfo* pData(pURLManager->GetFromURI("http://wii-bolt-thrower.googlecode.com/hg/" + MasterUpdateFile + ".xml"));

		MyThread.m_Data.Message = MasterUpdateFile + ".xml : new file updates";

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
				if (Notes != NULL) {
					m_ReleaseNotes += Notes->GetText();
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
						m_MessageVersionReport += "ver " + m_LatestReleaseAvailable + " now available, visit http://wiibrew.org/wiki/BoltThrower";
					}
					else
					{
						// will be used by the bottom Bar Text in the menu menu
						SetMessageVersionReport( m_pWiiManager->GetText("RunningLatestVersion")  + s_ReleaseVersion + " - " + s_DateOfRelease );
					}
				}
				//-----------------------------------------------------------------------------------------
			}
		}
//	else
//	ExitPrintf(doc.ErrorDesc());
	}

	return Report;
}


bool UpdateManager::UpdatePackageYesOrNo()
{
	//m_pWiiManager->GetCamera()->SetCameraView(0,0) ;
	m_pWiiManager->GetMessageBox()->SetUpMessageBox( m_LatestReleaseAvailable + " Available", m_ReleaseNotes );			

	MyThread.m_Data.State = ThreadData::QUESTION;
	MyThread.m_Data.Message = "";
	MyThread.m_Data.WorkingBytesDownloaded = 0;

	// this section is logic only (a separate thread takes care of the screen updates)
	do {
		Util::SleepForMilisec(10);
		WPAD_ScanPads();
		if (WPAD_ButtonsUp(0) & WPAD_BUTTON_B)
			return false; // don't update

	} while( (WPAD_ButtonsUp(0) & WPAD_BUTTON_A ) == 0 );

	return true; // update
}


//
// This function Downloads Files read from the configuration, using anything keep in AddURI
//		<AddFile 	URI="http://wii-bolt-thrower.googlecode.com/hg/Data/BoltThrower_v0.62/boot.dol"
//					FullDownloadPath="/" OverwriteExistingFile="YES"/>
//
bool UpdateManager::DownloadFilesListedInConfiguration(bool MisssingCheckOnly)
{
	URLManager* pURLManager = m_pWiiManager->GetURLManager();

	if (pURLManager->m_Initialised) {

		if (!MisssingCheckOnly) {
			MyThread.Start(ThreadData::ONLINEDOWNLOAD_EXTRAFILES);
		}

		// download missing files
		for ( vector<FileInfo>::iterator Iter( m_pWiiManager->GetDownloadInfoBegin()); Iter !=  m_pWiiManager->GetDownloadInfoEnd() ; ++Iter )
		{
			string FullDownloadPath(WiiFile::GetGamePath() + Iter->FullDownloadPath );
			string FullDownloadPathWithoutEndSlash = FullDownloadPath.substr(0,FullDownloadPath.rfind("/"));

			string URI_FilePath ( Iter->LogicName.c_str() );
			string FileName ( FullDownloadPath + WiiFile::GetFileNameWithoutPath( URI_FilePath ) );

			if ( !(WiiFile::CheckFileExist(FileName)) )
			{
				if (MisssingCheckOnly) {
					return true;
				}
				else {
					if ( !(WiiFile::CheckFileExist(FullDownloadPathWithoutEndSlash)) ) {
						mkdir(FullDownloadPathWithoutEndSlash.c_str(), 0777);
					}

					MyThread.m_Data.Message = "downloading "+ WiiFile::GetFileNameWithoutPath(URI_FilePath);
					if ( ! pURLManager->SaveURI(URI_FilePath , FullDownloadPath ) )
						MyThread.m_Data.Message = "Not Found "+ WiiFile::GetFileNameWithoutPath(URI_FilePath);
				}
			}
		}

		if (!MisssingCheckOnly)
			MyThread.Stop();

		//Refresh music list - may have just download something
		m_pWiiManager->ScanMusicFolder();

	}
	return false;
}
