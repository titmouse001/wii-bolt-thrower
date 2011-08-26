#ifndef MapManager_H_
#define MapManager_H_

#include <gccore.h>
#include <string>
#include <map>
#include <vector>
#include "WiiFile.h"

using namespace std;

typedef u16	MapSize;

enum EMapType { eMapIsSolidLayer, eMapIsTransparentLayer, eMapIsInternal, eMapIsHit, eLayerBrush,eMapNone } ;


class Map
{
public:
	MapSize*	ReadMapBody(FILE* pFile);
	u32		GetTotalWidth() const		{ return m_TotalWidth; }
	u32		GetTotalHeight() const		{ return m_TotalHeight; }

	f32		GetViewXpos()	const		{ return m_ViewXpos; }
	f32		GetViewYpos()	const		{ return m_ViewYpos; }
//	void	SetViewXpos(s32 Value);		
//	void	SetViewYpos(s32 Value);	

	void	SetView(f32 x, f32 y);		

	void	SetTotalWidth(u32 Value)	{ m_TotalWidth = Value; m_TotalPixelWidth=Value*16; }
	void	SetTotalHeight(u32 Value)	{ m_TotalHeight = Value; m_TotalPixelHeight=Value*16; }
	void	SetImageWidth(u32 Value)	{ m_ImageWidth = Value; }
	void	SetImageHeight(u32 Value)	{ m_ImageHeight = Value; }
//	MapSize		GetValue(u32 x, u32 y) const;
	MapSize		GetValueByIndex(u32 Index) const;
	MapSize		GetValueFromOrigin(u32 x, u32 y) const;
	void	SetValueFromOrigin(u32 x, u32 y, MapSize Value);

	void	ValidateView(u32 ScreenWidth, u32 ScreenHeight);


	u32 GetTotalPixelWidth()  { return m_TotalPixelWidth; }
	u32 GetTotalPixelHeight() { return m_TotalPixelHeight; }


	void SetMapType(EMapType Value) { m_eMapType = Value; }
	EMapType GetMapType() const { return m_eMapType; }


	const char* const GetMapName() { return m_MapName.c_str(); }
	void SetMapName(string Name) { m_MapName = Name; }

private:
	u32 m_TotalPixelWidth;
	u32 m_TotalPixelHeight;
	u32	m_TotalWidth;
	u32	m_TotalHeight;

	f32 m_ViewXpos;
	f32 m_ViewYpos;
//	u32 m_ViewXpos;
//	u32 m_ViewYpos;

	u32 m_ImageWidth;			// maybe loose this from the format .. not really needed
	u32 m_ImageHeight;
	MapSize* m_MapData;
	EMapType	m_eMapType;
	string	m_MapName;

};


class MapLayer
{
public:
	void	GetMap(string FileName);
	vector<Map*>::iterator	GetBegin() { return m_MapLayersContainer.begin(); }
	vector<Map*>::iterator	GetEnd()	{ return m_MapLayersContainer.end(); }


	Map* GetSolidLayer() 
	{
		for (vector<Map*>::iterator Iter(GetBegin()); Iter != GetEnd(); ++Iter)
		{
			if ((*Iter)->GetMapType() == eMapIsSolidLayer)
				return *Iter;
		}
		return NULL;
	}


	Map* GetTransparentLayer() 
	{
		for (vector<Map*>::iterator Iter(GetBegin()); Iter != GetEnd(); ++Iter)
		{
			if ((*Iter)->GetMapType() == eMapIsTransparentLayer)
				return *Iter;
		}
		return NULL;
	}

	Map* GetHitMap() 
	{
		for (vector<Map*>::iterator Iter(GetBegin()); Iter != GetEnd(); ++Iter)
		{
			if ((*Iter)->GetMapType() == eMapIsHit)
				return *Iter;
		}
//		ExitPrintf("GetHitMap is null");
		return NULL;
	}

private:
	vector<Map*> m_MapLayersContainer;
};


class MapManager 
{
public:
	MapLayer* GetLayerContainer(string Name);
	void GetMap(string FileName);

	void SetCurrentWorkingMapLayer(string Name);
	MapLayer* GetCurrentWorkingMapLayer() const  { return m_pCurrentWorkingMapLayer;}

private:
	std::map<string, MapLayer*> m_MapManagerContainer;
	MapLayer* m_pCurrentWorkingMapLayer;
};


#endif
