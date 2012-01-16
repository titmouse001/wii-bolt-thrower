#include <math.h>
#include <sstream>
#include <iomanip> //std::setw

#include "Panels3D.h"

#include "GameDisplay.h"
#include "GameLogic.h"
#include "Vessel.h"
#include "Mission.h"

#include "WiiManager.h"
#include "Singleton.h"
#include "debug.h"

void PannelManager::Add(std::string Message, float yPos, Panels3D* pPanels3D )
{
//	Panels3DScore* pPanels3D( new Panels3DScore );

//	Panels3DScore* pPanels3D( pPanels3D );
	pPanels3D->m_Message = Message;
	pPanels3D->m_yPos = yPos;
	m_Panels3DContainer.push_back( pPanels3D );  // needed ... add the message here
}

void PannelManager::Show()
{
	for (vector<Panels3D*>::iterator iter(m_Panels3DContainer.begin()); iter!=m_Panels3DContainer.end(); ++iter)
	{
	//	Panels3DScore* pPanels3D = static_cast<Panels3DScore*>(&(*iter));

		(*iter)->DisplayInformationPanels();
	}
}
//-----

Panels3D::Panels3D() : m_TiltAction(0), m_Count(0), LastTotal(0), ScrapTilt(- M_PI/2.0)
{
}

int  Panels3DScore::GetValue()
{ 
	//return 0;
	WiiManager* pWii = Singleton<WiiManager>::GetInstanceByPtr();
	return pWii->GetGameLogic()->GetScore(); 
}

int  Panels3DScrap::GetValue()
{ 
	//return 0;
	WiiManager* pWii = Singleton<WiiManager>::GetInstanceByPtr();
	return pWii->GetGameLogic()->GetPlrVessel()->GetPickUpTotal(); 
}

void Panels3D::DisplayInformationPanels()
{
	WiiManager* pWii = Singleton<WiiManager>::GetInstanceByPtr();
	GameDisplay* pDisplay( pWii->GetGameDisplay() );

	int Total = GetValue();
	if (Total > 0)
	{
		std::stringstream ss;
		ss << m_Message << std::setw( 6 ) << std::setfill( '0' ) << Total;
		pDisplay->Display3DInfoBar( 228 , m_yPos, ss.str(),ScrapTilt  );  // origin is centre

		if (LastTotal == Total)
		{
			m_Count++;
			if ((ScrapTilt > -0.1f) && (m_Count > 4*60) )
			{
				m_TiltAction = -(M_PI/2.0);
			}
		}
		else
		{
			m_Count = 0;
			m_TiltAction = 0;
		}
		ScrapTilt += ( m_TiltAction - ScrapTilt) * 0.045f;
		LastTotal = Total;
	}


	//////// summary of baddies left
	//////float fCamX( m_pWii->GetCamera()->GetCamX() );
	//////float fCamY( m_pWii->GetCamera()->GetCamY() );
	//////float BoxWidth = 46;
	//////float BoxHeight = 26;
	//////float x = (  -m_pWii->GetScreenWidth()*0.065f )  + ( m_pWii->GetScreenWidth() / 2) - BoxWidth;
	//////float y = ( -m_pWii->GetScreenHeight()*0.075f) + ( m_pWii->GetScreenHeight() / 2) - BoxHeight;

	//////int size = m_pGameLogic->GetSporesContainerSize();
	//////if (size>0)
	//////{
	//////	m_pWii->TextBoxWithIcon( fCamX + x, fCamY + y, BoxWidth, BoxHeight,
	//////		WiiManager::eRight, HashString::SpinningSpore16x16x9,"%02d",size, m_pWii->GetMissionManager()->GetCurrentMission() );
	//////}
	//////size = m_pGameLogic->GetTotalEnemiesContainerSize();
	//////if (size>0)
	//////{
	//////	m_pWii->TextBoxWithIcon( fCamX + x - BoxWidth -2, fCamY + y, BoxWidth, BoxHeight,
	//////		WiiManager::eRight, HashString::SmallWhiteEnemyShip16x16x2, "%02d", size );
	//////}

}