#include "TGA.h"

#include <fat.h>
#include <dirent.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Wiifile.h"

void Tga::UnLoadTGA(u8* pData)
{ 
	delete [] pData; 
}

u8*		Tga::LoadTGA(std::string FileName, TGA_HEADER& TgaHeader)
{
	return LoadTGA(FileName.c_str(), TgaHeader);
}


u8* Tga::LoadTGA(const char* const pFileName, TGA_HEADER& TgaHeader)
{
	FILE* pFile( WiiFile::FileOpenForRead(pFileName) );

	TgaHeader.idlength =		WiiFile::ReadInt8(pFile);
	TgaHeader.colourmaptype =	WiiFile::ReadInt8(pFile);
	TgaHeader.datatypecode =	WiiFile::ReadInt8(pFile);
	TgaHeader.colourmaporigin = WiiFile::ReadInt16(pFile);
	TgaHeader.colourmaplength = WiiFile::ReadInt16(pFile);
	TgaHeader.colourmapdepth=	WiiFile::ReadInt8(pFile);
	TgaHeader.x_origin =		WiiFile::ReadInt16(pFile);
	TgaHeader.y_origin =		WiiFile::ReadInt16(pFile);
	TgaHeader.width =			WiiFile::ReadInt16(pFile);
	TgaHeader.height =			WiiFile::ReadInt16(pFile);
	TgaHeader.bitsperpixel =	WiiFile::ReadInt8(pFile);
	TgaHeader.imagedescriptor =	WiiFile::ReadInt8(pFile);

	//printf("\t%s:%dx%d (x%d)\n",pFileName, TgaHeader.width, TgaHeader.height, TgaHeader.bitsperpixel);

	// allocate memory even when the next stages fail
	PIXEL* pPixelData ( new PIXEL [TgaHeader.width*TgaHeader.height] );
	memset(pPixelData,0,TgaHeader.width*TgaHeader.height*sizeof(PIXEL));

	PIXEL* pWorkingPixelData(pPixelData);
	if (CheckTGAIsSupported(TgaHeader))
	{
		fclose(pFile);
		memset(pPixelData,0xff,TgaHeader.width*TgaHeader.height*sizeof(PIXEL));
		return (u8*)pPixelData;
	}

	// skip any data in the information section
	u32 skipover(TgaHeader.idlength + (TgaHeader.colourmaptype * TgaHeader.colourmaplength));
	fseek(pFile,skipover,SEEK_CUR);

	// allocate memory and read data.
	u32 LoadBufferSize( (TgaHeader.width * TgaHeader.height) * (TgaHeader.bitsperpixel /8) );
	u8* pDataTemp = new u8 [LoadBufferSize]; 

	fread(pDataTemp,sizeof(char),LoadBufferSize,pFile);
	u8* pWorkingTgaPixelDataToDelete(pDataTemp);

	u8 uBytesPerPixel(TgaHeader.bitsperpixel / 8);
	PIXEL* pPixelDataEnd(pPixelData + (TgaHeader.width * TgaHeader.height));

	if (TgaHeader.datatypecode == 2) /* Uncompressed */
	{
		while (pWorkingPixelData < pPixelDataEnd ) 
		{
			ConvertColourDepth(pWorkingPixelData, pDataTemp, uBytesPerPixel);
			pDataTemp+=uBytesPerPixel;
			pWorkingPixelData++;
		}
	}
	else if (TgaHeader.datatypecode == 10)  /* Compressed */
	{
		while (pWorkingPixelData < pPixelDataEnd ) 
		{
			// Why the +1? ... well '7 bit repetition count minus 1'  i.e. 127 is really 128!
			u8 uRun((*pDataTemp & 0x7f) + 1);				// Get length
			u8 bCompressedBodyChunk(*pDataTemp & 0x80);	// Body chunk compressed flag
			
			++pDataTemp;	// The 1 byte header is now followed by a variable-length body.

			if (bCompressedBodyChunk != 0)  /* RLE chunk */
			{ 
				for (u32 i(0); i < uRun; i++) 
				{
					ConvertColourDepth(pWorkingPixelData, pDataTemp, uBytesPerPixel);
					pWorkingPixelData++;
				}
				pDataTemp += uBytesPerPixel;	// Now next chunk is ready to read
			} 
			else  /* Normal chunk */
			{   
				for (u32 i(0); i < uRun; i++) 
				{
					ConvertColourDepth(pWorkingPixelData, pDataTemp, uBytesPerPixel);
					pWorkingPixelData++;
					pDataTemp += uBytesPerPixel;
				}	
			}
		}
	}

	fclose(pFile);

	delete [] pWorkingTgaPixelDataToDelete;

	return (u8*)pPixelData;
}

