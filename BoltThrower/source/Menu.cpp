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

Menu* MenuManager::AddMenu(int x, int y, int w, int h, std::string Name, bool JustText)
{
	Menu* pMenu( new Menu );
	pMenu->SetMenu(x,y,w,h);
	pMenu->SetText(Name);
	pMenu->SetHashLabel( HashLabel(Name) );
	pMenu->SetJustText( JustText );
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
			while (pItem != NULL && !pItem->GetJustText() )
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
		Menu* pItem = *Iter;
		if ( (*Iter)->GetHashLabel() == Name )
		{
			pItem->NextItem();
			pItem->SetText( pItem->GetCurrentTextItem() );
			break;
		}
	}
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
			if (pWorkingItem->GetJustText())
			{
				Wii.GetFontManager()->DisplayLargeTextCentre(pWorkingItem->GetText().c_str(),rect.x, rect.y,180);
			}
			else
			{
				if (pWorkingItem->IsHighLight())
				{
					Wii.DrawRectangle(rect.x -(rect.w/2), rect.y- (rect.h/2), rect.w, rect.h, 160, 20,20,60);
					Wii.GetFontManager()->DisplayLargeTextCentre(pWorkingItem->GetText().c_str(),rect.x, rect.y,222);
				}
				else
				{
					rect.w*=0.9f;
					Wii.DrawRectangle(rect.x-(rect.w/2), rect.y- (rect.h/2), rect.w, rect.h, 120,10,10,30);
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
m_Disable(false), m_HighLight(false), m_bSelected(false),m_HashLabel(""), m_bJustText(false)
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