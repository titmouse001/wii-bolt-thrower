#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "WiiManager.h"
#include "Image.h"
#include "debug.h"
#include "util.h"
#include "ImageManager.h"
#include "FontManager.h"
#include "tga.h"
#include "pngu.h"

#include "JPEGDEC.h"
#include <string>

typedef struct s_BM_header {
	WORD BMP_id ; // 'B''M'
	DWORD size; // size in bytes of the BMP file
	DWORD zero_res; // 0
	DWORD offbits; // 54
	DWORD biSize; // 0x28
	DWORD Width;  // X
	DWORD Height;  // Y
	WORD  biPlanes; // 1
	WORD  biBitCount ; // 24
	DWORD biCompression; // 0 = BI_RGB
	DWORD biSizeImage; // 0
	DWORD biXPelsPerMeter; // 0xB40
	DWORD biYPelsPerMeter; // 0xB40
	DWORD biClrUsed; //0
	DWORD biClrImportant; //0
} BM_header;
typedef struct s_RGB {
	BYTE B;
	BYTE G;
	BYTE R;
} RGB;

void write_buf_to_BMP(BYTE* im_buffer, WORD X_bitmap, WORD Y_bitmap, char* BMPname)
{
	FILE* fp_bitmap=fopen(BMPname,"wb");
	if (fp_bitmap==NULL) ExitPrintf("File cannot be created");

	BYTE nr_fillingbytes=0;
	if (X_bitmap%4!=0) 
		nr_fillingbytes=4-((X_bitmap*3L)%4);

	BM_header BH;
	BH.BMP_id = 'M'*256+'B';     WiiFile::WriteInt16(BH.BMP_id,fp_bitmap);
	BH.size=54+Y_bitmap*(X_bitmap*3+nr_fillingbytes);WiiFile::WriteInt32(BH.size,fp_bitmap);
	BH.zero_res = 0;             WiiFile::WriteInt32(BH.zero_res,fp_bitmap);
	BH.offbits = 54;             WiiFile::WriteInt32(BH.offbits,fp_bitmap);
	BH.biSize = 0x28;            WiiFile::WriteInt32(BH.biSize,fp_bitmap);
	BH.Width = X_bitmap;	      WiiFile::WriteInt32(BH.Width,fp_bitmap);
	BH.Height = Y_bitmap;	      WiiFile::WriteInt32(BH.Height,fp_bitmap);
	BH.biPlanes = 1;             WiiFile::WriteInt16(BH.biPlanes,fp_bitmap);
	BH.biBitCount = 24;          WiiFile::WriteInt16(BH.biBitCount,fp_bitmap);
	BH.biCompression = 0;        WiiFile::WriteInt32(BH.biCompression,fp_bitmap);
	BH.biSizeImage = 0;          WiiFile::WriteInt32(BH.biSizeImage,fp_bitmap);
	BH.biXPelsPerMeter = 0xB40;  WiiFile::WriteInt32(BH.biXPelsPerMeter,fp_bitmap);
	BH.biYPelsPerMeter = 0xB40;  WiiFile::WriteInt32(BH.biYPelsPerMeter,fp_bitmap);
	BH.biClrUsed = 0;	          WiiFile::WriteInt32(BH.biClrUsed,fp_bitmap);
	BH.biClrImportant = 0;	      WiiFile::WriteInt32(BH.biClrImportant,fp_bitmap);

	// printf("Writing bitmap ...\n");
	DWORD im_loc_bytes=(DWORD)im_buffer+((DWORD)Y_bitmap-1)*X_bitmap*4;

	for (int y=0;y<Y_bitmap;y++)
	{
		for (int x=0;x<X_bitmap;x++)
		{
			RGB* pixel( (RGB*)im_loc_bytes );
			fwrite(pixel, 3, 1, fp_bitmap);
			im_loc_bytes+=4; 
		}
		for (int i=0;i<nr_fillingbytes;i++)
		{
			BYTE zero_byte=0;
			fwrite(&zero_byte,1,1,fp_bitmap);
		}
		im_loc_bytes-=2L*X_bitmap*4; // jump back two lines, as we move forward 1 line each iteration
	}
	//printf("Done.\n");
	fclose(fp_bitmap);
}

u8* write_buf_to_mem(u32* SrcData, WORD X_bitmap, WORD Y_bitmap)
{
	u8* MemStart = new u8[ (Y_bitmap * X_bitmap*4) ]; 
	//	u8* MemStart( (u8*)malloc(Y_bitmap * X_bitmap*4) );
	u32* pWorking((u32*)MemStart);

	for (SWORD i=0;i<Y_bitmap * X_bitmap ;i++)
	{
		u8* RGB = (u8*)SrcData;
		SrcData++;
		*pWorking = 0xff | RGB[0]<<8 | RGB[1]<<16 | RGB[2]<<24; // reorder colours & Slap on a alpha value
		pWorking++;
	}

	return MemStart;
}


