#ifndef _EFB_H
#define _EFB_H

#include "GCTypes.h"
#include "Image.h"

namespace EFB
{
	void	CopyTextureFromGX(u32* pTexture, u32 x, u32 y, u32 uWidth, u32 uHeight);
	Image*	NewTextureFromGX(u32 x, u32 y, u32 uWidth, u32 uHeight);
}


#endif