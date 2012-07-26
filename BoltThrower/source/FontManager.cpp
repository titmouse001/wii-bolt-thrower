#include <gccore.h>
#include <stdio.h>
#include "WiiManager.h"
#include "Image.h"
#include "debug.h"
#include "ImageManager.h"
#include "InputDeviceManager.h"

#include "FontManager.h"
#include "Font.h"
#include "HashString.h"

void FontManager::LoadFont( std::string FullFileNameWithPath, std::string LookUpName )
{
	m_FontContainer[ (HashLabel)LookUpName ] = new Font(FullFileNameWithPath)  ;
}

Font* FontManager::GetFont(HashLabel Font)
{
	return m_FontContainer[ Font ];
}

void FontManager::DisplayTextVertCentre(const string& Text, int uXpos, int uYpos, u8 Alpha, HashLabel FontSize)
{
	uYpos -= GetFont(FontSize)->GetHeight()/2;
	DisplayText(Text, uXpos,uYpos,Alpha,FontSize);
}
void FontManager::DisplayTextCentre(const string& Text, int uXpos, int uYpos, u8 Alpha, HashLabel FontSize)
{
	uXpos -= GetTextWidth(Text,FontSize)/2;
	uYpos -= GetFont(FontSize)->GetHeight()/2;
	DisplayText(Text, uXpos,uYpos,Alpha,FontSize);
}

void FontManager::DisplayTextCentre(const string& Text, int uXpos, int uYpos, HashLabel FontSize)
{
	uXpos -= GetTextWidth(Text,FontSize)/2;
	uYpos -= GetFont(FontSize)->GetHeight()/2;
	DisplayText(Text, uXpos, uYpos, FontSize);
}

void FontManager::DisplayText(const string& Text, int uXpos, int uYpos, HashLabel FontSize)
{
	WiiManager& Wii(Singleton<WiiManager>::GetInstanceByRef());
	Font* pFont(Wii.GetFontManager()->GetFont( FontSize ));

#if defined (BUILD_FINAL_RELEASE)
	if (pFont==NULL)
		return;  // just incase - need to display this but we are still loading font (safegard again FUTURE loading times!!!)
	// (note: small font is loaded well before any threads startup)
#else
	if (pFont==NULL)
		ExitPrintf("Font missing");
#endif


	for (string::const_iterator Iter(Text.begin()); Iter!=Text.end(); ++Iter )
	{
		const CharInfo* const  pChar ( pFont->GetChar( *Iter ) );

		uXpos += pChar->m_iWidthA;
		if (pChar->GetImage() != NULL)
		{
			pChar->Draw( uXpos, uYpos + pChar->m_iTopOffset, M_FontColour );
		}
		uXpos += pChar->m_uWidthB + pChar->m_iWidthC;
	}
}

void FontManager::DisplayText(const string& Text, int uXpos, int uYpos, u8 Alpha, HashLabel FontSize)
{
	M_FontColour.a = Alpha;
	DisplayText(Text, uXpos, uYpos, FontSize);
}


int FontManager::GetTextWidth(const string& Text, HashLabel FontType)
{
	WiiManager& Wii(Singleton<WiiManager>::GetInstanceByRef());
	Font* pFont( Wii.GetFontManager()->GetFont(FontType) ); // HashString::SmallFont));
	int Width(0);
	for (string::const_iterator Iter(Text.begin()); Iter!=Text.end(); ++Iter )
	{
		const CharInfo* const  pChar ( pFont->GetChar( *Iter ) );
		Width += pChar->m_iWidthA;
		Width += pChar->m_uWidthB; 
		Width += pChar->m_iWidthC;
//		Width +=  pChar->m_uWidth;
	}
	return Width;
}