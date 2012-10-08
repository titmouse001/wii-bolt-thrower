#include "Menu.h"
#include "WiiManager.h"
#include "FontManager.h"
#include "Util3D.h"
#include "InputDeviceManager.h"
#include "HashLabel.h"

#include "camera.h"
#include "MenuManager.h"
#include "Draw_Util.h"

MenuManager::MenuManager() : m_MenuGroupName("") 
{
}

void MenuManager::Init() 
{
	m_pWii = Singleton<WiiManager>::GetInstanceByPtr();
}

Menu* MenuManager::AddMenu(int x, int y, int w, int h, std::string Name, bool bShowTextOnly, bool bJustifyLeft, HashLabel FontSize )
{
	WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() );

	Menu* pMenu( new Menu );

	pMenu->m_TextSize = FontSize;

	pMenu->SetMenu(x,y,w,h);
	pMenu->SetText(Wii.GetText(Name));
	pMenu->SetHashLabel( HashLabel(Name) );
	pMenu->SetShowTextOnly( bShowTextOnly );
	pMenu->SetJustifyLeft( bJustifyLeft );
	m_MenuContainer[ GetMenuGroup() ].push_back( pMenu );

	return pMenu;
}

void MenuManager::MenuLogic()
{
	WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() );
	Vtx* pWiiMote( Wii.GetInputDeviceManager()->GetIRPosition() );

	if (pWiiMote != NULL)
	{
		int x =  pWiiMote->x - (Wii.GetScreenWidth()/2);
		int y =  pWiiMote->y - (Wii.GetScreenHeight()/2);

		std::vector<Menu*> thing(m_MenuContainer[ GetMenuGroup() ]);

		for (std::vector<Menu*>::iterator Iter(thing.begin()); Iter!=thing.end(); ++Iter)
		{
			Menu* pItem = *Iter;
			while (pItem != NULL && !pItem->GetShowTextOnly() )
			{
				if (pItem->IsOverMenuItem(x,y))
					pItem->SetHighLight();
				else
					pItem->SetHighLight(false);

				pItem = pItem->GetChildMenu();  // any children to work through
			}
		}
	}
}

void MenuManager::AdvanceMenuItemText(HashLabel Name)
{
	std::vector<Menu*> thing(m_MenuContainer[ GetMenuGroup() ]);
	for (std::vector<Menu*>::iterator Iter(thing.begin()); Iter!=thing.end(); ++Iter)
	{
		if ( (*Iter)->GetHashLabel() == Name )
		{
			(*Iter)->NextItem();
			(*Iter)->SetText( (*Iter)->GetCurrentTextItem() );
			break;
		}
	}
}

int MenuManager::GetMenuItemIndex(HashLabel Name)
{
	Menu* pMenu( GetMenuItem(Name) );
	if (pMenu!=NULL)
		return pMenu->GetCurrentItemIndex();
	else
		return 0;

	//std::vector<Menu*> thing(m_MenuContainer[ GetMenuGroup() ]);
	//for (std::vector<Menu*>::iterator Iter(thing.begin()); Iter!=thing.end(); ++Iter)
	//{
	//	if ( (*Iter)->GetHashLabel() == Name )
	//	{
	//		return (*Iter)->GetCurrentItemIndex();
	//	}
	//}

	//return 90;
}



Menu* MenuManager::GetMenuItem(HashLabel Name)
{
	std::vector<Menu*> thing(m_MenuContainer[ GetMenuGroup() ]);
	for (std::vector<Menu*>::iterator Iter(thing.begin()); Iter!=thing.end(); ++Iter)
	{
		if ( (*Iter)->GetHashLabel() == Name )
			return *Iter;
	}

	return NULL;
}



std::string MenuManager::GetMenuItemText(HashLabel Name)
{
	std::vector<Menu*> thing(m_MenuContainer[ GetMenuGroup() ]);
	for (std::vector<Menu*>::iterator Iter(thing.begin()); Iter!=thing.end(); ++Iter)
	{
		if ( (*Iter)->GetHashLabel() == Name )
		{
			return (*Iter)->GetText();
		}
	}
	return "-";
}

