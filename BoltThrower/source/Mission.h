#ifndef Mission_H_
#define Mission_H_

#include <string>
#include <vector>
#include "GCTypes.h"
#include "debug.h"
class Mission;

class MissionManager
{
	public:

		MissionManager();

		void AddMissionData(std::string m_MissionName,std::string MissionObjectiveText, std::string MissionCompletedText);
		Mission& GetMissionData(int Value = 0);

		void SetCurrentMission(int Value)	{ m_CurrentMission = Value; }
		void AdvanceToNextMission()			{ m_CurrentMission++; }
		int GetCurrentMission()	{ return m_CurrentMission; }

		void Clear()	{ m_MissionContainer.clear(); 	SetCurrentMission(1); }


		bool IsCurrentMissionObjectiveComplete();

	private:
		int						m_CurrentMission;
		std::vector<Mission>	m_MissionContainer;
};

class Mission
{
public:
	
	Mission();
	Mission(std::string MissionName , std::string Text, std::string  MissionCompletedText);
	
	u8 GetCompleted() { return	m_Completed;}
	void SetCompleted(u8 Value) { m_Completed = Value;}


	std::string GetMissionText()					{ return m_MissionText; }
	void		SetMissionText(std::string Name)	{ m_MissionText = Name; }

	std::string GetMissionName()					{ return m_MissionName; } 
	void		SetMissionName(std::string Name)	{ m_MissionName = Name; } 

	std::string GetMissionCompletedText()					{ return m_MissionCompletedText; } 
	void		SetMissionCompletedText(std::string Name)	{ m_MissionCompletedText = Name; } 

private:

	std::string	m_MissionName;
	std::string	m_MissionText;
	std::string	m_MissionCompletedText;
	u8		m_Completed;  // 0 to 100
};

#endif
