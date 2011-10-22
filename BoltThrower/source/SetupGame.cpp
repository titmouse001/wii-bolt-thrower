#include "Singleton.h"
#include "WiiManager.h"
#include "GameLogic.h"
#include "Util.h"
#include "Menu.h"
#include "MenuManager.h"
#include "MenuScreens.h"
#include "oggplayer/oggplayer.h"
#include <asndlib.h>
#include <gcmodplay.h>
#include <wiiuse/wpad.h>
#include  "SetupGame.h"
#include "debug.h"
//#include "config.h"
#include "MessageBox.h"
#include "Util3D.h"
#include "FontManager.h"


void SetUpGame::Init()
{
	m_pWii = Singleton<WiiManager>::GetInstanceByPtr();
}

void SetUpGame::Intro()
{
	m_pWii->GetGameLogic()->InitialiseGame();
	m_pWii->GetCamera()->SetCameraView( 0, 0, -(579.4f));
	m_pWii->GetGameLogic()->ClearBadContainer();
	m_pWii->GetGameLogic()->InitialiseIntro();

	do 
	{	
		Util::DoResetSystemCheck();

		m_pWii->GetGameLogic()->Intro();

		if ( (WPAD_ButtonsUp(0) & WPAD_BUTTON_HOME)!= 0 )
		{
			m_pWii->SetGameState(WiiManager::eExit);
			return;
		}

		//if ( (WPAD_ButtonsUp(0) & WPAD_BUTTON_1)!= 0 )
		//{
		//		//-------------
		//		//MODPlay_Stop( &m_pWii->m_ModuleTrackerPlayerInterface );
		//		//MODPlay_Unload( &m_pWii->m_ModuleTrackerPlayerInterface );
		//		//-------------
		//		
		//		OggPlayer Ogg;
		//		string FullFileName = WiiFile::GetGamePath() + "03-Law of One-Indidginus.ogg"; // "09-Faerie tale-Indidginus.ogg";
		//		FILE* pOggFile( WiiFile::FileOpenForRead( FullFileName.c_str() ) );
		//		u32 OggSize = WiiFile::GetFileSize(pOggFile);
		//		u8* pOggData = (u8*) malloc(OggSize);
		//		fread( pOggData, OggSize, 1, pOggFile);
		//		Ogg.PlayOgg(pOggData, OggSize, 0, OGG_ONE_TIME);
		//}

	} while( (WPAD_ButtonsUp(0) & (WPAD_BUTTON_A | WPAD_BUTTON_B) )== 0 );

	m_pWii->SetGameState(WiiManager::eMenu);
}