void MenuManager::SetMenuItemText(HashLabel Name, std::string Text)
{
	std::vector<Menu*> thing(m_MenuContainer[ GetMenuGroup() ]);
	for (std::vector<Menu*>::iterator Iter(thing.begin()); Iter!=thing.end(); ++Iter)
	{
		Menu* pItem = *Iter;
		if ( (*Iter)->GetHashLabel() == Name )
		{
			pItem->SetText(Text);
		}
	}
}

HashLabel MenuManager::GetSelectedMenu()
{
	std::vector<Menu*> thing(m_MenuContainer[ GetMenuGroup() ]);
	for (std::vector<Menu*>::iterator Iter(thing.begin()); Iter!=thing.end(); ++Iter)
	{
		Menu* pItem = *Iter;
		if ( (*Iter)->IsHighLight() )
		{
			return pItem->GetHashLabel();
		}
	}
	return HashString::BLANK;
}

void MenuManager::Draw()
{
	WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() );
	Util3D::TransRot(Wii.GetCamera()->GetCamX(),Wii.GetCamera()->GetCamY(),0,0);

	std::vector<Menu*> thing(m_MenuContainer[ GetMenuGroup() ]);

	int Down( 0);
//	int Up( 0);

	for (std::vector<Menu*>::iterator Iter(thing.begin()); Iter!=thing.end(); ++Iter)
	{
		if ((*Iter)->IsHighLight())
		{
		//	Down = -30;
			break;
		}
	}

	for (std::vector<Menu*>::iterator Iter(thing.begin()); Iter!=thing.end(); ++Iter)
	{
		Menu* pWorkingItem = *Iter; 
		while (pWorkingItem != NULL)
		{
			SDL_Rect rect = pWorkingItem->GetRect();
			if (pWorkingItem->GetShowTextOnly())
			{
				Wii.GetFontManager()->DisplayTextCentre(pWorkingItem->GetText().c_str(),rect.x, rect.y,180,HashString::SmallFont);
			}
			else
			{
				if (pWorkingItem->IsHighLight())
				{
					rect.w*=1.15f; 
					rect.h*=1.10f; 
					Draw_Util::DrawRectangle(rect.x-(rect.w/2), Down + rect.y- (rect.h/2), rect.w, rect.h, 160, 20,20,60);

// add yet aother hack to this lot  ... all this is bad code, needs refactor big time
//NEED TO DO SOMETHING LIKE THIS...
// TL, TM, TR
// CL, CM, CR
// BL, BM, BR
					HashLabel Size = HashString::SmallFont; //pWorkingItem->m_TextSize;
					if ( pWorkingItem->GetJustifyLeft() )
					{
						float w =( rect.w * (1.0f-0.9f))*0.5f;
						w+=28.0f;
						Wii.GetFontManager()->DisplayTextVertCentre(pWorkingItem->GetText().c_str(),w+rect.x-(rect.w/2), rect.y,222, Size);
					}
					else
						Wii.GetFontManager()->DisplayTextCentre(pWorkingItem->GetText().c_str(),rect.x, rect.y,222,Size);
				}
				else
				{
				//	rect.h*=0.9f;
				//	rect.w*=0.9f;  // fudge something todo later, if I ever do!

					Draw_Util::DrawRectangle(rect.x-(rect.w/2), Down + rect.y- (rect.h/2), rect.w, rect.h, 120,10,10,30);
					if ( pWorkingItem->GetJustifyLeft() )
					{
						float w = 28.0f;
						Wii.GetFontManager()->DisplayTextVertCentre(pWorkingItem->GetText().c_str(),
							w+rect.x-(rect.w/2), Down + rect.y,128,pWorkingItem->m_TextSize);
					}
					else
						Wii.GetFontManager()->DisplayTextCentre(pWorkingItem->GetText().c_str(),
						rect.x, Down + rect.y,128,pWorkingItem->m_TextSize);
				}
			}
			pWorkingItem = pWorkingItem->GetChildMenu();  // any children to work through
		}
	}
}



