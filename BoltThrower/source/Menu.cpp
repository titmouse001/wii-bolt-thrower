#include "Menu.h"
#include "WiiManager.h"
#include "FontManager.h"
#include "Util3D.h"
#include "InputDeviceManager.h"
#include "HashLabel.h"

//============================
// MENU - section
//============================

Menu::Menu() : m_ChildMenu(NULL), m_Active(NULL), 
m_Disable(false), m_HighLight(false), m_bSelected(false),m_HashLabel(""), m_bShowTextOnly(false),m_bJustifyLeft(false)
{
}

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
