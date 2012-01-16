#ifndef Panels3D_H
#define Panels3D_H

#include "GCTypes.h"
#include "ogc/gx.h"
#include <gccore.h>
#include <stdio.h> 
#include <vector>
#include <string>

#include "Singleton.h"
//#include "WiiManager.h"
//#include "GameLogic.h"

using namespace std;

class Panels3D;

class PannelManager
{
public:	
	PannelManager() {;}
	//~PannelManager() {;}

	void Show();
	void Add(std::string Message, float yPos, Panels3D* pPanels3D);
	std::vector<Panels3D*> m_Panels3DContainer;
};



class Panels3D
{
public:	
	Panels3D();
	void DisplayInformationPanels();
	float	m_TiltAction;
	int		m_Count;
	int		LastTotal;
	float	ScrapTilt;
	float	m_yPos;
	string m_Message;

	virtual int  GetValue() { return 0; }

	//virtual ~Panels3D() {}
private:

};

class Panels3DScore: public Panels3D
{
public:	
	int  GetValue();
};

class Panels3DScrap: public Panels3D
{
public:	
	int  GetValue();
};

#endif