void SetUpGame::Menus()
{
	m_pWii->SetGameState(WiiManager::eMenu);
	m_pWii->GetCamera()->SetCameraView( 0, 0, -(579.4f));
	m_pWii->GetMenuScreens()->SetTimeOutInSeconds();
	m_pWii->GetGameLogic()->InitMenu();

	m_pWii->BuildMenus( true );

	while (1) 
	{	
		Util::DoResetSystemCheck();

		if (m_pWii->IsGameStateMenu())
		{
			m_pWii->GetMenuManager()->SetMenuGroup("MainMenu");

			m_pWii->GetGameLogic()->MoonRocksLogic();
			m_pWii->GetGameLogic()->CelestialBodyLogic();
			m_pWii->GetMenuScreens()->DoMenuScreen();
	


			if ( (WPAD_ButtonsUp(0) & WPAD_BUTTON_B) || (m_pWii->GetMenuScreens()->HasMenuTimedOut()) )  
			{
				m_pWii->SetGameState(WiiManager::eIntro);
				break;
			}

			if ( (WPAD_ButtonsUp(0) & WPAD_BUTTON_A) != 0) 
			{
				HashLabel Name = m_pWii->GetMenuManager()->GetSelectedMenu();


				if (Name == HashString::Options)
				{
					m_pWii->SetGameState(WiiManager::eOptions);
				}
				else if (Name == HashString::Start_Game)
				{
					m_pWii->SetGameState(WiiManager::eGame);
					break;
				}
				else if (Name == HashString::Intro)
				{
					m_pWii->SetGameState(WiiManager::eIntro);
					break;
				}
				else if ( Name ==  HashString::Change_Tune )
				{
	
					Util3D::TransRot(-280,-150,0, M_PI *0.5f );
					m_pWii->GetFontManager()->DisplayTextCentre("Loading...", 
						0,0,
						200,HashString::SmallFont);
					GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);
					m_pWii->SwapScreen();  // to clear zbuffer keep GX_SetZMode on until after this call 

					m_pWii->GetMenuScreens()->DoMenuScreen();
					m_pWii->GetFontManager()->DisplayTextCentre("Loading...", 
						0,0,
						200,HashString::SmallFont);
					GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);
					m_pWii->SwapScreen();  // to clear zbuffer keep GX_SetZMode on until after this call 


					Util::SleepForMilisec(2000);

					m_pWii->NextMusic();

					continue; // don't do the loops screen swap since its already do just above
				}
				else if ( Name == HashString::download_extra_music )
				{
					extern bool DownloadFilesListedInConfiguration(bool);
					DownloadFilesListedInConfiguration(false);
					m_pWii->m_MusicStillLeftToDownLoad = false;
				}
				else if (Name == HashString::Credits)
				{
					m_pWii->SetGameState(WiiManager::eCredits);
				}
				else if (Name == HashString::Controls)
				{
					m_pWii->SetGameState(WiiManager::eControls);
				}
			}
			
			GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);
			m_pWii->SwapScreen();  // to clear zbuffer keep GX_SetZMode on until after this call 
		}

		// ***************
		// *** CREDITS ***
		// ***************
		if (m_pWii->IsGameStateCredits())
		{
			m_pWii->GetMenuScreens()->DoCreditsScreen();
			if ( WPAD_ButtonsUp(0) & (WPAD_BUTTON_A|WPAD_BUTTON_B) )
			{
				//m_pWii->GetMenuScreens()->SetTimeOutInSeconds();
				m_pWii->SetGameState(WiiManager::eMenu);
			}
		}

		// ****************
		// *** Controls ***
		// ****************
		if (m_pWii->IsGameStateControls())
		{
			m_pWii->GetMenuScreens()->DoControlsScreen();
			if ( WPAD_ButtonsUp(0) & (WPAD_BUTTON_A|WPAD_BUTTON_B) )
			{
				//m_pWii->GetMenuScreens()->SetTimeOutInSeconds();
				m_pWii->SetGameState(WiiManager::eMenu);
			}
		}

		// ***************
		// *** OPTIONS ***
		// ***************
		if (m_pWii->IsGameStateOptions())
		{
			m_pWii->GetMenuScreens()->DoOptionsScreen();		
			m_pWii->GetMenuManager()->SetMenuGroup("OptionsMenu");

			if ( WPAD_ButtonsUp(0) & (WPAD_BUTTON_B) )
			{
				m_pWii->GetMenuScreens()->SetTimeOutInSeconds();
				m_pWii->SetGameState(WiiManager::eMenu);
				m_pWii->BuildMenus( true );
			}

			if ( WPAD_ButtonsUp(0) & (WPAD_BUTTON_A) )
			{
				HashLabel Name = m_pWii->GetMenuManager()->GetSelectedMenu();		

				m_pWii->GetMenuManager()->SetMenuGroup("OptionsMenu");

				//if (Name == HashString::Credits)
				//{
				//	m_pWii->SetGameState(WiiManager::eCredits);
				//}
				//else if (Name == HashString::Controls)
				//{
				//	m_pWii->SetGameState(WiiManager::eControls);
				//}
				if (Name == (HashLabel)"Ingame_Music")
				{
					m_pWii->GetMenuManager()->AdvanceMenuItemText(HashString::IngameMusicState);
					m_pWii->SetMusicEnabled( m_pWii->GetMenuManager()->GetMenuItemIndex(HashString::IngameMusicState) );
				}
				else if (Name == (HashLabel)"Ingame_MusicVolume")
				{
					m_pWii->GetMenuManager()->AdvanceMenuItemText(HashString::IngameMusicVolumeState);
					m_pWii->SetIngameMusicVolume(m_pWii->GetMenuManager()->GetMenuItemIndex(HashString::IngameMusicVolumeState));

					printf("%d",m_pWii->GetIngameMusicVolume());
					if ( (m_pWii->GetIngameMusicVolume()==0) && (m_pWii->GetMusicEnabled()) )
					{
						m_pWii->SetMusicEnabled(false);
						m_pWii->GetMenuManager()->AdvanceMenuItemText(HashString::IngameMusicState);
					}
					if ( (m_pWii->GetIngameMusicVolume()>0) && (!m_pWii->GetMusicEnabled()) )
					{
						m_pWii->SetMusicEnabled(true);
						m_pWii->GetMenuManager()->AdvanceMenuItemText(HashString::IngameMusicState);
					}
				}
				
				else if (Name == (HashLabel)"Difficulty_Level")
				{
					m_pWii->GetMenuManager()->AdvanceMenuItemText(HashString::DifficultySetting);
					m_pWii->SetDifficulty(m_pWii->GetMenuManager()->GetMenuItemText(HashString::DifficultySetting));
				}
				else if (Name == (HashLabel)"Set_Language")
				{
					m_pWii->GetMenuManager()->AdvanceMenuItemText(HashString::LanguageSetting);
					m_pWii->SetLanguage( m_pWii->GetMenuManager()->GetMenuItemText(HashString::LanguageSetting) );
				}
				else if ((Name == (HashLabel)"Back") )
				{
					m_pWii->GetMenuScreens()->SetTimeOutInSeconds();
					m_pWii->SetGameState(WiiManager::eMenu);
				}
				m_pWii->BuildMenus( true );
			}
		}

		if ( (WPAD_ButtonsUp(0) & WPAD_BUTTON_HOME)!= 0 )
		{
			m_pWii->SetGameState(WiiManager::eExit);
			break;
		}
	}
}


