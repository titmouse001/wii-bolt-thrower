#ifndef Image_H
#define Image_H

#include "GCTypes.h"
#include "ogc/gx.h"
#include "hashlabel.h"
#include <malloc.h>
#include <string.h>
#include <gccore.h>
#include <string>
using namespace std;

class CharInfo;
class Vessel;


class Image
{
public:

	Image();
	Image(string pFileName);
	Image(u32 Width, u32 Height);
	Image(CharInfo* pChar, u8* pAlphaData);
	Image(u8* pTgaData, int Width , int Height);

	void ImageLoad(const char* pFileName);
	u32* GetImageData() const { return m_ImageData; }

	u16 GetWidth() const { return m_Width; }
	u16 GetHeight() const { return m_Height; }
	s16 GetOriginX() const { return m_OriginX; }
	s16 GetOriginY() const { return m_OriginY; }

	void RefreshImage();
	void RemoveImage();
	void SetImageTo4x4RGBA(int x, int y, GXColor color);

	GXTexObj* GetHardwareTextureInfo() { return &m_HardwareTextureInfo; }

	void DrawImageXYZ(f32 xpos, f32 ypos ,f32 z, u8 Alpha, f32 rad, f32 scale ) ;
	void DrawImageXYZ(f32 xpos, f32 ypos ,f32 z, u8 Alpha = 0xff, f32  rad=0);
	void DrawImage(Vessel& item);
	
	void DrawImage();
	void DrawImage(u8 AlphaValue);

	void DrawImage(f32 xpos, f32 ypos);
	void DrawImageTL(f32 xpos, f32 ypos, u8 Alpha = 0xff);

	void SetFileName(std::string Name) { m_FileNameAsHash = (HashLabel)Name; }
	HashLabel GetFileName() const { return m_FileNameAsHash; }

	//void DrawImage2(f32 xpos, f32 ypos ,u8 Alpha );
private:

	u16			m_Width;
	u16			m_Height;
	s16			m_OriginX;
	s16			m_OriginY;
	u32*		m_ImageData;
	HashLabel	m_FileNameAsHash;
	GXTexObj	m_HardwareTextureInfo;
};



#endif
