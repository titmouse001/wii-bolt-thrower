#ifndef Menu_H_
#define Menu_H_

#include "GCTypes.h"
#include "HashLabel.h"
#include <string>
#include <map>
#include <vector>

struct SDL_Rect {int x,y,w,h; };

class Menu
{
public:
	Menu();
	void SetMenu(int x, int y, int w, int h);
	void SetText(std::string Name) { m_Name = Name; }
	std::string GetText() const { return m_Name; }
	SDL_Rect& GetRect() { return m_Rect; }

	//Menu*	AddChildMenu(int w, int h, std::string Name);
	void	SetChildMenu(Menu* pMenu) { m_ChildMenu = pMenu; } 
	Menu*	GetChildMenu() const  {return m_ChildMenu;}
	void SetActive(bool bState) { m_Active = bState; }
	bool IsActive() const { return m_Active; }
	void SetHighLight(bool bState = true) { m_HighLight = bState; }
	bool IsHighLight() const { return m_HighLight; }
	bool GetDisable() const { return m_Disable; }
	void SetDisable(bool bState = true) { m_Disable = bState; }
	void SetSelected(bool bState) { m_bSelected = bState; }
	bool IsSelected() const { return m_bSelected; }
	bool	IsOverMenuItem(int x, int y);
	void SetHashLabel(HashLabel Hash) { m_HashLabel = Hash; }
	HashLabel GetHashLabel() const { return m_HashLabel; }

	void SetShowTextOnly(bool Value) { m_bShowTextOnly = Value; }
	bool GetShowTextOnly() { return m_bShowTextOnly; }
	void SetJustifyLeft(bool Value) { m_bJustifyLeft = Value; }
	bool GetJustifyLeft() { return m_bJustifyLeft; }

	Menu* AddTextItem(std::string Name) { m_TextItems.push_back(Name); return this; }
	std::string GetCurrentTextItem() { return m_TextItems[m_CurrentItemIndex];  }
	int  GetCurrentItemIndex() { return m_CurrentItemIndex;  }
	void SetCurrentItemIndex(int Value) { m_CurrentItemIndex = Value; SetText(GetCurrentTextItem()) ; }
	void NextItem();

private:
	Menu*		m_ChildMenu;
	SDL_Rect	m_Rect;
	std::string	m_Name;
	bool		m_Active;
	bool		m_Disable;
	bool		m_HighLight;
	bool		m_bSelected;
	HashLabel	m_HashLabel;
	std::vector<std::string> m_TextItems;
	u32		 m_CurrentItemIndex;
	bool		m_bShowTextOnly;
	bool		m_bJustifyLeft;
};



#endif