void SetUpGame::Play()
{
	if ( (m_pWii->GetMusicEnabled()) && (m_pWii->GetIngameMusicVolume() > 0) )
	{
		printf("%d", m_pWii->GetIngameMusicVolume()*20);
		MODPlay_SetVolume( &m_pWii->m_ModuleTrackerPlayerInterface, m_pWii->GetIngameMusicVolume()*20,m_pWii->GetIngameMusicVolume()*20);     
	}
	else
		MODPlay_Stop(&m_pWii->m_ModuleTrackerPlayerInterface);

	m_pWii->GetGameLogic()->InitialiseGame();

	WPAD_ScanPads();   // Do a read now, this will flush out anything old ready for a fresh start.
	
	while (m_pWii->IsGameStateGame()) 
	{
		m_pWii->GetGameLogic()->InGameLogic();

		if ( m_pWii->GetGameLogic()->IsEndLevelTrigger() )
		{
			if (WPAD_ButtonsUp(0) & WPAD_BUTTON_A)
			{
				m_pWii->SetGameState(WiiManager::eIntro);
			}
		}

		Util::DoResetSystemCheck();

		if ( (WPAD_ButtonsUp(0) & WPAD_BUTTON_HOME)!= 0 )
		{
			m_pWii->SetGameState(WiiManager::eExit);
		}

	};
}


void SetUpGame::MainLoop() 
{	
	//MODPlay_Init(&m_pWii->m_ModuleTrackerPlayerInterface);
	
	m_pWii->SetGameState(WiiManager::eIntro);

	while (1)
	{
		//if (!m_pWii->m_ModuleTrackerPlayerInterface.playing)
		//{
		//	MODPlay_SetMOD(&m_pWii->m_ModuleTrackerPlayerInterface, m_pWii->m_pModuleTrackerData);
		//	MODPlay_Start(&m_pWii->m_ModuleTrackerPlayerInterface); 
		//	MODPlay_SetVolume( &m_pWii->m_ModuleTrackerPlayerInterface, 100,100);    
		//}



		switch( (int)m_pWii->GetGameState() )
		{
		case WiiManager::eIntro:
			Intro();
			break;
		case WiiManager::eMenu:
			Menus();
			break;
		case WiiManager::eGame:
			Play();
			break;
		case WiiManager::eExit:
			return; // quit game
		}
	}
}




//string FullFileName = Util::GetGamePath() + "03-Law of One-Indidginus.ogg"; // "09-Faerie tale-Indidginus.ogg";
//FILE* pOggFile( WiiFile::FileOpenForRead( FullFileName.c_str() ) );
//u32 OggSize = WiiFile::GetFileSize(pOggFile);
//u8* pOggData = (u8*) malloc(OggSize);
//fread( pOggData, OggSize, 1, pOggFile);
//PlayOgg(pOggData, OggSize, 0, OGG_ONE_TIME);

//-------------
//u8* pOggData = (u8*) malloc(222);
//memset(pOggData,0,4);
//int fd = open (FullFileName.c_str(), O_RDONLY ); // WRONLY);
//read( fd, pOggData, 4);
//printf("%x", pOggData[0] );
//printf("%x", pOggData[1] );
//printf("%x", pOggData[2] );
//printf("%x", pOggData[3] );
//close(fd);