void MenuManager::BuildMenus(bool KeepSettings)
{
	int Music = 1; 
	int Difficulty = 1;
	int Language = 0;
	int MusicVolume = 3;
	string Group = GetMenuGroup();   // nasty code... GetMenuItemIndex() depends on this setting.

	SetMenuGroup("OptionsMenu");

	if ( KeepSettings )
	{
		Music = GetMenuItemIndex(HashString::IngameMusicState);
		Difficulty = GetMenuItemIndex(HashString::DifficultySetting);
		Language = GetMenuItemIndex(HashString::LanguageSetting);
		MusicVolume = GetMenuItemIndex(HashString::IngameMusicVolumeState);
	}

	m_pWii->SetMusicEnabled( (bool) Music );
	m_pWii->SetIngameMusicVolume( MusicVolume );

	ClearMenus();

	//========================================
	// Main Menu - one time setup
	int y=-(60);
	int step=24+8+4+2;
	int height=24+4;
	float width=180;
	
	SetMenuGroup("MainMenu");     // nasty code... AddMenu() depends on this setting.

	AddMenu(-width*0.10, y, width,height,"Start_Game",false,true);
	y+=step;
	AddMenu(-width*0.15, y, width,height,"Options",false,true);
	y+=step;
	AddMenu(-width*0.20, y, width,height,"Intro",false,true);
	y+=step;
	//move this one into options

	if ( m_pWii->GetMusicFilesContainerSize() > 1)
		AddMenu( -width*0.25, y, width,height,"Change_Tune",false,true);
	else
		AddMenu( -width*0.25, y, width,height,"Change_Tune",true,false);
	
	if (m_pWii->m_MusicStillLeftToDownLoad)
		AddMenu( 160, y, 220,height,"download_extra_music",false,false);

	y+=step;
	AddMenu( -width*0.30, y, width,height ,"Controls",false,true );
	y+=step;
	AddMenu( -width*0.35, y, width,height ,"Credits",false,true );

	//==========================================================
	// Options Menu - one time setup
	SetMenuGroup("OptionsMenu");

	int x=0; // centre of screen
	y=-98;
	height=26;
	step=26+8;

	//GetMenuManager()->AddMenu( x - 108, y, 200, height ,"Credits" );
	//GetMenuManager()->AddMenu( x + 108, y, 200, height ,"Controls" );
	y+=step;

	AddMenu(x, y, 600, height, "Ingame_Music",false,true);
	AddMenu(x+222, y, 1, height, "IngameMusicState",true)->
		AddTextItem( m_pWii->GetText("off") )->AddTextItem( m_pWii->GetText("on"))->SetCurrentItemIndex(Music);

	y+=step;
	AddMenu(x, y, 600, height, "Ingame_MusicVolume",false,true);
	AddMenu(x+222, y, 1, height, "IngameMusicVolumeState",true)->
		AddTextItem(("0"))->AddTextItem(("1"))->
		AddTextItem(("2"))->AddTextItem(("3"))->
		AddTextItem(("4"))->AddTextItem(("5"))->SetCurrentItemIndex(MusicVolume);

	y+=step;
	AddMenu(x, y , 600, height, "Difficulty_Level",false,true );
	AddMenu(x+222, y, 1, height, "DifficultySetting",true)->
		AddTextItem( m_pWii->GetText("easy"))->AddTextItem( m_pWii->GetText("medium"))->
		AddTextItem( m_pWii->GetText("hard"))->SetCurrentItemIndex(Difficulty);

	y+=step;
	AddMenu(x, y , 600, height, "Set_Language",false,true);
	// GetMenuManager()->AddMenu(x+300, y, 1, height, "LanguageSetting",true)->AddTextItem("English")->AddTextItem("Italian")->AddTextItem("Esperanto")->SetCurrentItemIndex(0);
	if ( m_pWii->GetSupportedLanguagesEmpty() == false )
	{
		Menu* NextItem = NULL; 
		//TODO - typedef this !!!
		for (map<string, map<string,string> >::iterator IterSupportedLanguages( m_pWii->GetSupportedLanguagesBegin() ) ;
			IterSupportedLanguages != m_pWii->GetSupportedLanguagesEnd(); ++IterSupportedLanguages)
		{
			// (first is the language, second english word we wish to find in that language
			if (NextItem==NULL)
				NextItem= AddMenu(x+222, y, 1, height, "LanguageSetting",true)->AddTextItem(IterSupportedLanguages->first) ;
			else
				NextItem = NextItem->AddTextItem(IterSupportedLanguages->first) ;
		}
		NextItem->SetCurrentItemIndex(Language); // set the fisrt one as the default language
	}
	y+=step+30;
	AddMenu(0, y , 600, height, "Back");
	SetMenuGroup( Group );
}
