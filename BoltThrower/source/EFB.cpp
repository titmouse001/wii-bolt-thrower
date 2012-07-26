// Embedded Frame Buffer

#include "EFB.h"
#include "Singleton.h"
#include "WiiManager.h"

void EFB::CopyTextureFromGX(u32* pTexture, u32 x, u32 y, u32 uWidth, u32 uHeight) 
{
	WiiManager& rWii(Singleton<WiiManager>::GetInstanceByRef());
	GXRModeObj* pMode( rWii.GetGXRMode() );

    GX_SetCopyFilter(GX_FALSE, pMode->sample_pattern, GX_FALSE, pMode->vfilter);
    GX_SetTexCopySrc(x, y , uWidth, uHeight);
    GX_SetTexCopyDst(uWidth, uHeight, GX_TF_RGBA8, GX_FALSE);
    GX_CopyTex(pTexture, GX_FALSE);
    GX_PixModeSync();
    DCFlushRange(pTexture, GX_GetTexBufferSize(uWidth, uHeight, GX_TF_RGBA8, GX_FALSE, 0));
	GX_SetCopyFilter(pMode->aa, pMode->sample_pattern, GX_TRUE, pMode->vfilter);

	//return pTexture;
}

Image* EFB::NewTextureFromGX(u32 x, u32 y, u32 uWidth, u32 uHeight) 
{
	Image* pImage = new Image(uWidth,uHeight);
	CopyTextureFromGX(pImage->GetImageData(), x,  y, uWidth, uHeight);
	return pImage;
}