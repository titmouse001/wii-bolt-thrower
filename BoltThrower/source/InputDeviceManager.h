#ifndef InputDeviceManager_H_
#define InputDeviceManager_H_

#include <gccore.h>
#include <vector>
#include <wiiuse/wpad.h>


class InputDeviceManager
{
public:

	InputDeviceManager( int Value = 60*5 );
	~InputDeviceManager();

	void Store();

	Vtx* GetIRPosition();
	std::vector<Vtx> GetIRPositionContainer();
//	u32 GetPadButton() const { return m_PadButtonHeldState; }

	bool IsWiiMoteReady(s32 Chan)
	{
		u32 Status;
		return (WPAD_Probe(Chan, &Status) != WPAD_ERR_NONE);
	}

private:

//	u32 m_PadButtonHeldState;
	std::vector<Vtx>	ControllContainer[WPAD_MAX_WIIMOTES];
};


#endif