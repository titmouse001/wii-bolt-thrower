#include "ImageManager.h"
#include "SpriteManager.h"
#include "Image.h"
#include "Tga.h"

#include "Debug.h"

void SpriteManager::AddImage(u8* pTgaData, u32 uWidth, u32 uHeight)
{
	// mask the colour purple by setting alpha to transparent
	Tga::PIXEL* pData((Tga::PIXEL*)pTgaData);
	for (u32 Index(0); Index < uWidth * uHeight; ++Index )
	{
		if ((pData[Index].r==255) && (pData[Index].g==0) && (pData[Index].b==255))
		{
			pData[Index].a = 0;
		}		
	}
	ImageManager::AddImage( pTgaData, uWidth, uHeight );
};
