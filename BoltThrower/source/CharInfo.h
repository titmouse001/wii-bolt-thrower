#ifndef Char_H_
#define Char_H_

#include "GCTypes.h"
class Image;

class CharInfo
{
private:

	Image*		m_pImage;

public:
	u16			m_uWidth;
	u16			m_uHeight;
	s16			m_iWidthA;
	u16			m_uWidthB;
	s16			m_iWidthC;
	s16			m_iLeftOffset;
	s16			m_iTopOffset;

	void		SetImage(Image* pImage) {m_pImage = pImage;}
	Image*		GetImage() const {return m_pImage;} 

	void		Draw(int uXpos, int uYpos, u32 uAlpha = 255) const;
};

#endif