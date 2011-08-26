#include "Mission.h"
#include "WiiManager.h"
#include "GameLogic.h"
#include "debug.h"
#include "vessel.h"

MissionManager::MissionManager() : m_CurrentMission(1)
{
}

void MissionManager::AddMissionData(std::string MissionName, std::string MissionObjectiveText, std::string MissionCompletedText)
{
	Mission MissionData(MissionName, MissionObjectiveText, MissionCompletedText);
	m_MissionContainer.push_back( MissionData );
}

Mission& MissionManager::GetMissionData(int Value)
{
	if (Value==0)
		return m_MissionContainer[m_CurrentMission - 1]; // container stars at zero
	else
		return m_MissionContainer[Value - 1];

}

bool MissionManager::IsCurrentMissionObjectiveComplete()
{
	//static int PickUpsNeeded = 0;

	WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() );
	switch (m_CurrentMission)
	{
		case 1:
			return (Wii.GetGameLogic()->IsEnemyDestroyed());
			break;	// yes the break is not needed, but I like it this way
		case 2: 
			return (Wii.GetGameLogic()->IsJustOneShiledSatelliteLeftToSalvaged());
			break;
		case 3: 
			return Wii.GetGameLogic()->IsSalvagedShiledSatellites();
			//PickUpsNeeded = Wii.GetGameLogic()->GetPlrVessel()->GetPickUpTotal();
			break;
		case 4: 
			if (Wii.GetGameLogic()->GetPlrVessel()->GetPickUpTotal() > 85)
				return (true);
			break;
		case 5:
			if (Wii.GetGameLogic()->GetPlrVessel()->GetPickUpTotal() > 155)
				return (true);
			break;
		case 6:
			return (Wii.GetGameLogic()->IsEnemyDestroyed());
			break;
	}

	return false;
}

//=======================================================

Mission::Mission() :  
	m_MissionName("-m-"),m_MissionText("-m-"),m_MissionCompletedText("-m-"),m_Completed(0)
{
}

Mission::Mission(std::string MissionName, std::string Text, std::string MissionCompletedText) :  
	m_MissionName("-m-"),m_MissionText("-m-"),m_MissionCompletedText("-m-"),m_Completed(0)
{
	SetMissionName(MissionName);
	SetMissionText(Text);
	SetMissionCompletedText(MissionCompletedText);
}
