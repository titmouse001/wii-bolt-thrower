#ifndef Render3D_H
#define Render3D_H

#include <string>
#include <vector>
#include <map>
#include "HashLabel.h"
#include "LWOSource/Lwo2.h"
#include "GCTypes.h"
#include "ogc/GU.h"


class PolyXYZ;
class PointXYZ;
class Image;

typedef std::vector<std::string>	VectorOfStrings;
typedef std::vector<PolyXYZ>		VectorOfPolyXYZ;
typedef std::vector<PointXYZ>		VectorOfPointXYZ;

class Object3D
{
public:
	void AddPloy(PolyXYZ& rPoly) { m_PloyContainer.push_back( rPoly ); }
	void AddFullFileName(std::string Name) { m_TextureFullFileNamesContainer.push_back(Name); }
	void AddTexture(Image* pImage) { m_ImageContainer.push_back( pImage ); }  
	//TODO... object only uses first texture 
	Image* GetTexture() { return m_ImageContainer.front(); }
//	Image* GetTexture(int index) { return m_ImageContainer[index]; }

	int GetTotalPolygons() const { return m_PloyContainer.size(); }
	VectorOfStrings::iterator GetFullFileNameBegin()	{ return m_TextureFullFileNamesContainer.begin(); }
	VectorOfStrings::iterator GetFullFileNameEnd()	{ return m_TextureFullFileNamesContainer.end(); }
	bool IsFullFileNameEmpty()	{ return m_TextureFullFileNamesContainer.empty(); }
	VectorOfPolyXYZ::iterator GetPolyBegin()	{ return m_PloyContainer.begin(); }
	VectorOfPolyXYZ::iterator GetPolyEnd()	{ return m_PloyContainer.end(); }
	int GetTotalPointsInObject() const { return m_TotalPointsInObject; }
	void SetTotalPointsInObject(int iValue) { m_TotalPointsInObject = iValue; }
	int CalcTotalPointsInObject();

	const std::string& GetName() const { return m_Name; }
	void SetName(std::string Name) { m_Name = Name; }

	std::vector<Image*>::iterator GetImageDataBegin() { return m_ImageContainer.begin(); }
	std::vector<Image*>::iterator GetImageDataEnd() { return m_ImageContainer.end(); }

	void ClearPloyContainer() { m_PloyContainer.clear(); }
	void ClearTextureFullFileNamesContainer() { m_TextureFullFileNamesContainer.clear(); }

	void SetRadius(float Value) {m_Radius = Value;}
	float GetRadius() const { return m_Radius; }
private:
	// used for creating a display list or if needed a slower coded display
	// the contaners are not needed after we have a list and so can be ereased
	int				m_TotalPointsInObject;
	VectorOfPolyXYZ	m_PloyContainer;
	VectorOfStrings m_TextureFullFileNamesContainer;
	float			m_Radius;

	// used by the display list - keep until program close down
	std::vector<Image*>	m_ImageContainer;	
	std::string m_Name;		
};

class Render3D
{
public:
	Render3D() : CurrentMemoryUsedForDisplayList(0) {;}
	Render3D(std::string FullFileName);
	std::vector<Object3D>::iterator GetObjectBegin()	{ return m_Object3DContainer.begin(); }
	std::vector<Object3D>::iterator GetObjectEnd()	{ return m_Object3DContainer.end(); }
	
//	Object3D*  Add3DObject(std::string FullFileName, bool bSmooth3DObject = true);
	Object3D*  Add3DObject(std::string FullFileName, bool bSmooth3DObject = true, int IndexLayerForBones=-1);

	void SetTextureFromModelName(HashLabel Name, int TextureMap);
	void CreateDisplayList(std::string ModelName);
	void RenderModel(HashLabel, Mtx& ModelView);
	void RenderModelMinimal(HashLabel, Mtx& ModelView);
	void RenderModelPreStage(HashLabel ModelName);
	void AddModel(HashLabel ModelName, void* DispList, int DispListSize, float Radius = 1.0f);
	void RenderModelMinimalHardNorms(HashLabel ModelName, Mtx& ModelView);
	void RenderModelHardNorms(HashLabel ModelName, Mtx& ModelView);

	void DrawModelPoints(std::string ModelName, float Rot, float dist);

	Object3D* GetModel(std::string ModelName);


	float GetDispayListModelRadius(HashLabel ModelName)
	{	
		return m_DispayListContainer[ModelName].m_Radius;
	}


	VectorOfPointXYZ GetModelPointsFromLayer(std::string ModelName);

private:
	void LoadTextures(Object3D& Obj3D);
	Object3D& AddObjectToList(Object3D& rObj3D) { m_Object3DContainer.push_back( rObj3D );  return m_Object3DContainer.back(); }
	std::vector<Object3D> m_Object3DContainer;

	struct DisplayListInfo
	{
		void*	m_dispList;
		u32		m_dispSize;
		float	m_Radius;
	};
	std::map<HashLabel, DisplayListInfo> m_DispayListContainer;
	int CurrentMemoryUsedForDisplayList;
};


class PointXYZ
{
public:
	PointXYZ() {;}
	PointXYZ(float x, float y, float z)			{m_x=x; m_y=y; m_z=z;  }

	void SetColour(unsigned char r, unsigned char g, unsigned char b)	{ m_Red = r; m_Green = g ; m_Blue = b; }
	void SetUV(float u, float v)				{ m_u = u; m_v = v; }
	void SetXYZ(float x, float y, float z)		{ m_x = x; m_y = y; m_z = z; }

	void SetNorms(float nx, float ny, float nz)				{ m_normx = nx; m_normy = ny; m_normz = nz; }
	float GetNormX()	const { return m_normx;}
	float GetNormY()	const { return m_normy;}
	float GetNormZ()	const { return m_normz;}

	float Getx()	const { return m_x;}
	float Gety()	const { return m_y;}
	float Getz()	const { return m_z;}
	float Getu()	const { return m_u; }
	float Getv()	const { return m_v; }
	unsigned char GetRed()		const { return m_Red; }
	unsigned char GetGreen()	const { return m_Green; }
	unsigned char GetBlue()		const { return m_Blue; }

private:

	float m_x,m_y,m_z;
	float m_normx,m_normy,m_normz;
	float m_u,m_v;
	unsigned char /*float*/ m_Red,m_Green,m_Blue;
};


class PolyXYZ
{
public:
	void ClearPoints() { m_PointsContainer.clear(); }
	void AddPoint(PointXYZ& rPointXYZ) { m_PointsContainer.push_back(rPointXYZ); }
	int	 GetTotalPoints() const { return m_PointsContainer.size(); }
	void SetTextureIndex(int Value) { m_TextureIndex = Value; }
	int  GetTextureIndex() const { return m_TextureIndex; }

	VectorOfPointXYZ::iterator GetPointBegin()	{ return m_PointsContainer.begin(); }
	VectorOfPointXYZ::iterator GetPointEnd()		{ return m_PointsContainer.end(); }
	VectorOfPointXYZ::reverse_iterator GetPointRBegin()	{ return m_PointsContainer.rbegin(); }
	VectorOfPointXYZ::reverse_iterator GetPointREnd()		{ return m_PointsContainer.rend(); }

	void RemovePoint(int Index)		{ m_PointsContainer.erase(GetPointBegin() + Index  ); }

private:
	int			m_TextureIndex;
	VectorOfPointXYZ	m_PointsContainer;
};

#endif