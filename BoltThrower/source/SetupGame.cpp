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
	
					//----------------------------------------------------------------------
					// The menu screen hs been set above, so now just set the message
					Util3D::Identity();
					Util3D::TransRot(-280,-150,0, M_PI *0.5f );
					m_pWii->GetFontManager()->DisplayTextCentre("Loading...", 
						0,0,
						200,HashString::SmallFont);
					GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);
					m_pWii->SwapScreen();  // to clear zbuffer keep GX_SetZMode on until after this call 
					//----------------------------------------------------------------------
					// next frame - now need to set the menu screen again before the text message
					m_pWii->GetMenuScreens()->DoMenuScreen();  // draw menu screen again
					Util3D::Identity();
					Util3D::TransRot(-280,-150,0, M_PI *0.5f );
					m_pWii->GetFontManager()->DisplayTextCentre("Loading...", 
						0,0,
						200,HashString::SmallFont);
					GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);
					m_pWii->SwapScreen();  // to clear zbuffer keep GX_SetZMode on until after this call 
					//----------------------------------------------------------------------

					m_pWii->NextMusic();

					continue; // don't do the loops screen swap since its already done
				}
				else if ( Name == HashString::download_extra_music )
				{
					extern bool DownloadFilesListedInConfiguration(bool);
					DownloadFilesListedInConfiguration(false);
					m_pWii->m_MusicStillLeftToDownLoad = false;
					m_pWii->BuildMenus(true);
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

					//printf("%d",m_pWii->GetIngameMusicVolume());

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
	if (m_pWii->GetMusicEnabled())
		m_pWii->SetMusicVolume( m_pWii->GetIngameMusicVolume() );
	else
		m_pWii->SetMusicVolume( 0 );


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

	}
}

void SetUpGame::MainLoop() 
{	
	m_pWii->SetGameState(WiiManager::eIntro);

	while (1)
	{
		m_pWii->SetMusicVolume( 5 ); // 0 to 5, 0 is off - 5 is max

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
