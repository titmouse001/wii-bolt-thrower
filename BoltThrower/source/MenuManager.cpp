#include "Menu.h"
#include "WiiManager.h"
#include "FontManager.h"
#include "Util3D.h"
#include "InputDeviceManager.h"
#include "HashLabel.h"

#include "MenuManager.h"

MenuManager::MenuManager() : m_MenuGroupName("")
{
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
	std::vector<Menu*> thing(m_MenuContainer[ GetMenuGroup() ]);
	for (std::vector<Menu*>::iterator Iter(thing.begin()); Iter!=thing.end(); ++Iter)
	{
		if ( (*Iter)->GetHashLabel() == Name )
		{
			return (*Iter)->GetCurrentItemIndex();
		}
	}

	return 0;
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
				//	Down = rect.h;

					//Wii.DrawRectangle(rect.x -(rect.w/2), rect.y- (rect.h/2), rect.w, rect.h, 160, 20,20,60);
					Wii.DrawRectangle(rect.x -(rect.w/2), rect.y - rect.h * (0.5f*1.4f), rect.w, rect.h*1.4f, 160, 20,20,60);

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
					rect.w*=0.9f;  // fudge something todo later, if I ever do!

					Wii.DrawRectangle(rect.x-(rect.w/2), Down + rect.y- (rect.h/2), rect.w, rect.h, 120,10,10,30);
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