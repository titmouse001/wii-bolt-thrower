#include "CharInfo.h"
#include "Image.h"
#include "WiiManager.h"
#include "FontManager.h"

void CharInfo::Draw(int uXpos, int uYpos, GXColor& Colour) const
{ 
	m_pImage->DrawImageTL(uXpos, uYpos, Colour); 
}