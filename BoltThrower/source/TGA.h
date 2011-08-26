#ifndef _TGA_H
#define _TGA_H

//#include <stdio.h>
#include "GCTypes.h"
//#include "utils.h"
#include <string>

namespace Tga
{
	
//	#define RGB_TO_565(r,g,b) (u16) ((r & 0xf8 )<<8) | ((g&0xfc)<<3) | ((b&0xf8)>>3)

	struct TGA_HEADER
	{
	   char  idlength;
	   char  colourmaptype;
	   char  datatypecode;
	   short int colourmaporigin;
	   short int colourmaplength;
	   char  colourmapdepth;
	   short int x_origin;
	   short int y_origin;
	   short width;
	   short height;
	   char  bitsperpixel;
	   char  imagedescriptor;
	} ;

	struct PIXEL
	{
	   unsigned char r,g,b,a;
	} ;


	void	ConvertColourDepth(PIXEL* pixel,u8* p,u8 bytes);
	u8*		LoadTGA(const char* const pFileName, TGA_HEADER& header);
	u8*		LoadTGA(std::string FileName, TGA_HEADER& TgaHeader);

	void	UnLoadTGA(u8* pData);
	bool	CheckTGAIsSupported(TGA_HEADER& TgaHeader);
	u8*		CreateDataFromTGA(TGA_HEADER& TgaHeader, u8* pTGAData, int x, int y, int w, int h);

	void	ApplyAlphaToTGAFromPink(TGA_HEADER& TgaHeader, PIXEL* pData);
};

#endif