void ImageManager::Init()
{
	m_Wii = Singleton<WiiManager>::GetInstanceByPtr();
}


ImageManager::~ImageManager()
{
	RemoveAllImages();
}

void ImageManager::RemoveAllImages()
{
	for (vector<Image*>::iterator Iter(GetImageDataBegin()); Iter!=GetImageDataEnd(); /* NOP */)
	{
		if ( (*Iter) != NULL )
		{
			(*Iter)->RemoveImage(); // cleanup & remove from list
			Iter = m_ImageContainer.erase( Iter );
		}
		else
		{
			++Iter;
		}
	}
}

void ImageManager::RefreshAllImages()
{
	for (vector<Image*>::iterator Iter(GetImageDataBegin()); Iter!=GetImageDataEnd(); ++Iter)
	{
		if ( (*Iter) != NULL )
		{
			(*Iter)->RefreshImage();
		}
	}
}

Image* ImageManager::GetImage(HashLabel rHashName)
{
//	WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() );
	return GetImage( m_Wii->m_FrameEndStartConstainer[rHashName].StartFrame );
}

Image* ImageManager::GetImage(u32 ImageID)
{
// dev only - should be no need for this check for this kind of thing in final version
//	if ( ImageID < m_ImageContainer.size())
		return  m_ImageContainer[ImageID] ;
//	else
//		return  m_ImageContainer[0] ;
}

void ImageManager::AddImage(string FullName)
{
	m_ImageContainer.push_back( new Image( FullName.c_str() ) );
};

void ImageManager::AddImage(Image* pImage)
{
	m_ImageContainer.push_back( pImage );
};

void ImageManager::AddImage(u32 uWidth, u32 uHeight)
{
	m_ImageContainer.push_back( new Image( uWidth, uHeight ) );
};

void ImageManager::AddImage( u8* pTgaData, u32 uWidth, u32 uHeight)
{
	m_ImageContainer.push_back( new Image( pTgaData, uWidth, uHeight ) );
};

void ImageManager::AddImage(string FullName, u32 CutSizeWidth, u32 CutSizeHeight, u32 uTotal )
{
	ImageManager::AddImage(FullName, 0,0, CutSizeWidth, CutSizeHeight, uTotal );
}

void ImageManager::AddImage(string FullName, u32 StartX, u32 StartY, u32 CutSizeWidth, u32 CutSizeHeight, u32 uTotal )
{
	BeginGraphicsFile(FullName.c_str());

	AddImage(StartX, StartY,CutSizeWidth, CutSizeHeight, uTotal );

	EndGraphicsFile();
};

int ImageManager::AddImage( u32 StartX, u32 StartY, u32 CutSizeWidth, u32 CutSizeHeight, u32 uTotal, EDirection Cutdirection )
{
	// just a missing data thing.... this is a temp work arround for missing alpha data in texuters
	//	Tga::ApplyAlphaToTGAFromPink(TgaHeader, (Tga::PIXEL*) TgaData);

	int StoreCount = GetImageCount();

	if (CutSizeWidth==0)
	{
		CutSizeWidth = m_TgaHeader.width;
	}
	
	if (CutSizeHeight==0)
	{
		CutSizeHeight = m_TgaHeader.height;
	}


	int uCount(uTotal);
	if (uTotal==0)
	{
		uCount = (m_TgaHeader.width / CutSizeWidth) * (m_TgaHeader.height / CutSizeHeight);
	}

	if (Cutdirection == eRight)  // scan right
	{
		for (u32 y(StartY); y<(u32)m_TgaHeader.height; y+=CutSizeHeight)
		{
			for (u32 x(StartX); x<(u32)m_TgaHeader.width; x+=CutSizeWidth )
			{
				u8* Data( Tga::CreateDataFromTGA(m_TgaHeader, m_pTgaData, x, y, CutSizeWidth, CutSizeHeight) );
				AddImage(Data,CutSizeWidth,CutSizeHeight);
				delete [] Data;  

				--uCount;

				if (uCount<=0)
					return StoreCount;	
			}
		}
	}
	else  // scan down
	{
		for (u32 x(StartX); x<(u32)m_TgaHeader.width; x+=CutSizeWidth )
		{
			for (u32 y(StartY); y<(u32)m_TgaHeader.height; y+=CutSizeHeight)
			{
				u8* Data( Tga::CreateDataFromTGA(m_TgaHeader, m_pTgaData, x, y, CutSizeWidth, CutSizeHeight) );
				AddImage(Data,CutSizeWidth,CutSizeHeight);
				delete [] Data;  

				--uCount;

				if (uCount<=0)
					return StoreCount;	
			}
		}
	}
	return StoreCount;
};


