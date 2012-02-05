#include "InputDeviceManager.h"
#include <stdio.h>
#include <math.h>
#include "debug.h"



void DeviceNunChuck::GetJoyValuesFromCentre(expansion_t* pData, float& rX, float& rY)
{
rX = pData->nunchuk.js.pos.x - pData->nunchuk.js.min.x;
rY = pData->nunchuk.js.pos.y - pData->nunchuk.js.min.y;
rX -= (pData->nunchuk.js.max.x - pData->nunchuk.js.min.x) / 2;
rY -= (pData->nunchuk.js.max.y - pData->nunchuk.js.min.y) / 2;
//------------------------------------------------------------------------------------
static float DriftX(0);
if ( (fabs(rX) - 100.0f) > DriftX)
{
	if (rX<0)
		DriftX = (-rX) - 100.0f;
	else
		DriftX = 100.0f - rX;
}
//------------------------------------------------------------------------------------
static float DriftY(0);
if ( (fabs(rY) - 100.0f) > DriftY)
{
	if (rY<0)
		DriftY = (-rY) - 100.0f;
	else
		DriftY = 100.0f - rY;
}
//------------------------------------------------------------------------------------
// just incase a new nunchuck is swapped in (new levels need setting)
// should only happen when new h/w is swapped in during play
if (fabs(rX + DriftX) > 100.0f)
	DriftX = 0;  // reset since swapping in a new nunchuck will need re-offsetting 
if (fabs(rY + DriftY) > 100.0f)
	DriftY = 0;	// again... needs re-offsetting 
//------------------------------------------------------------------------------------
rX += DriftX;	// Corrects distance between opposite directions
rY += DriftX;
rX *= 0.01;		// scale -1 to +1
rY *= 0.01;

if (fabs(rX) < 0.15f) rX = 0;
if (fabs(rY) < 0.15f) rY = 0;

printf("\x1b[11;25HNCUHCK mx %1.4f my %1.4f  ", rX, rY);


}

void DeviceNunChuck::SetJoyValues(expansion_t* pData)
{
	GetJoyValuesFromCentre( pData, m_NunChuck_X, m_NunChuck_Y );
}

//-----------------------------------------------------

InputDeviceManager::InputDeviceManager( int Value )
{ 
	WPAD_Init();
	WPAD_SetIdleTimeout(Value); 

	m_pNunChuck = new DeviceNunChuck;
}

InputDeviceManager::~InputDeviceManager()
{ 
	delete m_pNunChuck;

	WPAD_Flush(0); 
	WPAD_Disconnect(0);  
	WPAD_Shutdown(); 

}
bool InputDeviceManager::IsWiiMoteReady(s32 Chan)
{
	u32 Status;
	return (WPAD_Probe(Chan, &Status) != WPAD_ERR_NONE);
}

void InputDeviceManager::Store()
{
	//	WPAD_ScanPads();
	//TEMP just 1 InputDeviceManager for now
	//	m_PadButtonHeldState = WPAD_ButtonsHeld(0);
	//   // m_PadButtonHeldState = WPAD_ButtonsDown(0);

	for (s32 PadCount(0); PadCount<1; ++PadCount)
	{
		WPADData* pPadData(WPAD_Data(PadCount));	// top level structure

		expansion_t* pData = &pPadData->exp;		// expansion devices like nunchuck
		m_pNunChuck->SetEnable(false);
		switch (pData->type)
		{
		case WPAD_EXP_NUNCHUK:
			m_pNunChuck->SetJoyValues(pData);
			m_pNunChuck->SetEnable(true);
			break;
		}

		if (pPadData->ir.valid) 
		{	
			Vtx VertexFor2D;

			VertexFor2D.x = pPadData->ir.x-50;  // fudge ... FinaliseInputDevices uses +100
			VertexFor2D.y = pPadData->ir.y-50;
			VertexFor2D.z = pPadData->ir.z;

			if (ControllContainer[PadCount].size()>=10)
			{
				ControllContainer[PadCount].erase(ControllContainer[PadCount].begin());
			}

			ControllContainer[PadCount].push_back(VertexFor2D);
		}
	}
}

Vtx* InputDeviceManager::GetIRPosition()
{
	u32 IRPadNumber(0);
	if (ControllContainer[IRPadNumber].empty())
	{
		return NULL;
	}
	else
	{
		return &ControllContainer[IRPadNumber].back(); // take last one - most up to date coords
	}
}

//
//std::vector<Vtx> InputDeviceManager::GetIRPositionContainer()
//{
//	u32 IRPadNumber(0);
//	return ControllContainer[IRPadNumber];
//}
