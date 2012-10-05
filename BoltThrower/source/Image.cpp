#include "Image.h"

#include "WiiManager.h"
#include "debug.h"
#include "tga.h"
#include "ImageManager.h"

#include <gccore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>  // memset
#include <malloc.h>
#include <dirent.h>
#include "pngu.h"
#include "Util3D.h"
#include "vessel.h"
#include "HashLabel.h"
#include "CharInfo.h"

// need to look... think this is missing free memory stuff???

Image::Image() : m_Width(0) , m_Height(0), m_OriginX(0), m_OriginY(0), m_ImageData(NULL), m_FileNameAsHash("")
{
}

void Image::Initialise(int Width, int Height)
{
	m_Width = Width;
	m_Height = Height;
	m_OriginX = m_Width/2;
	m_OriginY = m_Height/2;
//	m_Red = 0xff;
//	m_Green = 0xff;
//	m_Blue = 0xff;;
}

//void Image::SetColour(u8 r, u8 g, u8, b)
//{
//	m_Red = r;
//	m_Green = g;
//	m_Blue = b;;
//}

Image::Image(string pFileName) : m_FileNameAsHash("")
{
	ImageLoad(pFileName.c_str());
}

Image::Image(u8* pTgaData, int Width, int Height) : m_FileNameAsHash("")
{
	WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() ); 
	ImageManager* pImageManager( Wii.GetImageManager() );

	Initialise(Width,Height);
	m_ImageData = pImageManager->AllocateMemConvertToRGBA8(pTgaData,m_Width,m_Height);
	
	GX_InitTexObj(&m_HardwareTextureInfo, m_ImageData, m_Width,m_Height, GX_TF_RGBA8,GX_CLAMP, GX_CLAMP,GX_FALSE);
}

Image::Image(u32 Width, u32 Height) : m_FileNameAsHash("")
{
	WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() ); 
	ImageManager* pImageManager( Wii.GetImageManager() );

	Tga::PIXEL Col = {255,0,255,255};  // Should never see this, so purple to make it stand out
	Tga::PIXEL* TgaData ( new Tga::PIXEL [Width*Height] ); 
	for (u32 i(0); i<Width*Height; ++i)
	{
		TgaData[i] = Col;
	}

	Initialise(Width,Height);
	m_ImageData = pImageManager->AllocateMemConvertToRGBA8((u8*)TgaData,m_Width,m_Height);

	delete [] TgaData;

	GX_InitTexObj(&m_HardwareTextureInfo, m_ImageData, m_Width,m_Height, GX_TF_RGBA8,GX_CLAMP, GX_CLAMP,GX_FALSE);  

}

Image::Image(CharInfo* pChar, u8* pAlphaData) : m_FileNameAsHash("")
{
	WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() ); 
	ImageManager* pImageManager( Wii.GetImageManager() );

	//m_Width = pChar->m_uWidth;
	//m_Height = pChar->m_uHeight;
	Initialise(pChar->m_uWidth,pChar->m_uHeight);

	Tga::PIXEL* TgaData(NULL);
	if (m_Width==1) // can't have a 1 pixel width - size will be increased.
	{
		u32 OriginalWidth(m_Width);
		m_Width=2;
		TgaData = new Tga::PIXEL [m_Width * m_Height]; 
		memset(TgaData,0,m_Width * m_Height * sizeof(TgaData));
		for (u16 y(0); y<m_Height; ++y)
		{
			for (u16 x(0); x<OriginalWidth; ++x)
			{
				TgaData[x + (m_Width*y)].r = pAlphaData[x + (OriginalWidth*y)];//255;
				TgaData[x + (m_Width*y)].g = pAlphaData[x + (OriginalWidth*y)];//255;
				TgaData[x + (m_Width*y)].b = pAlphaData[x + (OriginalWidth*y)];//255;
				TgaData[x + (m_Width*y)].a = pAlphaData[x + (OriginalWidth*y)];
			}
		}
	}
	else
	{
		TgaData = new Tga::PIXEL [m_Width * m_Height]; 
		for (u16 i(0); i<m_Width * m_Height; ++i)
		{
			TgaData[i].r = pAlphaData[i];//255;
			TgaData[i].g = pAlphaData[i];//255;
			TgaData[i].b = pAlphaData[i];//255;
			TgaData[i].a = pAlphaData[i];
		}
	}

