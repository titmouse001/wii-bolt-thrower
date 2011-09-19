#include "Menu.h"
#include "WiiManager.h"
#include "FontManager.h"
#include "Util3D.h"
#include "InputDeviceManager.h"
#include "HashLabel.h"

//============================
// MENU MANAGER - section
//============================

MenuManager::MenuManager() : m_MenuGroupName("")
{
}

Menu* MenuManager::AddMenu(int x, int y, int w, int h, std::string Name, bool bShowTextOnly, bool bJustifyLeft)
{
	WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() );

	Menu* pMenu( new Menu );
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
	//WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() );

	std::vector<Menu*> thing(m_MenuContainer[ GetMenuGroup() ]);
	for (std::vector<Menu*>::iterator Iter(thing.begin()); Iter!=thing.end(); ++Iter)
	{
		Menu* pItem = *Iter;
		if ( (*Iter)->GetHashLabel() == Name )
		{
			pItem->NextItem();
			pItem->SetText( pItem->GetCurrentTextItem() );
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
		Menu* pItem = *Iter;
		if ( (*Iter)->GetHashLabel() == Name )
		{
			return pItem->GetText();
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
	//for (std::vector<Menu*>::iterator Iter(m_MenuContainer.begin()); Iter!=m_MenuContainer.end(); ++Iter)
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

	for (std::vector<Menu*>::iterator Iter(thing.begin()); Iter!=thing.end(); ++Iter)
	{
		Menu* pWorkingItem = *Iter; 
		while (pWorkingItem != NULL)
		{
			SDL_Rect rect = pWorkingItem->GetRect();
			if (pWorkingItem->GetShowTextOnly())
			{
				//Wii.GetFontManager()->DisplayLargeTextCentre(pWorkingItem->GetText().c_str(),rect.x, rect.y,180);
				Wii.GetFontManager()->DisplaySmallTextCentre(pWorkingItem->GetText().c_str(),rect.x, rect.y,180);
			}
			else
			{
				if (pWorkingItem->IsHighLight())
				{
					Wii.DrawRectangle(rect.x -(rect.w/2), rect.y- (rect.h/2), rect.w, rect.h, 160, 20,20,60);

// add yet aother hack to this lot  ... all this is bad code, needs refactor big time

//NEED TO DO SOMETHING LIKE THIS...
// TL, TM, TR
// CL, CM, CR
// BL, BM, BR

					if ( pWorkingItem->GetJustifyLeft() )
					{
						float w =( rect.w * (1.0f-0.9f))*0.5f;
						w+=28.0f;
						//Wii.GetFontManager()->DisplayLargeTextVertCentre(pWorkingItem->GetText().c_str(),rect.x-(rect.w/2)+w, rect.y,222);
						Wii.GetFontManager()->DisplayLargeTextVertCentre(pWorkingItem->GetText().c_str(),w+rect.x-(rect.w/2), rect.y,222);
					}
					else
						Wii.GetFontManager()->DisplayLargeTextCentre(pWorkingItem->GetText().c_str(),rect.x, rect.y,222);
				}
				else
				{
					rect.w*=0.9f;  // fudge something todo later, if I ever do!

					Wii.DrawRectangle(rect.x-(rect.w/2), rect.y- (rect.h/2), rect.w, rect.h, 120,10,10,30);
					if ( pWorkingItem->GetJustifyLeft() )
					{
						float w = 28.0f;
					//	float w = rect.w * (1.0f-0.9f);
					//	Wii.GetFontManager()->DisplayLargeTextVertCentre(pWorkingItem->GetText().c_str(),rect.x-(rect.w/2)+w, rect.y,128);
						Wii.GetFontManager()->DisplayLargeTextVertCentre(pWorkingItem->GetText().c_str(),w+rect.x-(rect.w/2), rect.y,128);
					}
					else
						Wii.GetFontManager()->DisplayLargeTextCentre(pWorkingItem->GetText().c_str(),rect.x, rect.y,128);

				}
			}
		
			pWorkingItem = pWorkingItem->GetChildMenu();  // any children to work through
		}
	}
}

//============================
// MENU - section
//============================

Menu::Menu() : m_ChildMenu(NULL), m_Active(NULL), 
m_Disable(false), m_HighLight(false), m_bSelected(false),m_HashLabel(""), m_bShowTextOnly(false),m_bJustifyLeft(false)
{
}

//////Menu* Menu::AddChildMenu(int w, int h, std::string Name)
//////{
//////	Menu* pChild( new Menu );
//////	pChild->SetText(Name);
//////	pChild->SetHashLabel( HashLabel(Name) );
//////
//////	if (Name == "_" )
//////	{
//////		pChild->SetMenu(GetRect().x,GetRect().y+GetRect().h,w,h/2);
//////		pChild->SetDisable(true);
//////	}
//////	else
//////		pChild->SetMenu(GetRect().x,GetRect().y+GetRect().h,w,h);
//////
//////	SetChildMenu(pChild);
//////	return pChild;
//////}


bool Menu::IsOverMenuItem(int x, int y)
{
	SDL_Rect& Rect( GetRect() );

	int Rx = Rect.x - (Rect.w/2);
	int Ry = Rect.y - (Rect.h/2);

	if ((x < Rx + Rect.w) && (x >= Rx ) && (y < Ry + Rect.h) && (y >= Ry ))
		return true;
	else
		return false;
}


void Menu::SetMenu(int x, int y, int w, int h)
{
	m_Rect.x = x;
	m_Rect.y = y;
	m_Rect.w = w;
	m_Rect.h = h;
}


	
void Menu::NextItem() 
{
	m_CurrentItemIndex++; 
	if (m_CurrentItemIndex >= m_TextItems.size()) 
		m_CurrentItemIndex=0;  
}

//////////thisis now broken..FindMenuItemOver finds child! 
////////bool MenuManager::AnyParentMenuActive()
////////{
//////////	WiiManager& rManager( Singleton<WiiManager>::GetInstanceByRef() );
//////////	Menu* pItem = FindMenuItemOver( rManager.GetMouseX(),rManager.GetMouseY() );
////////	for (std::vector<Menu*>::iterator Iter(m_MenuContainer.begin()); Iter!=m_MenuContainer.end(); ++Iter)
////////	{
////////		if ( (*Iter)->IsActive() )
////////			return true;
////////	}
////////	return false;
////////}

//////void MenuManager::ClearAllHighLightMenus()
//////{
//////	for (std::vector<Menu*>::iterator Iter(m_MenuContainer.begin()); Iter!=m_MenuContainer.end(); ++Iter)
//////	{
//////		(*Iter)->SetHighLight(false);
//////		Menu* pChildItem = (*Iter)->GetChildMenu(); 
//////		while (pChildItem != NULL)
//////		{
//////			pChildItem->SetHighLight(false);
//////			pChildItem = pChildItem->GetChildMenu();
//////		}
//////	}
//////}


////////void Menu::ClearSelectForChildren(bool bState)
////////{
////////	Menu* pChildItem = this; 
////////	while (pChildItem != NULL)
////////	{
////////		pChildItem->SetSelected(bState); 
////////		pChildItem = pChildItem->GetChildMenu();
////////	}
////////}
////////
////////



//////
//////
//////Menu* MenuManager::FindActiveMenuItemOver(int x, int y)
//////{
//////	for (std::vector<Menu*>::iterator Iter(m_MenuContainer.begin()); Iter!=m_MenuContainer.end(); ++Iter)
//////	{
//////		if ((*Iter)->IsActive())
//////		{
//////			Menu* pChildItem = *Iter;
//////			while (pChildItem != NULL)
//////			{
//////				if (pChildItem->IsOverMenuItem(x,y) )
//////				{
//////					return pChildItem;
//////				}
//////				pChildItem = pChildItem->GetChildMenu();
//////			}
//////		}
//////	}
//////	return NULL;
//////}
//////
//////Menu* MenuManager::FindMenuItemOver(int x, int y)
//////{
//////	// reverse iter to help lessen overlapping menu by looking from right to left
//////	for (std::vector<Menu*>::reverse_iterator Iter(m_MenuContainer.rbegin()); Iter!=m_MenuContainer.rend(); ++Iter)
//////	{
//////		Menu* pChildItem = *Iter;
//////		while (pChildItem != NULL)
//////		{
//////			if (pChildItem->IsOverMenuItem(x,y) )
//////			{
//////				return pChildItem;
//////			}
//////			pChildItem = pChildItem->GetChildMenu();
//////		}
//////	}
//////	return NULL;
//////}