u8* Tga::CreateDataFromTGA(TGA_HEADER& TgaHeader, u8* pTGAData, int x, int y, int w, int h)
{
	PIXEL* pPixelData ( new PIXEL [w*h] );
	memset(pPixelData,0,w*h*sizeof(PIXEL));

	PIXEL* pData = (PIXEL*)pTGAData;
	pData += x;
	pData += (y*TgaHeader.width);

	for (int iy(0); iy<h; iy++)
	{
		for (int ix(0); ix<w; ix++)
		{
			PIXEL p = pData[ix + (iy*TgaHeader.width)];

			pPixelData[ix + (iy*w)] = p;
		}
	}

	return (u8*)pPixelData;
}

bool Tga::CheckTGAIsSupported(TGA_HEADER& TgaHeader)
{
	bool bError(false);
	if (TgaHeader.datatypecode != 2 && TgaHeader.datatypecode != 10) 
	{
		//printf("image type %d not supported\n",TgaHeader.datatypecode);
		bError = true;
	}
	if (TgaHeader.bitsperpixel != 16 && TgaHeader.bitsperpixel != 24 && TgaHeader.bitsperpixel != 32) 
	{
		//printf("depth %d not supported\n",TgaHeader.bitsperpixel);
		bError = true;
	}
	if (TgaHeader.colourmaptype != 0 && TgaHeader.colourmaptype != 1) 
	{
		//printf("colour map type %d not supported\n",TgaHeader.colourmaptype);
		bError = true;
	}

	return bError;
}

void Tga::ConvertColourDepth(PIXEL* pixel,u8* p,u8 bytes)
{
	static const u32 ALPHA_FULL_ON (255);

	switch (bytes)
	{
		case 2:
			pixel->r = (p[1] & 0x7c) << 1;
			pixel->g = ((p[1] & 0x03) << 6) | ((p[0] & 0xe0) >> 2);
			pixel->b = (p[0] & 0x1f) << 3;
			pixel->a = ALPHA_FULL_ON; //(p[1] & 0x80);
		break;

		case 3:
			pixel->r = p[2];
			pixel->g = p[1];
			pixel->b = p[0];
			pixel->a = ALPHA_FULL_ON;
		break;

		case 4:
			pixel->r = p[2];
			pixel->g = p[1];
			pixel->b = p[0];
			pixel->a = p[3];
		break;

		default:
			pixel->r = 255;
			pixel->g = 255;
			pixel->b = 0;
			pixel->a = ALPHA_FULL_ON;
		break;
	}
}


void Tga::ApplyAlphaToTGAFromPink(TGA_HEADER& TgaHeader, PIXEL* pData)
{
	static const u32 ALPHA_FULL_OFF (0);

	for (int iy(0); iy<TgaHeader.height; iy++)
	{
		for (int ix(0); ix<TgaHeader.width; ix++)
		{
			PIXEL* p = &pData[ix + (iy*TgaHeader.width)];
			if ((p->r == 255) && (p->g==255) && (p->b==255))
			{
				(p->a) = ALPHA_FULL_OFF;
			}
		}
	}
}