//	m_OriginX = m_Width/2;
//	m_OriginY = m_Height/2;
	m_ImageData = pImageManager->AllocateMemConvertToRGBA8((u8*)TgaData,m_Width,m_Height);
	delete [] TgaData;

	GX_InitTexObj(&m_HardwareTextureInfo, m_ImageData, m_Width,m_Height, GX_TF_RGBA8,GX_CLAMP, GX_CLAMP,GX_FALSE);  
}

void Image::ImageLoad(const char* pFileName)
{
	WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() ); 
	ImageManager* pImageManager( Wii.GetImageManager() );
	pImageManager->BeginGraphicsFile(pFileName);
//	m_Width = pImageManager->GetTgaHeader().width;
//	m_Height = pImageManager->GetTgaHeader().height;
//	m_OriginX = m_Width/2;
//	m_OriginY = m_Height/2;
	Initialise(pImageManager->GetTgaHeader().width,pImageManager->GetTgaHeader().height);

	m_ImageData = pImageManager->AllocateMemConvertToRGBA8(pImageManager->GetTgaData(),m_Width,m_Height);
	GX_InitTexObj(&m_HardwareTextureInfo, m_ImageData, m_Width,m_Height, GX_TF_RGBA8,GX_CLAMP, GX_CLAMP,GX_FALSE);
	pImageManager->EndGraphicsFile();

	SetFileName(pFileName);
}


void Image::DrawImageTL(f32 xpos, f32 ypos ,u8 Alpha )
{	
	GX_LoadTexObj(&m_HardwareTextureInfo, GX_TEXMAP0);   

	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE); 

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc (GX_VA_TEX0,GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

	GX_Begin(GX_QUADS, GX_VTXFMT1,4);
	GX_Position3s16(xpos, ypos,0);	
	GX_Color4u8(0xFF,0xFF,0xFF,Alpha);        
	GX_TexCoord2f32(0, 0);		
	GX_Position3s16(xpos+(m_Width), ypos,0);
	GX_Color4u8(0xFF,0xFF,0xFF,Alpha);            
	GX_TexCoord2f32(1, 0);           
	GX_Position3s16(xpos+(m_Width), ypos+(m_Height),0);         
	GX_Color4u8(0xFF,0xFF,0xFF,Alpha);        
	GX_TexCoord2f32(1, 1);           
	GX_Position3s16(xpos, ypos+(m_Height),0);
	GX_Color4u8(0xFF,0xFF,0xFF,Alpha);              
	GX_TexCoord2f32(0, 1);         
	GX_End();
} 


void Image::DrawImageTL(f32 xpos, f32 ypos , GXColor& Colour) //u8 r, u8 g, u8 b, u8 Alpha )
{	
	GX_LoadTexObj(&m_HardwareTextureInfo, GX_TEXMAP0);   

	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE); 

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc (GX_VA_TEX0,GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

	GX_Begin(GX_QUADS, GX_VTXFMT1,4);
	GX_Position3s16(xpos, ypos,0);	
	GX_Color4u8(Colour.r,Colour.g,Colour.b,Colour.a);        
	GX_TexCoord2f32(0, 0);		
	GX_Position3s16(xpos+(m_Width), ypos,0);
	GX_Color4u8(Colour.r,Colour.g,Colour.b,Colour.a);             
	GX_TexCoord2f32(1, 0);           
	GX_Position3s16(xpos+(m_Width), ypos+(m_Height),0);         
	GX_Color4u8(Colour.r,Colour.g,Colour.b,Colour.a);          
	GX_TexCoord2f32(1, 1);           
	GX_Position3s16(xpos, ypos+(m_Height),0);
	GX_Color4u8(Colour.r,Colour.g,Colour.b,Colour.a);                
	GX_TexCoord2f32(0, 1);         
	GX_End();
} 


