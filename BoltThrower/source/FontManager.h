#ifndef FontManager_H_
#define FontManager_H_


#include "HashString.h"
class Font;

class FontManager
{

public:

	FontManager()
	{
		M_FontColour.a = 0xff;
		M_FontColour.r = 0xff;
		M_FontColour.g = 0xff;
		M_FontColour.b = 0xff;
	}

	void LoadFont( std::string FullFileNameWithPath, std::string LookUpName );
	Font* GetFont(HashLabel Font);

	void SetFontColour(u8 Red,u8 Green,u8 Blue, u8 Alpha = 0xff) { M_FontColour.r=Red; M_FontColour.g=Green, M_FontColour.b=Blue; M_FontColour.a= Alpha; }

	GXColor GetFontColour() const  { return M_FontColour; }


	void DisplayText(const string& Text, int uXpos, int uYpos,u8 Alpha=128,HashLabel FontSize = HashString::LargeFont);
	void DisplayText(const string& Text, int uXpos, int uYpos, HashLabel FontSize = HashString::LargeFont);
	//void DisplaySmallText(const string& Text, int uXpos, int uYpos, u8 Alpha=128);

	void	DisplayTextCentre(const string& Text, int uXpos, int uYpos, u8 Alpha=222, HashLabel FontType = HashString::LargeFont);
	void	DisplayTextVertCentre(const string& Text, int uXpos, int uYpos, u8 Alpha=222, HashLabel FontType = HashString::LargeFont);

	void	DisplayTextCentre(const string& Text, int uXpos, int uYpos, HashLabel FontSize);

	int GetTextWidth(const string& Text, HashLabel FontType = HashString::SmallFont);
	////void	DisplayLargeTextCentre(const string& Text, int uXpos, int uYpos, u8 Alpha=222);
	////void	DisplaySmallTextCentre(const string& Text, int uXpos, int uYpos, u8 Alpha=222);
	////void	DisplayLargeTextVertCentre(const string& Text, int uXpos, int uYpos, u8 Alpha=222);
	////void	DisplaySmallTextVertCentre(const string& Text, int uXpos, int uYpos, u8 Alpha=222);

private:


//	typedef vector<Font*>	TFontDetailsList;
//	TFontDetailsList		m_vFontData;

	std::map<HashLabel,Font*> m_FontContainer;


	GXColor M_FontColour;

};


#endif
