#ifndef Panels3D_H
#define Panels3D_H

#include "GCTypes.h"
#include "ogc/gx.h"
#include <gccore.h>
#include <stdio.h> 
#include <vector>
#include <string>

#include "Singleton.h"

using namespace std;

class Panels3D;
class WiiManager;


class PanelManager
{
public:	
	//PanelManager() {;}
	void Init();

	void Show();
	void Add(std::string Message, Panels3D* pPanels3D);
	std::vector<Panels3D*> m_Panels3DContainer;

	WiiManager* m_pWii;
};



class Panels3D
{
public:	
	Panels3D();
	void DisplayInformationPanel(int yPos);
	float	m_TiltAction;
	int		m_Count;
	int		m_LastTotal;
	float	m_Tilt;
//	float	m_yPos;
	string m_Message;

	float		m_DisplayTotal;

	static WiiManager* m_pWii;

	virtual int  GetValue() { return 0; }

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