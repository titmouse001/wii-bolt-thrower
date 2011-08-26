#include <stdlib.h>

#include "MapManager.h"
#include "WiiManager.h"
#include "debug.h"


void	Map::SetView(f32 x, f32 y)
{
	m_ViewXpos = x;
	m_ViewYpos = y;
}


//void	Map::SetViewXpos(s32 Value)
//{
//	WiiManager& Wii = Singleton<WiiManager>::GetInstanceByRef();
//
//	// code is a bit nasty as here it needs to know about screen values
//	//maybe add a 'CurrentViewableMapSsize' or something to make it less complex
//	if (Value<0)
//		m_ViewXpos = 0; 
//	else if ( Value > ( (int) (m_TotalPixelWidth) - (int) Wii.GetScreenWidth() ) )
//		m_ViewXpos = m_TotalPixelWidth - Wii.GetScreenWidth();
//	else
//		m_ViewXpos = Value;
//}
//
//void	Map::SetViewYpos(s32 Value)
//{ 
//	WiiManager& Wii = Singleton<WiiManager>::GetInstanceByRef();
//
//	if (Value<0)
//		m_ViewYpos = 0; 
//	else if ( Value > ((int) (m_TotalPixelHeight) - ((int)Wii.GetScreenHeight() )) )
//		m_ViewYpos = m_TotalPixelHeight - Wii.GetScreenHeight();
//	else
//		m_ViewYpos = Value;
//}

///// sort this lot out later....
//MapSize Map::GetValue(u32 x, u32 y) const
//{ 
//	x = ( x + m_ViewXpos ) / 16;
//	y = ( y + m_ViewYpos ) / 16;
//
//	u32 Index(x + ( y * GetTotalWidth() ));
//
//	return m_MapData[Index]; 
//
MapSize Map::GetValueFromOrigin(u32 x, u32 y) const
{ 
	x = ( x  ) / 16;
	y = ( y  ) / 16;

	u32 Index(x + ( y * GetTotalWidth() ));

	return m_MapData[Index]; 
}

MapSize Map::GetValueByIndex(u32 Index) const
{ 
	return m_MapData[Index]; 
}

void Map::SetValueFromOrigin(u32 x, u32 y, MapSize Value)
{ 
	x = x / 16;
	y = y / 16;

	u32 Index(x + ( y * GetTotalWidth() ));

	m_MapData[Index] = Value; 
}

MapSize* Map::ReadMapBody(FILE* pFile)
{
	// -----------------------
	m_MapName = WiiFile::ReadString(pFile);
	//-----------------------

	SetTotalWidth(WiiFile::ReadInt32(pFile) );
	SetTotalHeight(WiiFile::ReadInt32(pFile) );
	SetView(WiiFile::ReadInt32(pFile), WiiFile::ReadInt32(pFile));// camera top left starting point
	SetImageWidth( WiiFile::ReadInt32(pFile) );
	SetImageHeight( WiiFile::ReadInt32(pFile) );

	u32 uSize(GetTotalWidth() * GetTotalHeight());
	m_MapData = new MapSize[uSize];
	for (u32 k(0); k<uSize; k++)
	{
		m_MapData[k] = WiiFile::ReadInt16(pFile);
	}
	return m_MapData;
}


void  MapLayer::GetMap(string FileName)
{
	
	FILE* pFile(WiiFile::FileOpenForRead(FileName.c_str()));

	string Name;
	Name.push_back(WiiFile::ReadInt8(pFile));
	Name.push_back(WiiFile::ReadInt8(pFile));
	Name.push_back(WiiFile::ReadInt8(pFile));
	Name.push_back(WiiFile::ReadInt8(pFile));
	// check that we really are loading the correct file type
	if (Name=="Map1") 
	{
		u32	uLayersFound( WiiFile::ReadInt32( pFile ) );
		for (u32 uLayerCount(0); uLayerCount<uLayersFound; ++uLayerCount)
		{
		
			Map* pMap = new Map;
			pMap->ReadMapBody(pFile);

			if (uLayerCount==0)
				pMap->SetMapType(eMapIsSolidLayer);
			else if (uLayerCount==1)
				pMap->SetMapType(eMapIsHit);
			else
				pMap->SetMapType(eMapIsTransparentLayer);

			// Store each map layer
			m_MapLayersContainer.push_back(pMap);
		}
	}
	fclose (pFile);
}


MapLayer* MapManager::GetLayerContainer(string Name) 
{ 
	std::map<string, MapLayer*>::iterator iter( m_MapManagerContainer.find(Name) ); // find returns the end of the map if nothing found

	if (iter==m_MapManagerContainer.end())
	{
		ExitPrintf("GetLayerContainer(%s) not found",Name.c_str());
		return NULL;
	}
	else
		return iter->second; 
}

void MapManager::GetMap(string FileName)
{
	MapLayer* pData = new MapLayer;
	pData->GetMap(FileName);
	m_MapManagerContainer.insert( pair<string, MapLayer*>(FileName, pData) );
}

void MapManager::SetCurrentWorkingMapLayer(string Name)
{ 
	m_pCurrentWorkingMapLayer = GetLayerContainer(Name);
}
