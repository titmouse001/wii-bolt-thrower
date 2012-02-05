#ifndef InputDeviceManager_H_
#define InputDeviceManager_H_

#include <gccore.h>
#include <vector>
#include <wiiuse/wpad.h>

class DeviceNunChuck;

class InputDeviceManager
{
public:

	InputDeviceManager( int Value = 60*5 );
	~InputDeviceManager();

	void Store();

	Vtx* GetIRPosition();
	std::vector<Vtx> GetIRPositionContainer();
	bool IsWiiMoteReady(s32 Chan);

	DeviceNunChuck* GetNunChuck() { return m_pNunChuck; }
	
private:
	DeviceNunChuck*		m_pNunChuck;
	std::vector<Vtx>	ControllContainer[WPAD_MAX_WIIMOTES];
};


class DeviceNunChuck
{
public:

	float GetJoyX() const { return m_NunChuck_X; }
	float GetJoyY() const { return m_NunChuck_Y; }
	void SetJoyValues(expansion_t* pData);

	void SetEnable(bool bState ) { m_Enabled = bState; }
	bool GetEnable() const { return  m_Enabled; }

private:
	
	void GetJoyValuesFromCentre(expansion_t* pData, float& rX, float& rY);

	float	m_NunChuck_X;
	float	m_NunChuck_Y;

	bool	m_Enabled;

};



#endif