void Image::DrawImage(Vessel& item)
{	
	Util3D::TransRot(item.GetX(),item.GetY(),item.GetZ(),item.GetFacingDirection());

	GX_LoadTexObj(&m_HardwareTextureInfo, GX_TEXMAP0);   

	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE); 
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc (GX_VA_TEX0,GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	u8 AlphaValue(item.GetAlpha());
	GX_Begin(GX_QUADS, GX_VTXFMT1,4);	
	GX_Position3s16(-m_OriginX,-m_OriginY,0);	
	GX_Color4u8(0xFF,0xFF,0xFF,AlphaValue);		
	GX_TexCoord2f32(0, 0);			
	GX_Position3s16(m_OriginX, -m_OriginY,0);
	GX_Color4u8(0xFF,0xFF,0xFF,AlphaValue);
	GX_TexCoord2f32(1, 0);           
	GX_Position3s16(m_OriginX, m_OriginY, 0);         
	GX_Color4u8(0xFF,0xFF,0xFF,AlphaValue);
	GX_TexCoord2f32(1, 1);            
	GX_Position3s16(-m_OriginX,m_OriginY,0);
	GX_Color4u8(0xFF,0xFF,0xFF,AlphaValue);    
	GX_TexCoord2f32(0, 1);      
	GX_End(); 
} 

void Image::DrawImage()
{
	DrawImage(255);
}

void Image::DrawImage(u8 AlphaValue)
{	
	GX_LoadTexObj(&m_HardwareTextureInfo, GX_TEXMAP0);   

	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE); 
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc (GX_VA_TEX0,GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

	GX_Begin(GX_QUADS, GX_VTXFMT1,4);	
	GX_Position3s16(-m_OriginX,-m_OriginY,0);	
	GX_Color4u8(0xFF,0xFF,0xFF,AlphaValue);		
	GX_TexCoord2f32(0, 0);			
	GX_Position3s16(m_OriginX, -m_OriginY,0);
	GX_Color4u8(0xFF,0xFF,0xFF,AlphaValue);
	GX_TexCoord2f32(1, 0);           
	GX_Position3s16(m_OriginX, m_OriginY, 0);         
	GX_Color4u8(0xFF,0xFF,0xFF,AlphaValue);
	GX_TexCoord2f32(1, 1);            
	GX_Position3s16(-m_OriginX,m_OriginY,0);
	GX_Color4u8(0xFF,0xFF,0xFF,AlphaValue);    
	GX_TexCoord2f32(0, 1);      
	GX_End(); 
} 

void Image::DrawImageXYZ(f32 xpos, f32 ypos ,f32 z, u8 Alpha, f32 rad, f32 scale )
{
	f32 sx(m_OriginX);
	f32 sy(m_OriginY);	
	m_OriginX*=scale;
	m_OriginY*=scale;	
	DrawImageXYZ(xpos, ypos ,z,Alpha, rad );
	m_OriginX=sx;
	m_OriginY=sy;
}


void Image::DrawImageXYZ(f32 xpos, f32 ypos ,f32 z, u8 Alpha, f32 rad )
{	
	Util3D::TransRot(xpos,ypos,z,rad);
	GX_LoadTexObj(&m_HardwareTextureInfo, GX_TEXMAP0);   

	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE); 
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc (GX_VA_TEX0,GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

	GX_Begin(GX_QUADS, GX_VTXFMT1,4);	
	GX_Position3s16(-m_OriginX,-m_OriginY,0);	
	GX_Color4u8(0xFF,0xFF,0xFF,Alpha);		
	GX_TexCoord2f32(0, 0);			
	GX_Position3s16(m_OriginX, -m_OriginY,0);
	GX_Color4u8(0xFF,0xFF,0xFF,Alpha);
	GX_TexCoord2f32(1, 0);           
	GX_Position3s16(m_OriginX, m_OriginY,0);         
	GX_Color4u8(0xFF,0xFF,0xFF,Alpha);
	GX_TexCoord2f32(1, 1);            
	GX_Position3s16(-m_OriginX, m_OriginY,0);
	GX_Color4u8(0xFF,0xFF,0xFF,Alpha);    
	GX_TexCoord2f32(0, 1);      
	GX_End(); 
} 


void Image::DrawImage(f32 xpos, f32 ypos)
{	
	Util3D::Trans(xpos,ypos);

	GX_LoadTexObj(&m_HardwareTextureInfo, GX_TEXMAP0);   

	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE); 
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc (GX_VA_TEX0,GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

	GX_Begin(GX_QUADS, GX_VTXFMT2,4);	
	GX_Position2s16(-m_OriginX,-m_OriginY);	
	GX_Color1u16(0xFFFF);		
	GX_TexCoord2u16(0, 0);			
	GX_Position2s16(m_OriginX, -m_OriginY);
	GX_Color1u16(0xFFFF);
	GX_TexCoord2u16(1, 0);           
	GX_Position2s16(m_OriginX, m_OriginY);         
	GX_Color1u16(0xFFFF);
	GX_TexCoord2u16(1, 1);            
	GX_Position2s16(-m_OriginX, m_OriginY);
	GX_Color1u16(0xFFFF);    
	GX_TexCoord2u16(0, 1);      
	GX_End();
} 

