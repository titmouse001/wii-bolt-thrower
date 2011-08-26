#ifndef SpriteManager_H_
#define SpriteManager_H_

#include "GCTypes.h"
#include "Image.h"
#include <string>
#include "imageManager.h"

class SpriteManager : public ImageManager
{
public:

	virtual void AddImage(u8* pTgaData, u32 uWidth, u32 uHeight);

private:

};

#endif
