#include "CharInfo.h"
#include "Image.h"
#include "WiiManager.h"
#include "FontManager.h"

void CharInfo::Draw(int uXpos, int uYpos, u32 uAlpha) const
{ 
	m_pImage->DrawImageTL(uXpos,uYpos,uAlpha); 
}