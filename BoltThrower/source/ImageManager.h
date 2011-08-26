#ifndef ImageManager_H_
#define ImageManager_H_

#include "GCTypes.h"
#include "ogc/gx.h"
#include "Tga.h"
#include "HashLabel.h"
#include <string>
#include <vector>

using namespace std;

class Image;


class ImageManager
{
public:

	enum EDirection { eDown,eRight };


	ImageManager() : m_pTgaData(NULL) { ; }
	~ImageManager();

	Image* GetImage(u32 ImageID);
	Image* GetImage(HashLabel rHashName);

	void AddImage(string FullName);
	void AddImage(u32 uWidth, u32 uHeight);
	void AddImage(Image* pImage);
	virtual void AddImage(u8* pTgaData, u32 Width , u32 Height);
	
	void AddImage(string FullName, u32 CutSizeWidth, u32 CutSizeHeight, u32 uTotal = 0);
	void AddImage(string FullName, u32 StartX, u32 StartY, u32 CutSizeWidth, u32 CutSizeHeight, u32 uTotal = 0 );
	int AddImage(u32 StartX, u32 StartY, u32 CutSizeWidth, u32 CutSizeHeight, u32 uTotal = 0 , EDirection Cutdirection = eRight);

	std::vector<Image*>::iterator GetImageDataBegin() { return m_ImageContainer.begin(); }
	std::vector<Image*>::iterator GetImageDataEnd() { return m_ImageContainer.end(); }
	int GetImageCount() const { return m_ImageContainer.size(); }

	void RemoveAllImages();
	void RefreshAllImages();

	bool BeginGraphicsFile(string pFullFileName);
	void EndGraphicsFile();

	Tga::TGA_HEADER& GetTgaHeader() { return  m_TgaHeader; }
	u8*				 GetTgaData() { return m_pTgaData; }

	u32* AllocateMemConvertToRGBA8(u8* src, u32 bufferWidth, u32 bufferHeight) ;

private:

	vector<Image*> m_ImageContainer;

	Tga::TGA_HEADER m_TgaHeader;
	u8*				m_pTgaData;
};

#endif