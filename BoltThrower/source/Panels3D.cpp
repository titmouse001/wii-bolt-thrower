#include <math.h>
#include <sstream>
#include <iomanip> //std::setw

#include "Panels3D.h"

#include "GameDisplay.h"
#include "GameLogic.h"
#include "Vessel.h"
#include "Mission.h"
#include "Camera.h"
#include "WiiManager.h"
#include "Singleton.h"
#include "debug.h"
#include "hashstring.h"


WiiManager* Panels3D::m_pWii;

void PanelManager::Init()
{
	m_pWii = Singleton<WiiManager>::GetInstanceByPtr();
}


void PanelManager::Add(std::string Message, Panels3D* pPanels3D )
{
	pPanels3D->m_Message = Message;
//	pPanels3D->m_yPos = yPos;
	pPanels3D->m_pWii = m_pWii;  // any way better than this ???
	m_Panels3DContainer.push_back( pPanels3D );  // needed ... add the message here
}

void PanelManager::Show()
{
	int yPos = -190;
	for (vector<Panels3D*>::iterator iter(m_Panels3DContainer.begin()); iter!=m_Panels3DContainer.end(); ++iter) {
		(*iter)->DisplayInformationPanel(yPos);
		yPos+=40;
	}

	
	GameLogic* pGameLogic( m_pWii->GetGameLogic() );

	//------------------------------------

	//
	// summary of baddies left
	//
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );
	float BoxWidth = 46;
	float BoxHeight = 26;
	float x = ( -m_pWii->GetScreenWidth()*0.065f )  + ( m_pWii->GetScreenWidth() / 2);
	float y = ( -m_pWii->GetScreenHeight()*0.075f) + ( m_pWii->GetScreenHeight() / 2) - BoxHeight;

	int size = pGameLogic->GetSporesContainerSize() + pGameLogic->GetEnemySatelliteContainerSize();
	if (size>0)
	{
		x -= BoxWidth;
		m_pWii->TextBoxWithIcon( fCamX + x, fCamY + y, BoxWidth, BoxHeight,
			WiiManager::eRight, HashString::SpinningSpore16x16x9,"%d",size, m_pWii->GetMissionManager()->GetCurrentMission() );
	}
	size = pGameLogic->GetTotalEnemiesContainerSize();
	if (size>0)
	{
		BoxWidth*=1.25f; // this one needs to be 25% bigger (may need to display up to 3 digits)
		x -= BoxWidth;
		m_pWii->TextBoxWithIcon( fCamX + x - 2, fCamY + y, BoxWidth, BoxHeight,
			WiiManager::eRight, HashString::SmallWhiteEnemyShip16x16x2, "%d", size );
	}

	//-------------------------------

	// Players Shield level	
	{
		float fCamX( m_pWii->GetCamera()->GetCamX() );
		float fCamY( m_pWii->GetCamera()->GetCamY() );
		float BoxWidth = 88;
		float BoxHeight = 26;
		float x = ( +m_pWii->GetScreenWidth()*0.065f )  - ( m_pWii->GetScreenWidth() / 2);
		float y = ( -m_pWii->GetScreenHeight()*0.075f) + ( m_pWii->GetScreenHeight() / 2) - BoxHeight;
		//x += BoxWidth;
		m_pWii->TextBoxWithIcon( fCamX + x - 2, fCamY + y, BoxWidth, BoxHeight,
			WiiManager::eRight, HashString::ShieldIcon32x32, "%3d%%", m_pWii->GetGameLogic()->GetPlrVessel()->GetShieldLevel() );
	}


}

//-----

Panels3D::Panels3D() : m_TiltAction(0), m_Count(0), m_LastTotal(0), m_Tilt(- M_PI/2.0), m_DisplayTotal(0.0f) {
}

int  Panels3DScore::GetValue() { 
	return m_pWii->GetGameLogic()->GetScore(); 
}

int  Panels3DScrap::GetValue() { 
	return m_pWii->GetGameLogic()->GetPlrVessel()->GetPickUpTotal(); 
}

void Panels3D::DisplayInformationPanel(int yPos)
{
	int Total = GetValue();

	if (Total > 0) {
		
		std::stringstream ss;
		ss << m_Message << std::setw( 6 ) << std::setfill( '0' ) << (int)m_DisplayTotal;

		 m_pWii->GetGameDisplay()->Display3DInfoBar( 228 , yPos, ss.str(), m_Tilt  );  // origin is centre

		if (m_LastTotal == Total) {
			m_Count++;
			if ((m_Tilt > -0.1f) && (m_Count > 4*60) ) {
				m_TiltAction = -(M_PI/2.0);
			}

			if (m_DisplayTotal > Total-1) {
				m_DisplayTotal = Total;
			} else if (m_DisplayTotal < Total) {
				m_DisplayTotal += ( (float)Total - m_DisplayTotal ) * 0.025f;
			} 
		}
		else {
			m_Count = 0;
			m_TiltAction = 0;
		}
		m_Tilt += ( m_TiltAction - m_Tilt) * 0.045f;
		m_LastTotal = Total;
	}

}