void Image::SetImageTo4x4RGBA(int x, int y, GXColor color) 
{
	u8 *truc( (u8*)GetImageData() );
	u32 block( (((y >> 2)<<4) * GetWidth() ) + ((x >> 2)<<6) + (((y%4 << 2) + x%4 ) <<1) );

	*(truc+block   )=color.a;
	*(truc+block+1 )=color.r;
	*(truc+block+32)=color.g;
	*(truc+block+33)=color.b;
}

void Image::RemoveImage()
{
	if (m_ImageData!=NULL)
	{
		m_Width=0;
		m_Height=0;
		m_OriginX=0;
		m_OriginY=0;
		memset(&m_HardwareTextureInfo,0,sizeof(m_HardwareTextureInfo));
		free(m_ImageData);  // cleanup from memalign 
		m_ImageData = NULL;
	}
}

void Image::RefreshImage()
{
	u32 bufferSizeBytes = ( ((m_Width+3)/4*4) * ((m_Height+3)/4*4) ) * 4 ; //sizeof(GXColor) ;

	DCFlushRange(m_ImageData,bufferSizeBytes);
	memset(&m_HardwareTextureInfo,0,sizeof(m_HardwareTextureInfo));
	GX_InitTexObj(&m_HardwareTextureInfo, m_ImageData, m_Width,m_Height, GX_TF_RGBA8,GX_CLAMP, GX_CLAMP,GX_FALSE);
	GX_Flush();

	//////////// Test for debugging only, use it to eat more memory
	//////////u32* dataBufferRGBA8 = (u32 *)memalign(32, bufferSizeBytes);
	//////////memcpy(dataBufferRGBA8,m_ImageData,bufferSizeBytes);
	//////////DCFlushRange(dataBufferRGBA8,bufferSizeBytes);
	//////////GX_InitTexObj(&m_HardwareTextureInfo, dataBufferRGBA8, m_Width,m_Height, GX_TF_RGBA8,GX_CLAMP, GX_CLAMP,GX_FALSE);
	//////////GX_Flush();
	//////////m_ImageData  = dataBufferRGBA8;
}
//////
//////void Image::DrawImage2(f32 xpos, f32 ypos ,u8 Alpha )
//////{	
//////	GX_LoadTexObj(&m_HardwareTextureInfo, GX_TEXMAP0);   
//////
//////	GX_SetNumTexGens(1);
//////	GX_SetTevOp(GX_TEVSTAGE0,GX_MODULATE);
//////	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
//////
//////	GX_ClearVtxDesc();
//////	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
//////	GX_SetVtxDesc(GX_VA_NRM, GX_DIRECT);
//////	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
//////	GX_SetVtxDesc (GX_VA_TEX0,GX_DIRECT);
//////
//////
//////	GX_Begin(GX_QUADS, GX_VTXFMT4,4);
//////	GX_Position3f32(xpos, ypos,0);	
//////	GX_Normal3f32(1 ,1,0);
//////	GX_Color4u8(0xFF,0xFF,0xFF,Alpha);		
//////	GX_TexCoord2f32(0, 0);		
//////
//////	GX_Position3f32(xpos+(m_Width), ypos,0);
//////	GX_Normal3f32(1, 0,0);
//////	GX_Color4u8(0x0,0xFF,0,Alpha);		
//////	GX_TexCoord2f32(1, 0);   
//////
//////	GX_Position3f32(xpos+(m_Width), ypos+(m_Height),0);         
//////	GX_Normal3f32(0, 0,1);
//////	GX_Color4u8(0x0,0xFF,0,Alpha);		
//////	GX_TexCoord2f32(1, 1);        
//////
//////	GX_Position3f32(xpos, ypos+(m_Height),0);
//////	GX_Normal3f32(0, 1,0);
//////	GX_Color4u8(0x0,0xFF,0,Alpha);		
//////	GX_TexCoord2f32(0, 1);     
//////
//////	GX_End();
//////} 