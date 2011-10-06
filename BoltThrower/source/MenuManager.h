#ifndef MenuManager_H_
#define MenuManager_H_

#include "GCTypes.h"
#include "HashLabel.h"
#include <string>
#include <map>
#include <vector>

class Menu;

//struct SDL_Rect {int x,y,w,h; };

class MenuManager
{
public:
	MenuManager();
	Menu*		AddMenu(int x, int y, int w, int h, std::string Name, bool m_bShowTextOnly = false, bool m_JustifyLeft= false);

	void		Draw();
	void		MenuLogic();
	HashLabel	GetSelectedMenu();
	void		SetMenuGroup(std::string Group) { m_MenuGroupName = Group; }
	std::string GetMenuGroup() const  { return m_MenuGroupName ;}
	void		SetMenuItemText(HashLabel Name, std::string Text);
	std::string GetMenuItemText(HashLabel Name);
	void		AdvanceMenuItemText(HashLabel Name);
	Menu*		GetMenuItem(HashLabel Name);
	int			GetMenuItemIndex(HashLabel Name);

	void		ClearMenus() { m_MenuContainer.clear(); }

private:
	std::string m_MenuGroupName;
	std::map< std::string, std::vector<Menu*> > m_MenuContainer;
};


#endif
