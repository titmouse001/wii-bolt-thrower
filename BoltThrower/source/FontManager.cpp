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

//////hack
////void FontManager::DisplayLargeTextVertCentre(const string& Text, int uXpos, int uYpos, u8 Alpha)
////{
////	uYpos -= GetFont(HashString::LargeFont)->GetHeight()/2;
////	DisplayText(Text, uXpos,uYpos,Alpha,HashString::LargeFont);
////}
////void FontManager::DisplayLargeTextCentre(const string& Text, int uXpos, int uYpos, u8 Alpha)
////{
////	uXpos -= GetTextWidth(Text,HashString::LargeFont)/2;
////	uYpos -= GetFont(HashString::LargeFont)->GetHeight()/2;
////	DisplayText(Text, uXpos,uYpos,Alpha,HashString::LargeFont);
////}
////
////void FontManager::DisplaySmallTextCentre(const string& Text, int uXpos, int uYpos, u8 Alpha)
////{
////	uXpos -= GetTextWidth(Text,HashString::SmallFont)/2;
////	uYpos -= GetFont(HashString::SmallFont)->GetHeight()/2;
////	DisplayText(Text, uXpos,uYpos,Alpha,HashString::SmallFont);
////}
////
////void FontManager::DisplaySmallTextVertCentre(const string& Text, int uXpos, int uYpos, u8 Alpha)
////{
////	uYpos -= GetFont(HashString::SmallFont)->GetHeight()/2;
////	DisplayText(Text, uXpos,uYpos,Alpha,HashString::SmallFont);
////}

void FontManager::DisplayText(const string& Text, int uXpos, int uYpos, u8 Alpha, HashLabel FontSize)
{
	WiiManager& Wii(Singleton<WiiManager>::GetInstanceByRef());
	Font* pFont(Wii.GetFontManager()->GetFont( FontSize ));

	for (string::const_iterator Iter(Text.begin()); Iter!=Text.end(); ++Iter )
	{
		const CharInfo* const  pChar ( pFont->GetChar( *Iter ) );

		uXpos += pChar->m_iWidthA;
		if (pChar->GetImage() != NULL)
		{
			pChar->Draw(uXpos, uYpos + pChar->m_iTopOffset,Alpha);
		}
		uXpos += pChar->m_uWidthB + pChar->m_iWidthC;
	}
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