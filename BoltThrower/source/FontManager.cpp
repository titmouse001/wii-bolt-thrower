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

//hack
void FontManager::DisplayLargeTextVertCentre(const string& Text, int uXpos, int uYpos, u8 Alpha)
{
	uYpos -= GetFont(HashString::LargeFont)->GetHeight()/2;
	DisplayLargeText(Text, uXpos,uYpos,Alpha);
}
void FontManager::DisplayLargeTextCentre(const string& Text, int uXpos, int uYpos, u8 Alpha)
{
	uXpos -= GetTextWidth(Text,HashString::LargeFont)/2;
	uYpos -= GetFont(HashString::LargeFont)->GetHeight()/2;
	DisplayLargeText(Text, uXpos,uYpos,Alpha);
}

void FontManager::DisplaySmallTextCentre(const string& Text, int uXpos, int uYpos, u8 Alpha)
{
	uXpos -= GetTextWidth(Text,HashString::SmallFont)/2;
	uYpos -= GetFont(HashString::SmallFont)->GetHeight()/2;
	DisplaySmallText(Text, uXpos,uYpos,Alpha);
}

void FontManager::DisplayLargeText(const string& Text, int uXpos, int uYpos, u8 Alpha)
{
	WiiManager& Wii(Singleton<WiiManager>::GetInstanceByRef());
	Font* pFont(Wii.GetFontManager()->GetFont( HashString::LargeFont));

	//GX_LoadPosMtxImm (Singleton<WiiManager>::GetInstanceByRef().GetCamera()->GetcameraMatrix(), GX_PNMTX0); 

	int x(uXpos);
	for (string::const_iterator Iter(Text.begin()); Iter!=Text.end(); ++Iter )
	{
		const CharInfo* const  pChar ( pFont->GetChar( *Iter ) );

		x += pChar->m_iWidthA;
		if (pChar->GetImage() != NULL)
		{
			pChar->Draw(x, uYpos + pChar->m_iTopOffset,Alpha);
		}
		x += pChar->m_uWidthB; 
		x += pChar->m_iWidthC;
	}
}

void FontManager::DisplaySmallText(const string& Text, int uXpos, int uYpos, u8 Alpha)
{
	WiiManager& Wii(Singleton<WiiManager>::GetInstanceByRef());
	Font* pFont(Wii.GetFontManager()->GetFont(HashString::SmallFont));

	//GX_LoadPosMtxImm (Singleton<WiiManager>::GetInstanceByRef().GetCamera()->GetcameraMatrix(), GX_PNMTX0); 

	int x(uXpos);
	for (string::const_iterator Iter(Text.begin()); Iter!=Text.end(); ++Iter )
	{
		const CharInfo* const  pChar ( pFont->GetChar( *Iter ) );

		x += pChar->m_iWidthA;
		if (pChar->GetImage() != NULL)
		{
			pChar->Draw(x, uYpos + pChar->m_iTopOffset,Alpha);
		}
		x += pChar->m_uWidthB; 
		x += pChar->m_iWidthC;
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