bool ImageManager::BeginGraphicsFile(string FullFileName)
{
	Util::StringToLower(FullFileName);
	if (FullFileName.substr(FullFileName.find_last_of(".") + 1) == "png")
	{		
		IMGCTX ctx( PNGU_SelectImageFromDevice (FullFileName.c_str()) ); 
		PNGUPROP imgProp;
		s32 Result( PNGU_GetImageProperties (ctx, &imgProp) );

		if (Result != PNGU_OK)
			ExitPrintf("PNGU_SelectImageFromDevice failed to load '%s'", FullFileName.c_str() );
	//	else
	//		printf("loading... '%s'\n", FullFileName.c_str());

		u32 bufferSizeBytes = ( ((imgProp.imgWidth+3)/4*4) * ((imgProp.imgHeight+3)/4*4) ) * 4 ; 

		m_pTgaData = new u8 [bufferSizeBytes]; 

		m_TgaHeader.height = imgProp.imgHeight;
		m_TgaHeader.width = imgProp.imgWidth;

		PNGU_DecodeToRGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, m_pTgaData,0, 255);

		PNGU_ReleaseImageContext (ctx);

	}
	else if (FullFileName.substr(FullFileName.find_last_of(".") + 1) == "tga")
	{
		m_pTgaData = Tga::LoadTGA(FullFileName.c_str(), m_TgaHeader) ; 
	}
	else if (FullFileName.substr(FullFileName.find_last_of(".") + 1) == "jpg")
	{
		FILE* pFile( WiiFile::FileOpenForRead(FullFileName.c_str()) );

		uint x,y;
		load_JPEG_header(pFile, &x, &y);
		m_TgaHeader.width = x;
		m_TgaHeader.height = y;

		decode_JPEG_image();

		BYTE *image_buffer=NULL;
		get_JPEG_buffer(x,y,&image_buffer);

		m_pTgaData = write_buf_to_mem((u32*)image_buffer,x,y);
	}
	else
	{
		ExitPrintf("Unkown file type for '%s'", FullFileName.c_str() );
	}

	return (m_pTgaData!=NULL);
}

void ImageManager::EndGraphicsFile()
{
	if (m_pTgaData != NULL)
	{
		Tga::UnLoadTGA( m_pTgaData );
	}
}




//I4 (4 bit intensity, 8x8 tiles) 
//I8 (8 bit intensity, 8x4 tiles) 
//IA4 (4 bit intensity with 4 bit alpha, 8x4 tiles) 
//IA8 (8 bit intensity with 8 bit alpha, 4x4 tiles) 
//RGB565 (4x4 tiles) 
//RGB5A3 (4x4 tiles) 
//RGBA8 (4x4 tiles in two cache lines - first is AR and second is GB) 
//CI4 (4 bit color index, 8x8 tiles) 
//CI8 (8 bit color index, 8x4 tiles) 
//CI14X2 (14 bit color index, 4x4 tiles) 
//CMP (S3TC compressed, 2x2 blocks of 4x4 tiles) 

// convertion to RGBA8 (4x4 tiles in two cache lines - first is AR and second is GB)
u32* ImageManager::AllocateMemConvertToRGBA8(u8* src, u32 bufferWidth, u32 bufferHeight) 
{
	// changed the way size allocation works, should now convert any size of image safely.  
	u32 bufferSizeBytes = ( ((bufferWidth+3)/4*4) * ((bufferHeight+3)/4*4) ) * 4 ; //sizeof(GXColor) ;

	u32* dataBufferRGBA8 = (u32 *)memalign(32, bufferSizeBytes);
	memset(dataBufferRGBA8, 0, bufferSizeBytes);

	u8 *dst((u8*)dataBufferRGBA8);
	for (u16 block(0); block < bufferHeight; block += 4) 
	{  
		for (u16 i(0); i < bufferWidth; i += 4) 
		{  
			for (u16 c(0); c < 4; ++c) 
			{  
				for (u16 AlphaAndRed(0); AlphaAndRed < 4; ++AlphaAndRed) 
				{  
					*dst++ = src[(((i + AlphaAndRed) + ((block + c) * bufferWidth)) * 4) + 3];	//Alpha 
					*dst++ = src[((i + AlphaAndRed) + ((block + c) * bufferWidth)) * 4];		//red
				}  
			}  
			for (u16 c(0); c < 4 ; ++c) 
			{  
				for (u16 GreenAndBluePixels(0); GreenAndBluePixels < 4; ++GreenAndBluePixels) 
				{  
					*dst++ = src[(((i + GreenAndBluePixels) + ((block + c) * bufferWidth)) * 4) + 1];  //green
					*dst++ = src[(((i + GreenAndBluePixels) + ((block + c) * bufferWidth)) * 4) + 2];  //blue
				}  
			}  
		}
	} 

	DCFlushRange(dataBufferRGBA8, bufferSizeBytes);
	return dataBufferRGBA8;
}