#include "GameLogic.h"
#include "Singleton.h"

#include <gccore.h>
#include <math.h>
#include <sstream>
#include <algorithm>

#include "WiiManager.h"
#include "InputDeviceManager.h"
#include "FontManager.h"
#include "SoundManager.h"
#include "Vessel.h"
#include "render3D.h"
#include "Util.h"
#include "Util3D.h"
#include "debug.h"
#include "HashString.h"
#include "Config.h"
#include "Menu.h"
#include "mission.h"
#include "MessageBox.h"
#include "Timer.h"
#include "GameDisplay.h"
#include "introDisplay.h"
#include "Camera.h"

#include "MenuManager.h"
#include "MenuScreens.h"
#include "UpdateManager.h"



//#ifndef BUILD_FINAL_RELEASE
profiler_t profile_ProbeMineLogic;
profiler_t profile_Asteroid ;
profiler_t profile_MoonRocks;
profiler_t profile_SmallEnemies;
profiler_t profile_GunShip;
profiler_t profile_Explosions;
profiler_t profile_Spores;
profiler_t profile_Missile;
profiler_t profile_Exhaust;
profiler_t profile_Projectile;
//profiler_t profile_Mission;
profiler_t profile_ShotAndGunTurret;
profiler_t profile_DyingEnemies;

profiler_t profile_MissileCollisionLogic;
//#endif

//	 KEEP reminder... TargetIter  = m_GunShipContainer->begin(); 
//		pTarget = &(*m_GunShipContainer->begin());  !!! WARNING:  THIS METHOD DOES NOT WORK (gives very odd results - ptr will not update with shifting data) !!!


// TODO 
// 1 clear missiles after a level
// 2 blue pickup glow
// 3 probe mines - show red lock on lines

GameLogic::GameLogic():	m_bEndLevelTrigger(false), m_Score(0), m_ZoomAmountForSpaceBackground(3.1f),m_ClippingRadiusNeededForMoonRocks(0),  
m_GamePaused(false),m_GamePausedByPlayer(false),m_IsBaseShieldOnline(false),m_LastChanUsedForSoundAfterBurn(NULL),
m_bEnableRetrieveProbeMineMode(false), m_TerraformingCounter(0.0f)
{
}

void GameLogic::Init()
{
	m_pWii = Singleton<WiiManager>::GetInstanceByPtr();
	m_pSoundManager =  m_pWii->GetSoundManager();

	//-----------------------------------------------

	m_Timer = new Timer;
	m_MyVessel = new PlayerVessel;
	m_AimPointerContainer = new vector<guVector>;
	m_ShieldGeneratorContainer = new std::vector<Item3DChronometry>;
	m_AsteroidContainer = new std::vector<Item3D>;
	m_SporesContainer = new std::vector<Vessel>;
	m_EnemySatelliteContainer = new std::vector<Vessel>;
	m_MissileContainer = new std::vector<Vessel>;
	m_SmallEnemiesContainer = new std::vector<Vessel>;
	m_ExplosionsContainer = new std::vector<Vessel>;
	m_ProbeMineContainer = new std::vector<Vessel>;
	m_GunShipContainer = new std::vector<Vessel>;
	m_ExhaustContainer = new std::vector<Vessel>;
	m_ProjectileContainer = new std::vector<Vessel>;
	m_pMoonRocksContainer = new std::vector<Item3D>;
	m_pGunTurretContainer = new std::vector<TurretItem3D>;
	m_pMaterialPickUpContainer= new std::vector<Item3D>;
	m_ShotForGunTurretContainer = new std::vector<Item3D>;
	m_DyingEnemiesContainer = new std::vector<Vessel>;
	m_CelestialBodyContainer = new std::vector<MoonItem3D>;
	m_pHealthPickUpContainer = new std::vector<Vessel>;
	m_pScorePingContainer = new std::vector<ScorePingVessel>;

	m_ExhaustContainer->reserve(700); // best guess for the top end - it will grow if needed 
	m_SmallEnemiesContainer->reserve(50);
	m_ProbeMineContainer->reserve(329);
	m_ExplosionsContainer->reserve(20);
}

void GameLogic::DoControls()
{
	WPAD_ScanPads();
	m_pWii->GetInputDeviceManager()->Store();

	if (WPAD_ButtonsDown(0) & (WPAD_BUTTON_A)) 
	{
		m_pWii->GetMessageBox()->FadeOut();
	}

	if ( (WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS) ) 
		SetPaused(IsGamePausedByPopUp(), !IsGamePausedByPlayer() ); 

	if ( GetPlrVessel()->IsShieldOk() && !IsGamePaused() )
	{
		if ((WPAD_ButtonsDown(0) & WPAD_BUTTON_A) || (WPAD_ButtonsUp(0) & WPAD_BUTTON_A)) 
		{
			// Fire Missle
			m_pSoundManager->PlaySound( HashString::FireMissle,34,34);

			Vessel Missile = *GetPlrVessel();
			// NOTE: STUPID balue in missle logic need changing with this - TODO redo this
			Missile.SetFuel(60*11);   // about 20 seconds of missle life
			Missile.SetFrame( m_pWii->m_FrameEndStartConstainer[HashString::SmallMissile16x16].StartFrame );

			float dir = Missile.GetFacingDirection();
			static bool toggle = false;   // Fudge - todo replace this with something better
			toggle = !toggle;
			if (toggle) 
				dir+=10;
			else
				dir-=10;

			Missile.SetPos( Missile.GetX() + sin( dir )* -12.0, Missile.GetY() - cos( dir )* -12.0,	Missile.GetZ());
			Missile.AddVel(sin( Missile.GetFacingDirection() )* 1.45,-cos( Missile.GetFacingDirection() )* 1.45,0);
			Missile.SetGravityFactor(1.0f);
			m_MissileContainer->push_back(Missile);
		}


		DeviceNunChuck* pNunChuck( m_pWii->GetInputDeviceManager()->GetNunChuck() );
		if ( (pNunChuck != NULL) && (pNunChuck->GetEnable()) )
		{
			if (fabs(pNunChuck->GetJoyX()) > 0 || fabs(pNunChuck->GetJoyY()) > 0)
			{
				GetPlrVessel()->AddVel(	pNunChuck->GetJoyX()*0.075, pNunChuck->GetJoyY()*(-0.075), 0);
				//float ang = atan2(pNunChuck->GetJoyX(), pNunChuck->GetJoyY() );
				//GetPlrVessel()->SetFacingDirection( ang );

				guVector PlrVessel;
				PlrVessel.x = GetPlrVessel()->GetX() + (pNunChuck->GetJoyX()*100);
				PlrVessel.y = GetPlrVessel()->GetY() - (pNunChuck->GetJoyY()*100);
				f32 Turn = GetPlrVessel()->GetTurnDirection( &PlrVessel );
				GetPlrVessel()->AddFacingDirection( Turn * 0.085f );
			}
		}
		else
		{
			if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_B)  // use thrusters
			{
				if (m_LastChanUsedForSoundAfterBurn == NULL)
				{
					//m_LastChanUsedForSoundAfterBurn = m_pSoundManager->PlaySound( HashString::AfterBurn,255,255,true);
					//	m_LastChanUsedForSoundAfterBurn = m_pSoundManager->PlaySound( HashString::AfterBurn,40,40,true);

					m_pSoundManager->PlaySoundFromVoice( HashString::AfterBurn,40,40,true, m_pSoundManager->m_FixSoundVoice );
					m_LastChanUsedForSoundAfterBurn = m_pSoundManager->m_FixSoundVoice;

				}

				//---------------
				GetPlrVessel()->AddVel(	 sin(GetPlrVessel()->GetFacingDirection() )*0.075,
					-cos( GetPlrVessel()->GetFacingDirection() )*0.075,0);
				//---------------

				// Thrust from back of ship  - todo need to make this far more simple to use, need more functionality
				Vessel Tail = *GetPlrVessel();

				Tail.SetFrameGroup( m_pWii->GetFrameContainer((HashString::ExplosionThrust1Type16x16x10)), 0.35f);


				Tail.SetGravityFactor(1.0f);
				Tail.SetPos( Tail.GetX() + sin( Tail.GetFacingDirection() )* -8.45, 
					Tail.GetY() - cos( Tail.GetFacingDirection() )* -8.45, Tail.GetZ());
				Tail.AddVel(sin( Tail.GetFacingDirection() )* -1.75,-cos( Tail.GetFacingDirection() )* -1.75,0);
				Tail.SetSpin( (1000 - (rand()%2000 )) * 0.00005f );
				m_ExhaustContainer->push_back(Tail);	
			}
			else
			{
				if (m_LastChanUsedForSoundAfterBurn != NULL)		
				{
					//AESND_SetVoiceLoop(m_LastChanUsedForSoundAfterBurn, false);
					m_pSoundManager->StopSound(m_LastChanUsedForSoundAfterBurn);
					m_LastChanUsedForSoundAfterBurn = NULL;	
				}
			}
		}

		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_DOWN)  // Mines
		{


			//for (int i=0; i<50; i++)  //NOT FOR RELEASE - A BIT OF FUN WITH MINES!!!!!
			//{
			Vessel ProbeMine = *GetPlrVessel();
			ProbeMine.SetFrameGroup( m_pWii->GetFrameContainer(HashString::ProbeMine16x16x2),0.05f);   // new mine

			float dir = ProbeMine.GetFacingDirection() - (((rand()%(314/16))-(314/32))*0.01);
			float v = (rand()%10) * 0.05f;
			ProbeMine.SetGravityFactor(0.985f);
			ProbeMine.AddVel(sin( dir  )* -(1.75+v),-cos( dir )* -(1.75+v),0);
			ProbeMine.SetFacingDirection(0);
			m_ProbeMineContainer->push_back(ProbeMine);
			//}

			m_pSoundManager->PlaySound( HashString::DropMine,112,112 );

		}
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_UP)  // Mines
		{
			m_bEnableRetrieveProbeMineMode = !m_bEnableRetrieveProbeMineMode;
		}
	}
}

void GameLogic::StillAlive()
{
	// debug test
	//			if (m_TerraformingCounter < 255.0f) {
	//		m_TerraformingCounter += 0.25f;
	//		}

	if ( GetPlrVessel()->GetShieldLevel() <= 20 ) 	{
		if (GetPlrVessel()->m_LastShieldLevel != GetPlrVessel()->GetShieldLevel()) {
			if ( GetPlrVessel()->GetShieldLevel() > 14 ) { // warning only in this small range - stops over use
				GetPlrVessel()->m_PopUpMessageTimer.SetTimerSeconds(4);
				GetPlrVessel()->m_LastShieldLevel = GetPlrVessel()->GetShieldLevel();
			}
		}
	}



	if (GetPlrVessel()->GetHitCoolDownTimer() > 0) {
		GetPlrVessel()->AddHitCoolDownTimer(-1);
	}

	TurnShipLogic();

	float ScrollFactor = 0.065f;

	if (IsGamePaused())
	{
		if (IsGamePausedByPlayer())
			ScrollFactor = 0.000f;	
		else
			ScrollFactor = 0.0125f;	
	}
	m_pWii->GetCamera()->CameraMovementLogic(GetPlrVessel(),ScrollFactor);

	MissionManager* pMissionManager( m_pWii->GetMissionManager() );
	Mission& MissionData( pMissionManager->GetMissionData() );

	//	m_SporesContainer->clear();    // debug test
	//	m_SmallEnemiesContainer->clear();
	//	m_GunShipContainer->clear();

	if ( pMissionManager->IsCurrentMissionObjectiveComplete() ){
		if ( (MissionData.GetCompleted()>0) && (!m_pWii->GetMessageBox()->IsEnabled()) )
		{
			if (!MissionData.GetMissionCompletedText().empty()) {
				m_pWii->GetMessageBox()->SetUpMessageBox("Mission Complete",MissionData.GetMissionCompletedText());
			}
			MissionData.SetCompleted(100);
			// setup things for the next mission... here
			switch (pMissionManager->GetCurrentMission())
			{
			case 1:  // get shiled generators online
				GetPlrVessel()->SetPos(0,0,0);
				GetPlrVessel()->SetVel(0,0,0);
				m_ProbeMineContainer->clear();
				m_ExhaustContainer->clear();
				InitialiseShieldGenerators(3);

				GetPlrVessel()->SetShieldLevel( m_pWii->GetXmlVariable(HashString::PlayerMaxShieldLevel) );

				break;
			case 2:  // Amarda Arround Last Shield Generator
				InitialiseEnermyAmardaArroundLastShieldGenerator( m_pWii->GetConfigValueWithDifficultyApplied(HashString::EnermyAmardaArroundLastShieldGenerator), 255.0f );
				break;
			case 3:  // shiled generators now online
				GetPlrVessel()->ClearPickUpTotal();
				GetPlrVessel()->SetPos(0,0,0);
				GetPlrVessel()->SetVel(0,0,0);
				m_ProbeMineContainer->clear();
				m_ExhaustContainer->clear();
				m_IsBaseShieldOnline = true;
				GetPlrVessel()->SetShieldLevel( m_pWii->GetXmlVariable(HashString::PlayerMaxShieldLevel) );
				break;
			case 4:  // pick up
				GetPlrVessel()->ClearPickUpTotal();
				GetPlrVessel()->SetPos(0,0,0);
				GetPlrVessel()->SetVel(0,0,0);
				m_ExhaustContainer->clear();
				m_ProbeMineContainer->clear();

				AddEnemy(0,0,m_pWii->ApplyDifficultyFactor(16),HashString::GunShip,1750);
				AddEnemy(0,0,m_pWii->ApplyDifficultyFactor(10),HashString::GunShip,1640);
				AddEnemy(0,0,m_pWii->ApplyDifficultyFactor(45),HashString::SmallWhiteEnemyShip16x16x2,1400);
				AddEnemy(0,0,m_pWii->ApplyDifficultyFactor(30),HashString::SmallRedEnemyShip16x16x2,1700);
				GetPlrVessel()->SetShieldLevel( m_pWii->GetXmlVariable(HashString::PlayerMaxShieldLevel) );
				break;
			case 5:  // defence now online
				GetPlrVessel()->ClearPickUpTotal();
				GetPlrVessel()->SetPos(0,0,0);
				GetPlrVessel()->SetVel(0,0,0);
				m_ExhaustContainer->clear();
				m_ProbeMineContainer->clear();
				InitialiseSmallGunTurret(4,700, 0,120,600, 3.14/3.0f );
				GetPlrVessel()->SetShieldLevel( m_pWii->GetXmlVariable(HashString::PlayerMaxShieldLevel) );
				break;
			case 6:
				GetPlrVessel()->SetPos(0,0,0);
				GetPlrVessel()->SetVel(0,0,0);
				m_ExhaustContainer->clear();
				m_ProbeMineContainer->clear();
				m_IsBaseShieldOnline = false;
				break;
			case 7:
				GetPlrVessel()->SetPos(0,0,0);
				GetPlrVessel()->SetVel(0,0,0);
				break;

			}

			pMissionManager->AdvanceToNextMission();
		}
	}

	if ( MissionData.GetCompleted()==1) 
	{
		switch (pMissionManager->GetCurrentMission())
		{
		case 1:  // get shiled generators online
			break;
		case 2:  // Amarda Arround Last Shield Generator
			break;
		case 3:  // shiled generators now online
			break;
		case 4:  // pick up

			if (m_SmallEnemiesContainer->size() < m_pWii->ApplyDifficultyFactor(14))
			{
				AddEnemy(0,0,m_pWii->ApplyDifficultyFactor(45),HashString::SmallWhiteEnemyShip16x16x2,1700);
				AddEnemy(0,0,m_pWii->ApplyDifficultyFactor(30),HashString::SmallRedEnemyShip16x16x2,1800);
			}
			break;
		case 5:  // defence now online

			if (m_SmallEnemiesContainer->size()<m_pWii->ApplyDifficultyFactor(14))
			{
				AddEnemy(0,0,m_pWii->ApplyDifficultyFactor(45),HashString::SmallWhiteEnemyShip16x16x2,1700);
				AddEnemy(0,0,m_pWii->ApplyDifficultyFactor(30),HashString::SmallRedEnemyShip16x16x2,1800);
			}
			//??? just attack what's left.... run out of things to shoot before enough scrap collected
			break;
		case 6:
			if (m_SmallEnemiesContainer->size()<m_pWii->ApplyDifficultyFactor(24))
			{
				AddEnemy(0,0,m_pWii->ApplyDifficultyFactor(45),HashString::SmallWhiteEnemyShip16x16x2,1700);
				AddEnemy(0,0,m_pWii->ApplyDifficultyFactor(30),HashString::SmallRedEnemyShip16x16x2,1800);
			}

			if (m_GunShipContainer->size()<m_pWii->ApplyDifficultyFactor(8))
			{
				AddEnemy(0,0,m_pWii->ApplyDifficultyFactor(28),HashString::GunShip,1700);
			}

			if (m_TerraformingCounter < 255.0f) {
				m_TerraformingCounter += 0.01f;
			}

			break;
		case 7:

			//
			// survival mission - never ends 
			//
			if (m_SmallEnemiesContainer->size()<m_pWii->ApplyDifficultyFactor(40))
			{
				AddEnemy(0,0,m_pWii->ApplyDifficultyFactor(55),HashString::SmallWhiteEnemyShip16x16x2,1700);
				AddEnemy(0,0,m_pWii->ApplyDifficultyFactor(40),HashString::SmallRedEnemyShip16x16x2,1800);
			}

			if (m_GunShipContainer->size()<m_pWii->ApplyDifficultyFactor(16))
			{
				AddEnemy(0,0,m_pWii->ApplyDifficultyFactor(32),HashString::GunShip,1700);
			}

			break;
		}
	}

	if ((MissionData.GetCompleted()==0) && (!m_pWii->GetMessageBox()->IsEnabled()) )
	{
		// next mission message
		m_pWii->GetMessageBox()->SetUpMessageBox(MissionData.GetMissionName(),MissionData.GetMissionText());
		MissionData.SetCompleted(1);  // 0% to 100%
		m_pSoundManager->PlaySound( HashString::Alarm2 );
	}

	if ( IsGamePausedByPlayer() )
	{
		if (m_LastChanUsedForSoundAfterBurn != NULL)		
		{
			m_pSoundManager->StopSound(m_LastChanUsedForSoundAfterBurn);
			m_LastChanUsedForSoundAfterBurn = NULL;	
		}
	}
	else
	{
		// logic pauses the game if msgbox is needed
		SetPaused( m_pWii->GetMessageBox()->IsEnabled() ); 
	}
}


map <u32, vector<Vessel*> > CollisionContainer;  

//int GetHash(float x,  float y)
int GetHash(Vessel* v)
{
	// return (x*1640531513ul ^ y*2654435789ul) % Amount;

	static const int CellSize = 64;
	static const int SceneWidth = CellSize*100;   // this figure will directly effect memory usage
	//return (u32)( floor(v->GetX()/CellSize) + ( (u32)(floor(v->GetY()/CellSize) ) * SceneWidth) );
	return (u32)( (v->GetX()/CellSize) + ( (u32)(v->GetY()/CellSize) * SceneWidth) );
}


void GameLogic::DoGameLogic()
{
	CollisionContainer.clear();



	for (std::vector<Vessel>::iterator BadIter(m_SmallEnemiesContainer->begin()); BadIter!=m_SmallEnemiesContainer->end(); ++BadIter)
	{
		CollisionContainer[ GetHash( &(*BadIter) ) ].push_back( &(*BadIter) ); // should do this for the corners not the centre!!!!

		// todo ...  rather then add 1/2 sprite for w/h (4 corners, so 4 lines of code) , alway make sure 4 different grids are hit
		// by adding the complete grid size to the corners.

		// this will alway give the worse case both will not require any logic to make sure a vessel is not addd to the some place

	}


	for (std::vector<Vessel>::iterator BadIter(m_GunShipContainer->begin()); BadIter!=m_GunShipContainer->end(); ++BadIter)
	{
		CollisionContainer[ GetHash( &(*BadIter) ) ].push_back( &(*BadIter) );
	}


	//----

	//	static float ang(0.0f);
	//	ang+=0.0065f;

	FakLockOn_AngInc+=0.0065f;

	if ( GetPlrVessel()->IsShieldOk() )
	{
		m_CPUTarget.x = GetPlrVessel()->GetX();
		m_CPUTarget.y = GetPlrVessel()->GetY();

		m_CPUTarget_Miss.x = GetPlrVessel()->GetX() + sin(FakLockOn_AngInc)*120.0f;
		m_CPUTarget_Miss.y = GetPlrVessel()->GetY() + cos(FakLockOn_AngInc)*120.0f;
	}
	else
	{
		m_CPUTarget.x = GetPlrVessel()->GetX() + sin(FakLockOn_AngInc)*220.0f;
		m_CPUTarget.y = GetPlrVessel()->GetY() + cos(FakLockOn_AngInc)*220.0f;
	}

	DoControls();

	if (!IsGamePaused())
	{
		GetPlrVessel()->FactorVel();
		GetPlrVessel()->AddVelToPos();

		PickUpsLogic();
		HealthPickUpsLogic();

		MissileLogic();

		if (m_bEnableRetrieveProbeMineMode)
		{
			RetrieveProbeMineLogic( 0.035f * 2 );
		}
		else
		{
			ProbeMineLogic(m_SmallEnemiesContainer, 0.035f, 22.0f);
			ProbeMineLogic(m_GunShipContainer, 0.035f, 48.0f); 
		}


		//ProbeMineCollisionLogic(m_SmallEnemiesContainer, 12.0f * 12.0f );
		//ProbeMineCollisionLogic(m_GunShipContainer, 38.0f * 38.0f );

		ProbeMineCollisionLogic();


		BadShipsLogic();

		PlayerCollisionLogic();

		FeoShieldLevelLogic();

		MissileCollisionLogic();

		MoonRocksLogic();

		//	m_pWii->profiler_start(&myjob1);
		AsteroidsLogic();
		//	m_pWii->profiler_stop(&myjob1);
		SporesMovementLogic();


		GunShipLogic();


		m_pWii->profiler_start(&profile_ShotAndGunTurret);
		GunTurretShotsLogic( m_GunShipContainer );
		GunTurretShotsLogic(m_SmallEnemiesContainer);  // before GunTurretLogic - so each shot leaves the barrel correctly
		GunTurretLogic();
		m_pWii->profiler_stop(&profile_ShotAndGunTurret);


		ExhaustLogic();
		ProjectileLogic();
		ExplosionLogic();

		DyingShipsLogic();

		CelestialBodyLogic();

		ScorePingLogic();
	}

	m_pWii->GetGameDisplay()->DisplayAllForIngame();

	if (GetPlrVessel()->HasShieldFailed())
	{
		// player has died
		static float AmountOfexplosionsToAddEachFrame(20);

		m_pWii->GetCamera()->CameraMovementLogic(GetPlrVessel(), 0.065f );

		if (m_bDoEndGameEventOnce)
		{
			GetPlrVessel()->m_PopUpMessageTimer.ResetTimer();
			m_Timer->SetTimerSeconds(1); // SetTimer(1); 
			m_bDoEndGameEventOnce = false;
			AmountOfexplosionsToAddEachFrame = 20;

			if ( m_LastChanUsedForSoundAfterBurn != NULL)		
			{
				m_pSoundManager->StopSound(m_LastChanUsedForSoundAfterBurn);
				m_LastChanUsedForSoundAfterBurn = NULL;	
				AmountOfexplosionsToAddEachFrame = 20;
			}

			// Eject loads of mines on death as a final death throw to the enemy
			Vessel ProbeMine = *GetPlrVessel();
			ProbeMine.SetGravityFactor(0.995f);
			ProbeMine.SetFrameGroup( m_pWii->GetFrameContainer(HashString::ProbeMine16x16x2),0); // zero no anim
			ProbeMine.SetFacingDirection(0);	
			for (int i=0; i<200; i++)
			{
				float vel = 1.5f  + ((rand()%100) * 0.01f);
				float ang = (rand()%(314*2)) * 0.1f;
				ProbeMine.SetVel( sin(ang) * vel , cos(ang) * vel ,0);
				m_ProbeMineContainer->push_back(ProbeMine);
			}
		}
		if (m_Timer->IsTimerDone())
		{
			if (IsEndLevelTrigger())
			{
				m_pWii->SetGameState(WiiManager::eIntro);
			}
			else
			{
				SetEndLevelTrigger(true);
				m_Timer->SetTimerSeconds(60); // wait before auto continue into menus
			}
		}

		static float wobble	(0);
		wobble+=0.065;

		GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
		Util3D::Trans(m_pWii->GetCamera()->GetCamX(),m_pWii->GetCamera()->GetCamY());
		m_pWii->GetFontManager()->SetFontColour(255,255,0,255);
		m_pWii->GetFontManager()->DisplayTextCentre("Final Score",	0,-200,128,HashString::LargeFont);
		m_pWii->GetFontManager()->DisplayTextCentre( Util::NumberToString( GetScore() ),	0,-150,166,HashString::LargeFont);
		m_pWii->GetFontManager()->SetFontColour(255,255,255,255);

		if (IsEndLevelTrigger())
		{
			Util3D::Trans(m_pWii->GetCamera()->GetCamX(),m_pWii->GetCamera()->GetCamY());
			m_pWii->GetFontManager()->DisplayTextCentre("PRESS A TO CONTINUE",exp(sin(wobble)*2.8f),150,128,HashString::LargeFont);
		}

		//	static const HashLabel names[] = { HashString::ExplosionThrust1Type16x16x10 , HashString::ExplosionSmoke1Type16x16x10 };
		static const HashLabel names[] = {	HashString::ExplosionThrust1Type16x16x10 , HashString::ExplosionSmoke1Type16x16x10 ,
			HashString::ExplosionSolidType32x32x10 , HashString::ExplosionSmoke2Type16x16x10 ,
			HashString::ExplosionRingType32x32x7 , HashString::SmokeTrail16x16x10 ,
			HashString::ExplosionStarType32x32x4 , HashString::ExplosionDull1Type16x16x10 };

		if (AmountOfexplosionsToAddEachFrame > 1)
		{
			Vessel Boom = *GetPlrVessel();
			Boom.SetGravityFactor(1);
			int size = ( sizeof(names) / sizeof(HashLabel) ) ;
			int Amount = (rand()%(int)AmountOfexplosionsToAddEachFrame);
			for (int i=0; i<Amount; i++) // add lots of random explosions
			{
				int nameindex = rand()%size; 	
				Boom.SetFrameGroup( m_pWii->GetFrameContainer(names[nameindex]) ,0.15f + ((rand()%100)*0.0015f));
				Boom.SetVel((1000.0f-(rand()%2000)) * 0.0028,(1000.0f-rand()%2000) * 0.0028, 2);
				Boom.AddVelToPos();  // Don't let the explosions bunch up in the centre
				Boom.AddVelToPos();
				Boom.SetSpin( (1000 - (rand()%2000)) * 0.00015f );
				m_ExplosionsContainer->push_back(Boom);	
			}
			AmountOfexplosionsToAddEachFrame -= 0.05;
		}
	}
	else  // we are still alive
	{
		StillAlive();
	}


	// Logic for  Aim Pointer
	Vtx* WiiMote( m_pWii->GetInputDeviceManager()->GetIRPosition() );
	if (WiiMote!=NULL)
	{
		m_AimPointerContainer->clear();
		int WorldX = m_pWii->GetCamera()->GetCamX() + WiiMote->x - (m_pWii->GetScreenWidth() / 2);
		int WorldY = m_pWii->GetCamera()->GetCamY() + WiiMote->y - (m_pWii->GetScreenHeight() / 2);
		guVector p = {WorldX,WorldY,0};
		m_AimPointerContainer->push_back(p);
	}


	m_pWii->SwapScreen();  

}

void GameLogic::AsteroidsLogic()
{
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );
	float Radius( 640 * 640 ); //m_pWii->GetXmlVariable(HashString::ViewRadiusForAsteroids));
	for (std::vector<Item3D>::iterator iter(m_AsteroidContainer->begin()); iter!= m_AsteroidContainer->end(); ++iter )
	{	
		/// timings before this  'InsideRadius' line: 1327 to 1428, now with this line: 87 to 192 ! 
		if ( iter->InsideRadius(fCamX, fCamY, Radius ) ) {  // fast clipping, sphere sits arround frustum
			iter->Rotate();
			iter->SetEnable(true);
		}
		else {
			iter->SetEnable(false);
		}
	}
}

void GameLogic::ProjectileLogic()
{
	m_pWii->profiler_start(&profile_Projectile);

	for (std::vector<Vessel>::iterator Iter(m_ProjectileContainer->begin()); Iter!= m_ProjectileContainer->end(); /*NOP*/)
	{	
		Iter->AddFrame();
		if ( (floor)(Iter->GetFrame()) <= Iter->GetEndFrame() ) {
			Iter->AddFacingDirection( Iter->GetSpin() );
			Iter->AddVelToPos(); 
			Iter->FactorVel();
			++Iter;
		}
		else  { // end of projectile's life - this will add a final spent explosion at the end
			AddScalingAnim(HashString::YellowEdgeExplosion64x64, 
				&(*Iter),
				0.05f, //frame speed
				( 800 - ( rand()%1600) ) * 0.00025f, // Spin Amount
				0.85f, //Top Scale
				0.001f, // Start Scale
				0.15f ); // Scale Factor

			*Iter = m_ProjectileContainer->back();  // erase
			m_ProjectileContainer->pop_back();		//
		}
	}
	m_pWii->profiler_stop(&profile_Projectile);
}

void GameLogic::ExhaustLogic()
{
	m_pWii->profiler_start(&profile_Exhaust);

	for (std::vector<Vessel>::iterator Iter(m_ExhaustContainer->begin()); Iter!= m_ExhaustContainer->end(); /*NOP*/)
	{	
		Iter->AddFrame();
		if ( (floor)(Iter->GetFrame()) <= Iter->GetEndFrame() )
		{
			Iter->AddFacingDirection( Iter->GetSpin() );
			Iter->AddVelToPos(); 
			Iter->FactorVel();
			++Iter;
		}
		else {
			// Better way to erase - erase can be Inefficient as it will delete elements from a vector shifting later elements down.
			*Iter = m_ExhaustContainer->back();		// ERASE
			m_ExhaustContainer->pop_back();			//
		}
	}

	m_pWii->profiler_stop(&profile_Exhaust);
}

void GameLogic::ExplosionLogic()
{
	m_pWii->profiler_start(&profile_Explosions);

	for (std::vector<Vessel>::iterator ExplosionIter(m_ExplosionsContainer->begin()); ExplosionIter!= m_ExplosionsContainer->end(); /*NOP*/) {	
		// round down current frame
		if (ExplosionIter->GetScaleToFactor() == 1.0f) {
			ExplosionIter->SetAlpha( max( 0, 255 - ((int)ExplosionIter->GetZ()/2) ) ); // fudge ... for when ships fall into distance
		}
		else {
			float Factor = 1.0f - (ExplosionIter->GetFrame() - ExplosionIter->GetFrameStart());
			ExplosionIter->SetAlpha( Factor * 255 ); 
		}

		ExplosionIter->AddFacingDirection( ExplosionIter->GetSpin() );
		ExplosionIter->AddVelToPos(); 
		ExplosionIter->FactorVel();
		ExplosionIter->AddCurrentScaleFactor( ((ExplosionIter->GetScaleToFactor() - ExplosionIter->GetCurrentScaleFactor()) * ExplosionIter->GetScaleToFactorSpeed() ));	
		ExplosionIter->AddFrame();


		if ( (floor)(ExplosionIter->GetFrame()) <= ExplosionIter->GetEndFrame() ) {
			++ExplosionIter;
		}
		else {
			// next two line will erase iter, since a call to "erase" can be inefficient
			*ExplosionIter = m_ExplosionsContainer->back();   // erase
			m_ExplosionsContainer->pop_back();
		}
	}
	m_pWii->profiler_stop(&profile_Explosions);
}

void GameLogic::TurnShipLogic()
{
	if (IsGamePaused())
		return;

	Vtx* WiiMote( m_pWii->GetInputDeviceManager()->GetIRPosition() );
	if (WiiMote!=NULL)
	{
		DeviceNunChuck* pNunChuck( m_pWii->GetInputDeviceManager()->GetNunChuck() );
		if ( (pNunChuck != NULL) && (pNunChuck->GetEnable()) )
		{
			//GetPlrVessel()->AddFacingDirection( pNunChuck->GetJoyX() * 0.25f );
		}
		else
		{
			f32 HalfScreenWidthIncludingOverrun  = m_pWii->GetScreenWidth() / 2;
			f32 HalfScreenHeightIncludingOverrun = m_pWii->GetScreenHeight() / 2;
			guVector PlrVessel;
			PlrVessel.x = m_pWii->GetCamera()->GetCamX() + WiiMote->x - HalfScreenWidthIncludingOverrun;
			PlrVessel.y = m_pWii->GetCamera()->GetCamY() + WiiMote->y - HalfScreenHeightIncludingOverrun;

			f32 Turn = GetPlrVessel()->GetTurnDirection( &PlrVessel );
			GetPlrVessel()->AddFacingDirection( Turn * 0.085f );
		}
	}
}

void GameLogic::NukeArea(guVector& Pos, float fRadius ) 
{
	fRadius *= fRadius;
	for (std::vector<Vessel>::iterator Iter(m_SporesContainer->begin()); Iter!=m_SporesContainer->end();++Iter) {
		if ( Iter->InsideRadius(Pos.x, Pos.y, fRadius ) ) {
			Iter->SetShieldLevel(0);
		}
	}

	for (std::vector<Vessel>::iterator Iter(m_SmallEnemiesContainer->begin()); Iter!=m_SmallEnemiesContainer->end();++Iter) {
		if ( Iter->InsideRadius(Pos.x, Pos.y, fRadius ) ) {
			Iter->SetShieldLevel(0);
		}
	}

	for (std::vector<Vessel>::iterator Iter(m_GunShipContainer->begin()); Iter!=m_GunShipContainer->end();++Iter) {
		if ( Iter->InsideRadius(Pos.x, Pos.y, fRadius ) ) {
			Iter->AddShieldLevel(-3);
		}
	}
}

void GameLogic::SporesMovementLogic()
{
	for (std::vector<Vessel>::iterator ThingIter(m_SporesContainer->begin()); ThingIter!=m_SporesContainer->end(); ++ThingIter)
	{
		guVector& dv = ThingIter->GetDestinationPos();

		if ( ThingIter->InsideRadius(dv.x,dv.y, 32*32) ) {
			ThingIter->SetGravityFactor(1.0f);
			ThingIter->SetFuel(0); // hack for drawing behind things
		}
		else {
			if ( ThingIter->GetFuel() ) {

				float x = dv.x + (32 - rand()%64);
				float y = dv.y + (32 - rand()%64);

				f32 LockOn = atan2( x - ThingIter->GetX(), y - ThingIter->GetY() );

				float temp_mx(sin( LockOn )* 0.01f);
				float temp_my(cos( LockOn )* 0.01f);

				ThingIter->AddVel(temp_mx,temp_my,0);
				ThingIter->SetGravityFactor(0.995f);
			} else {
				// wait until it's outside the radius
				if ( ThingIter->InsideRadius(dv.x,dv.y, 44*44) == false ) {
					ThingIter->SetFuel(1);
				}
			}

		}
		ThingIter->FactorVel();
		ThingIter->AddVelToPos();
	}
}

void GameLogic::MissileCollisionLogic()
{
	m_pWii->profiler_start(&profile_MissileCollisionLogic);


	float fRocketCollisionRadius = 3*16; // large gun ship
	fRocketCollisionRadius *= fRocketCollisionRadius;

	for (std::vector<Vessel>::iterator MissileIter(m_MissileContainer->begin()); MissileIter!=m_MissileContainer->end(); ++MissileIter )
	{
		for (std::vector<Vessel>::iterator GunShipIter(m_GunShipContainer->begin()); GunShipIter!=m_GunShipContainer->end(); ++GunShipIter)
		{
			if ( (GunShipIter->InsideRadius(MissileIter->GetX(), MissileIter->GetY(), fRocketCollisionRadius )) )
			{
				AddScore( &(*GunShipIter) );  // add points for each missile hit

				GunShipIter->AddShieldLevel(-1);

				if ( GunShipIter->IsShieldOk() )  // hit something but their shileds are holding
				{
					MissileIter->FactorVel(0.65f);

					AddAnim(HashString::ExplosionStarType32x32x4, &(*MissileIter), 0.20f ,0.25f);
					AddAnim(HashString::ExplosionSmoke2Type16x16x10, &(*MissileIter), 0.07f ,0.2f);

					m_pSoundManager->PlayRandomHullBang();
					m_pSoundManager->PlayRandomHull();
				}

				MissileIter->SetFuel(0);  // remove missle
				break;
			}
		}

		float fRocketCollisionRadius = m_pWii->GetXmlVariable( HashString::RocketCollisionRadius );
		fRocketCollisionRadius *= fRocketCollisionRadius;



		// slow		for (std::vector<Vessel>::iterator BadIter(m_SmallEnemiesContainer->begin()); BadIter!=m_SmallEnemiesContainer->end(); ++BadIter)
		vector<Vessel*>* p =  &CollisionContainer[ GetHash( &(*MissileIter) ) ];
		for (std::vector<Vessel*>::iterator BadIter2(p->begin() ); BadIter2!=(p->end() ); ++BadIter2)
		{
			Vessel* BadIter = *BadIter2;
			if ( (BadIter->InsideRadius(MissileIter->GetX(), MissileIter->GetY(), fRocketCollisionRadius )) )
			{
				AddAnim(HashString::ExplosionThrust1Type16x16x10, &(*BadIter), 0.25f, (1000 - (rand()%2000 )) * 0.00005f );

				BadIter->AddShieldLevel(-1);
				if (BadIter->IsShieldOk())  // hit something but their shileds are holding
				{
					AddScore( &(*BadIter) );  // add points for each missile hit

					m_pSoundManager->PlayRandomHullBang();

					BadIter->AddFacingDirection(M_PI*0.5f);
					BadIter->AddVel( sin( MissileIter->GetFacingDirection() )*2.1f,-cos( MissileIter->GetFacingDirection() )*2.1f, 0 );
				}

				MissileIter->SetFuel(0);
				break;
			}
		}

		//for (std::vector<Vessel>::iterator BadIter(m_SmallEnemiesContainer->begin()); BadIter!=m_SmallEnemiesContainer->end(); ++BadIter)
		//{
		//	if ( (BadIter->InsideRadius(MissileIter->GetX(), MissileIter->GetY(), fRocketCollisionRadius )) )
		//	{
		//		AddAnim(HashString::ExplosionThrust1Type16x16x10, &(*BadIter), 0.25f, (1000 - (rand()%2000 )) * 0.00005f );

		//		BadIter->AddShieldLevel(-1);
		//		if (BadIter->IsShieldOk())  // hit something but their shileds are holding
		//		{
		//			AddScore( &(*BadIter) );  // add points for each missile hit

		//			m_pSoundManager->PlayRandomHullBang();

		//			BadIter->AddFacingDirection(M_PI*0.5f);
		//			BadIter->AddVel( sin( MissileIter->GetFacingDirection() )*2.1f,-cos( MissileIter->GetFacingDirection() )*2.1f, 0 );
		//		}

		//		MissileIter->SetFuel(0);
		//		break;
		//	}
		//}

	}

	for (std::vector<Vessel>::iterator MissileIter(m_MissileContainer->begin()); MissileIter!=m_MissileContainer->end(); ++MissileIter )
	{
		for (std::vector<Vessel>::iterator ThingIter(m_SporesContainer->begin()); ThingIter!=m_SporesContainer->end();/* ++iter*/)
		{
			if ( ThingIter->InsideRadius(MissileIter->GetX(), MissileIter->GetY(), 10*10 ) ) {

				ThingIter->AddShieldLevel(-1);
				MissileIter->SetFuel(0);
				break;			
			}
			++ThingIter;
		}
	}
	EnemySatelliteCollisionLogic();

	m_pWii->profiler_stop(&profile_MissileCollisionLogic);
}


void GameLogic::EnemySatelliteCollisionLogic()
{
	for (std::vector<Vessel>::iterator MissileIter(m_MissileContainer->begin()); MissileIter!=m_MissileContainer->end(); ++MissileIter ) {
		for (std::vector<Vessel>::iterator ThingIter(m_EnemySatelliteContainer->begin()); ThingIter!=m_EnemySatelliteContainer->end();/* ++iter*/) {

			if ( ThingIter->InsideRadius(MissileIter->GetX(), MissileIter->GetY(), 65*65 ) ) {
				// // could opt this bit - but the vessel class is starting to bloat!
				int count(0);
				for (std::vector<Vessel>::iterator Spore(m_SporesContainer->begin()); Spore!=m_SporesContainer->end(); ++Spore) {
					if (ThingIter->GetID() == Spore->GetID()) {
						count++;
					}
				}

				// hit shield - bounce back missile
				if (count >= m_pWii->GetXmlVariable(HashString::AmountSporesNeededForShiled)) { // How many spores for shileds to stay up

					ThingIter->SetHitCoolDownTimer(170);

					if ( ThingIter->InsideRadius(MissileIter->GetX(), MissileIter->GetY(), 55*55 ) ) {
						MissileIter->FactorVel(-0.45f);
						MissileIter->SetFuel(0);

						m_pSoundManager->PlaySound( HashString::HitShield);

						AddScalingAnim(HashString::ShieldRed, 
							&(*MissileIter),
							0.023f,				// frame speed
							0.23f,				// Spin Amount
							0.60f, 				// Top Scale
							0.01f,				// Start Scale
							0.12f );			// Scale Factor
					}
				}
				else {
					ThingIter->SetHitCoolDownTimer(0);
				}

				// kill zone - shields down
				if ( ThingIter->InsideRadius(MissileIter->GetX(), MissileIter->GetY(), 30*30 ) ) {
					MissileIter->SetFuel(0);
					ThingIter->AddShieldLevel(-1);

					if (ThingIter->GetShieldLevel()==4) {
						ThingIter->AddFrame(1.0f); // it's taken that the next frame will be the damaged graphic
					}

					MissileIter->FactorVel(0.15f);
					AddScalingAnim(HashString::RedEdgeExplosion64x64, 
						&(*MissileIter),
						0.035f,				// frame speed
						0.0f,				// Spin Amount
						0.85f, 				// Top Scale
						0.01f,				// Start Scale
						0.0925f );			// Scale Factor

					AddAnim(HashString::ExplosionRingType32x32x7, &(*MissileIter), 0.25f, 0 );

					if (ThingIter->IsShieldOk()) {

						if ( ThingIter->GetShieldLevel() > 4)
							m_pSoundManager->PlayRandomHull();
						else
							m_pSoundManager->PlayRandomHullBang();

						AddScore( &(*ThingIter) );  // add points for each missile hit
					}
					break;  // check next missle
				}
			}
			++ThingIter;
		}
	}
}

void GameLogic::FeoShieldLevelLogic()   // explode enemy ships
{
	// Enemy ships (small)
	for (std::vector<Vessel>::iterator BadIter(m_SmallEnemiesContainer->begin()); BadIter!=m_SmallEnemiesContainer->end(); /*++BadIter*/ )
	{
		if ( BadIter->HasShieldFailed() ) {

			AddScore( &(*BadIter) );
			AddPickUps(&(*BadIter),1);

			if ( (rand() % ( 20 - BadIter->GetKillValue() ) ) == 0) { // KV : 5 to 10
				AddHealthPickUps( &(*BadIter) );
			}

			BadIter->SetFuel(100 + rand()%50);  // final death time
			BadIter->AddVel(0,0,0.95f);
			BadIter->SetGravityFactor(1.0f); // fall into deep space
			m_DyingEnemiesContainer->push_back(*BadIter); // add to DyingEnemies pot

			m_pSoundManager->PlayRandomExplodeSound();

			*BadIter = m_SmallEnemiesContainer->back();  // ERASE , swap last element with current
			m_SmallEnemiesContainer->pop_back();		 // now just delete the last element, "ta-dah!"
		}
		else {
			++BadIter;
		}
	}

	// Enemy Gun ships
	for (std::vector<Vessel>::iterator GunShipIter(m_GunShipContainer->begin()); GunShipIter!=m_GunShipContainer->end(); /* ++GunShipIter */)
	{
		if ( GunShipIter->HasShieldFailed() ) {

			AddHealthPickUps(&(*GunShipIter));

			AddScore( &(*GunShipIter) );
			AddPickUps(&(*GunShipIter),5);

			GunShipIter->SetFuel(175); // final death time
			GunShipIter->SetGravityFactor(1.0f);  // better prediction for turrets
			GunShipIter->SetSpeedFactor( 1.0 );

			for (int i=0; i<32 ; ++i) {
				GunShipIter->SetVel(0,0,0);
				GunShipIter->AddVel(3.0f - (( rand()%600) * 0.01f),3.0f - (( rand()%600) * 0.01f),1.2f);
				m_DyingEnemiesContainer->push_back((*GunShipIter)); // Add to DyingEnemies
			}

			m_pSoundManager->PlayRandomBigExplodeSound();

			*GunShipIter = m_GunShipContainer->back();  // erase
			m_GunShipContainer->pop_back();				//
		}
		else {
			++GunShipIter;
		}
	}


	// Enemy spores
	for (std::vector<Vessel>::iterator iter(m_SporesContainer->begin()); iter!=m_SporesContainer->end(); /* ++iter*/ ) {
		if (iter->HasShieldFailed()) {
			AddEnemySpawn(*iter);
			AddScore(&(*iter));
			m_pSoundManager->PlayRandomExplodeSound();
			*iter = m_SporesContainer->back();  // Quick erase
			m_SporesContainer->pop_back();
		} else {
			++iter;
		}
	}


	// Enemy Satellites
	for (std::vector<Vessel>::iterator iter(m_EnemySatelliteContainer->begin()); iter!=m_EnemySatelliteContainer->end(); /* ++iter*/ ) {

		if (iter->HasShieldFailed()) {

			AddScore(&(*iter));

			NukeArea(iter->GetPos(),380);   // BIG BOOOOOOOOM

			AddPickUps(&(*iter),12);

			m_pSoundManager->PlayRandomExplodeSound();
			m_pSoundManager->PlayRandomBigExplodeSound();

			AddScalingAnim(HashString::RedEdgeExplosion64x64, 
				&(*iter),
				0.017f,									// frame speed
				0,										// Spin Amount
				35.00f,									// Top Scale
				0.001f,									// Start Scale
				0.015f );

			AddScalingAnim(HashString::YellowEdgeExplosion64x64, 
				&(*iter),
				0.025f,									// frame speed
				0,										// Spin Amount
				25.00f,									// Top Scale
				0.001f,									// Start Scale
				0.05f );

			m_pSoundManager->PlaySound( HashString::ExplodeWithSparks);

			*iter = m_EnemySatelliteContainer->back();  // Quick erase
			m_EnemySatelliteContainer->pop_back();

		} else {

			int count(0);
			for (std::vector<Vessel>::iterator Spore(m_SporesContainer->begin()); Spore!=m_SporesContainer->end(); ++Spore) {
				if (iter->GetID() == Spore->GetID()) {
					count++;
				}
			}
			if (count>=2) {
				if (iter->GetHitCoolDownTimer() > 30) 
					iter->AddHitCoolDownTimer(-1);
				else
					iter->AddHitCoolDownTimer(30);

			} else {
				iter->SetHitCoolDownTimer(0);

				for (std::vector<Vessel>::iterator Spore(m_SporesContainer->begin()); Spore!=m_SporesContainer->end(); ++Spore) {
					if (iter->GetID() == Spore->GetID()) {
						Spore->SetID(-1);
					}
				}
			}


			++iter;
		}
	}

}

void GameLogic::PlayerCollisionLogic()
{
	// Check to see if MyShip hits any Bad ships
	if ( GetPlrVessel()->IsShieldOk() )
	{
		float fReducePlayerShieldLevelByAmount = m_pWii->GetXmlVariable(HashString::ReducePlayerShieldLevelByAmountForShipToShipCollision);
		float fPlayerCollisionRadius = m_pWii->GetXmlVariable(HashString::PlayerCollisionRadius);
		fPlayerCollisionRadius*=fPlayerCollisionRadius;

		for (std::vector<Vessel>::iterator BadIter(m_SmallEnemiesContainer->begin()); BadIter!=m_SmallEnemiesContainer->end(); ++BadIter)
		{
			if ( GetPlrVessel()->InsideRadius(BadIter->GetX(), BadIter->GetY(), fPlayerCollisionRadius) ) {
				GetPlrVessel()->SetHitCoolDownTimer(10);
				GetPlrVessel()->AddShieldLevel (-fReducePlayerShieldLevelByAmount);
				// step back so we are outside the hit zone
				BadIter->SetVel( -BadIter->GetVelX() + GetPlrVessel()->GetVelX(), -BadIter->GetVelY() + GetPlrVessel()->GetVelY(), 0.0f );
				BadIter->AddVelToPos();
				BadIter->AddShieldLevel(-1);

				m_pSoundManager->PlayRandomHull();

			}
		}

		fPlayerCollisionRadius = 46 * 46;

		for (std::vector<Vessel>::iterator BadIter(m_GunShipContainer->begin()); BadIter!=m_GunShipContainer->end(); ++BadIter)
		{
			bool bOnce=false;
			while ( GetPlrVessel()->InsideRadius(BadIter->GetX(), BadIter->GetY(), fPlayerCollisionRadius) ) {
				if (!bOnce) 	{
					bOnce = true;
					GetPlrVessel()->SetVel(0,0,0);
					GetPlrVessel()->SetHitCoolDownTimer(128);
					// Sling our craft away from the thing it's just hit, in the line of direction from the two points 
					GetPlrVessel()->AddShieldLevel (-fReducePlayerShieldLevelByAmount);
					BadIter->AddShieldLevel(-1);
					float ang = atan2( GetPlrVessel()->GetX() - BadIter->GetX(),GetPlrVessel()->GetY() - BadIter->GetY() );
					float dx = sin( ang )*1.55f;
					float dy = cos( ang )*1.55f;
					GetPlrVessel()->AddVel( dx, dy, 0 );

					m_pSoundManager->PlayRandomHullCreak();

				}
				GetPlrVessel()->AddVelToPos();
			}
		}

		fPlayerCollisionRadius = 8 * 8;

		for (std::vector<Vessel>::iterator ProjectileIter(m_ProjectileContainer->begin()); ProjectileIter!=m_ProjectileContainer->end(); /*nop*/ )
		{
			if ( GetPlrVessel()->InsideRadius(ProjectileIter->GetX(), ProjectileIter->GetY(), fPlayerCollisionRadius ) )
			{

				GetPlrVessel()->AddShieldLevel( -(ProjectileIter->GetID()) );  // fudge, id used for damage amount!
				GetPlrVessel()->SetHitCoolDownTimer(128);

				Vessel Boom = *ProjectileIter;
				Boom.SetFrameGroup( m_pWii->GetFrameContainer(HashString::ExplosionSolidType32x32x10) ,0.25f);
				Boom.SetSpin( (1000 - (rand()%2000 )) * 0.00005f );
				m_ExplosionsContainer->push_back(Boom);	// projectile explosion

				AddScalingAnim(HashString::YellowEdgeExplosion64x64, 
					&(*ProjectileIter),
					0.05f, //frame speed
					( 800 - ( rand()%1600) ) * 0.00025f, // Spin Amount
					0.75f + (rand()%75)*0.01 , //Top Scale
					0.5f, // Start Scale
					0.20f ); // Scale Factor

				m_pSoundManager->PlaySound( HashString::Explode1,100,100);
				*ProjectileIter = m_ProjectileContainer->back();	// erase
				m_ProjectileContainer->pop_back();					//
			}
			else {
				++ProjectileIter;
			}
		}

		fPlayerCollisionRadius = 65 * 65;

		for (std::vector<Vessel>::iterator Iter(m_EnemySatelliteContainer->begin()); Iter!=m_EnemySatelliteContainer->end(); ++Iter )
		{
			int Count(0);
			for (std::vector<Vessel>::iterator Spore(m_SporesContainer->begin()); Spore!=m_SporesContainer->end(); ++Spore) {
				if (Spore->GetID() == Iter->GetID()) {
					Count++;
				}
			}

			if ( Count >=  m_pWii->GetXmlVariable(HashString::AmountSporesNeededForShiled )) {
				if ( GetPlrVessel()->InsideRadius(Iter->GetX(), Iter->GetY(), fPlayerCollisionRadius ) ){
					GetPlrVessel()->AddShieldLevel( -fReducePlayerShieldLevelByAmount );
					GetPlrVessel()->FactorVel(-1);
					GetPlrVessel()->AddVelToPos();
					GetPlrVessel()->FactorVel(0.5f);
					GetPlrVessel()->SetHitCoolDownTimer(128);

					m_pSoundManager->PlayRandomHullCreak();

				}
			}
		}
	}
}

void GameLogic::MissileLogic()
{
	m_pWii->profiler_start(&profile_Missile);
	for (std::vector<Vessel>::iterator MissileIter(m_MissileContainer->begin()); MissileIter!= m_MissileContainer->end(); /* NOP */)
	{
		if ( MissileIter->GetFuel() > 0 ) {
			float dir( MissileIter->GetFacingDirection() );
			float mx(sin( dir ) *  0.15f);
			float my(cos( dir ) * -0.15f);
			if ( MissileIter->GetFuel() > (60*9) ) 	{
				Vessel thrust( *MissileIter ); // create some thrust flames coming out the back end of the rocket
				thrust.AddPos( -mx*2.0f, -my*2.0f, 0  );
				thrust.AddVel( -mx*32.0f, -my*32.0f, 0); 
				thrust.SetGravityFactor(0.965f);
				AddAnim(HashString::ExplosionSmoke1Type16x16x10, &thrust,0.75f, (1000 - (rand()%2000 )) * 0.0001f );
			}
			MissileIter->AddVel( mx, my, 0);
			MissileIter->AddVelToPos();
			MissileIter->ReduceFuel();
			++MissileIter;
		}
		else {
			*MissileIter = m_MissileContainer->back();  // erase
			m_MissileContainer->pop_back();				//
		}
	}

	m_pWii->profiler_stop(&profile_Missile);
}

void GameLogic::ProbeMineCollisionLogic()
{
	for (std::vector<Vessel>::iterator ProbeMineIter(m_ProbeMineContainer->begin()); ProbeMineIter!= m_ProbeMineContainer->end(); /* ++ProbeMineIter*/ )
	{
		bool bNothingHit(true);
		vector<Vessel*>* p =  &CollisionContainer[ GetHash( &(*ProbeMineIter) )];
		for (std::vector<Vessel*>::iterator Iter2(p->begin()); Iter2!= p->end(); ++Iter2 )
		{
			Vessel* Iter = *Iter2;
			if ( !Iter->HasShieldFailed() )  
			{
				static const float CraftSize=32;
				if ( ProbeMineIter->InsideRadius(*Iter, Iter->GetRadius() ) )
				{
					static const float BlastRadius = 34;
					NukeArea(Iter->GetPos(), BlastRadius );

					AddScalingAnim(HashString::YellowEdgeExplosion64x64, 
						&(*ProbeMineIter),
						0.08f, //frame speed
						0,		// Spin Amount
						1.01f, //Top Scale
						0.0001f, // Start Scale
						0.14f ); // Scale Factor

					AddAnim(HashString::ExplosionSolidType32x32x10, &(*ProbeMineIter), 0.25f, 0 );

					// ---------------------------------------------
					*ProbeMineIter = m_ProbeMineContainer->back();  // erase
					m_ProbeMineContainer->pop_back();
					// ---------------------------------------------
					m_pSoundManager->PlaySound( HashString::Explode2,100,100 );

					bNothingHit=false;
					break;
				}
			}
		}
		if (bNothingHit)
			++ProbeMineIter;
	}
}

void GameLogic::RetrieveProbeMineLogic( float ThrustPower )
{
	Vessel* AnyTarget( GetPlrVessel() );   
	for (std::vector<Vessel>::iterator ProbeMineIter(m_ProbeMineContainer->begin()); ProbeMineIter!= m_ProbeMineContainer->end(); ++ProbeMineIter )
	{
		ProbeMineIter->SetFrame( ProbeMineIter->GetFrameStart() );   // reset frame - this frame value may get overwritten depending if thrust is used	
		if ( ProbeMineIter->InsideRadius(*AnyTarget, 64*64))
		{
			if ( (fabs(ProbeMineIter->GetVelX()<0.075f)) &&  (fabs(ProbeMineIter->GetVelY()<0.075f)) ) {
				ProbeMineIter->AddVel( (1000 - rand()%2000) * 0.000315f, (1000 - rand()%2000) * 0.000315f, 0.0f );
			}
			ProbeMineIter->SetGravityFactor(1.0f);

			if ( ProbeMineIter->GetFrameSpeed() == 0.0f ) {
				ProbeMineIter->SetFrameSpeed(0.05f);  // start anim - red flash
				ProbeMineIter->SetFrame( ProbeMineIter->GetFrameStart() );  // for anim logic look in GameDisplay::DisplayProbMines
			}
		}
		else
		{
			Vessel ProbeMineTail = *ProbeMineIter;
			ProbeMineTail.SetGravityFactor(0.975f);
			if (ProbeMineIter->GetY() > AnyTarget->GetY() ) {
				ProbeMineIter->AddVel(0, -ThrustPower, 0);
				ProbeMineIter->SetFrame( m_pWii->GetFrameContainer(HashString::ProbeMineDown16x16)->StartFrame ); // use Bottom thruster
				ProbeMineTail.AddVel(0.5 - ((rand()%100)*0.01),2.0 + ((rand()%300)*0.01),0);
				ProbeMineTail.SetFrameGroup( m_pWii->GetFrameContainer(HashString::ProbeMineDownThrust16x16x5), 0.25F+((rand()%50)*0.01F) );
			}
			else {
				ProbeMineIter->AddVel(0, ThrustPower, 0);
				ProbeMineIter->SetFrame( m_pWii->GetFrameContainer(HashString::ProbeMineUp16x16)->StartFrame ); // use Top thruster
				ProbeMineTail.AddVel(0.5 + ((rand()%100)*0.01),-2.0 - ((rand()%300)*0.01),0);
				ProbeMineTail.SetFrameGroup( m_pWii->GetFrameContainer(HashString::ProbeMineUpThrust16x16x5), 0.25F+((rand()%50)*0.01F) );
			}
			if (rand()%8==0) {
				m_ExhaustContainer->push_back(ProbeMineTail);	
			}

			// rocket tail
			if (ProbeMineIter->GetX() > AnyTarget->GetX() ) {
				ProbeMineIter->AddVel(-ThrustPower, 0, 0);
				ProbeMineIter->SetFrame(  m_pWii->GetFrameContainer(HashString::ProbeMineRight16x16)->StartFrame ); // use Right thruster
				ProbeMineTail.AddVel(2.0 + ((rand()%300)*0.01),0.5 - ((rand()%100)*0.01),0);
				ProbeMineTail.SetFrameGroup( m_pWii->GetFrameContainer(HashString::ProbeMineRightThrust16x16x5), 0.25F+((rand()%50)*0.01F) );
			}
			else {
				ProbeMineIter->AddVel(ThrustPower, 0, 0);
				ProbeMineIter->SetFrame(  m_pWii->GetFrameContainer(HashString::ProbeMineLeft16x16)->StartFrame ); // use Left thruster
				ProbeMineTail.AddVel(-2.0 - ((rand()%300)*0.01),0.5 - ((rand()%100)*0.01),0);
				ProbeMineTail.SetFrameGroup(m_pWii->GetFrameContainer(HashString::ProbeMineLeftThrust16x16x5), 0.25F+((rand()%50)*0.01F) );
			}
			if (rand()%8==0) {
				m_ExhaustContainer->push_back(ProbeMineTail);	
			}
			ProbeMineIter->SetGravityFactor(0.991f);
			ProbeMineIter->SetFrameSpeed(0.0f); // stop anim - locking on
		}

		ProbeMineIter->FactorVel();
		ProbeMineIter->AddVelToPos();
	}
}

map< int, vector<Vessel*> > conainerX;
map< int, vector<Vessel*> > conainerY;

void GameLogic::ProbeMineLogic(std::vector<Vessel>*  pVesselContainer, 
							   float ThrustPower,  float ScanRange , float FrameSpeed)
{
	m_pWii->profiler_start(&profile_ProbeMineLogic);

	static const int SPATIALGRID(64);

	conainerX.clear();
	conainerY.clear();

	// first work out what is laying in the 'X'path and then 'Y' path
	for (std::vector<Vessel>::iterator Iter(pVesselContainer->begin()); Iter!= pVesselContainer->end(); ++Iter ){
		if ( !Iter->HasShieldFailed() ) {
			conainerX[Iter->GetX()/SPATIALGRID].push_back(&(*Iter));
			conainerY[Iter->GetY()/SPATIALGRID].push_back(&(*Iter));
			// I don't care about loosing spatial accuracy, e.g. edge cases (1/2 of vessle will be ignored at times)
		}
	}

	//	 Logic for Probe Mines - checks against bad ships in spatial container
	for (std::vector<Vessel>::iterator ProbeMineIter(m_ProbeMineContainer->begin()); ProbeMineIter!= m_ProbeMineContainer->end(); ++ProbeMineIter )
	{
		bool HaveLockOnSoForgetTheRest(false); 
		map< int, vector<Vessel*> >::iterator iter = conainerX.find(ProbeMineIter->GetX()/SPATIALGRID);
		if (iter!=conainerX.end()) {
			for (std::vector<Vessel*>::iterator TargetIter(iter->second.begin() ); TargetIter!=(iter->second.end() ); ++TargetIter)
			{
				if (!HaveLockOnSoForgetTheRest)
				{
					Vessel ProbeMineTail = *ProbeMineIter;
					if (ProbeMineIter->GetY() > (*TargetIter)->GetY() ) {
						ProbeMineIter->AddVel(0, -ThrustPower, 0);
						ProbeMineIter->SetFrame( m_pWii->GetFrameContainer(HashString::ProbeMineDown16x16)->StartFrame  ); // use Bottom thruster
						ProbeMineTail.AddVel(0.5 - ((rand()%100)*0.01),2.0 + ((rand()%300)*0.01),0);
						ProbeMineTail.SetFrameGroup( m_pWii->GetFrameContainer(HashString::ProbeMineDownThrust16x16x5), 0.25F+((rand()%50)*0.01F) );
					}
					else {
						ProbeMineIter->AddVel(0, ThrustPower, 0);
						ProbeMineIter->SetFrame( m_pWii->GetFrameContainer(HashString::ProbeMineUp16x16)->StartFrame ); // use Top thruster
						ProbeMineTail.AddVel(0.5 + ((rand()%100)*0.01),-2.0 - ((rand()%300)*0.01),0);
						ProbeMineTail.SetFrameGroup( m_pWii->GetFrameContainer(HashString::ProbeMineUpThrust16x16x5), 0.25F+((rand()%50)*0.01F) );
					}
					if (rand()%8==0) {
						ProbeMineTail.SetGravityFactor(0.975f);
						m_ExhaustContainer->push_back(ProbeMineTail);	
					}
					HaveLockOnSoForgetTheRest = true; 

					//conainerX[ProbeMineIter->GetX()/SPATIALGRID].erase(TargetIter);
					break;
				}
			}
		}

		bool HaveLockOnSoForgetTheRestY(false); 
		iter = conainerY.find(ProbeMineIter->GetY()/SPATIALGRID);
		if (iter!=conainerY.end()) {
			for (std::vector<Vessel*>::iterator TargetIter(iter->second.begin() ); TargetIter!=(iter->second.end() ); ++TargetIter)
			{
				if (!HaveLockOnSoForgetTheRestY) {
					//  tail
					Vessel ProbeMineTail = *ProbeMineIter;
					if (ProbeMineIter->GetX() > (*TargetIter)->GetX() ) {
						ProbeMineIter->AddVel(-ThrustPower, 0, 0);
						ProbeMineIter->SetFrame(m_pWii->GetFrameContainer(HashString::ProbeMineRight16x16)->StartFrame ); // use Right thruster
						ProbeMineTail.AddVel(2.0 + ((rand()%300)*0.01),0.5 - ((rand()%100)*0.01),0);
						ProbeMineTail.SetFrameGroup( m_pWii->GetFrameContainer(HashString::ProbeMineRightThrust16x16x5), 0.25F+((rand()%50)*0.01F) );
					}
					else {
						ProbeMineIter->AddVel(ThrustPower, 0, 0);
						ProbeMineIter->SetFrame( m_pWii->GetFrameContainer(HashString::ProbeMineLeft16x16)->StartFrame ); // use Left thruster
						ProbeMineTail.AddVel(-2.0 - ((rand()%300)*0.01),0.5 - ((rand()%100)*0.01),0);
						ProbeMineTail.SetFrameGroup( m_pWii->GetFrameContainer(HashString::ProbeMineLeftThrust16x16x5), 0.25F+((rand()%50)*0.01F) );
					}
					if (rand()%8==0) {
						ProbeMineTail.SetGravityFactor(0.975f);
						m_ExhaustContainer->push_back(ProbeMineTail);	
					}
					HaveLockOnSoForgetTheRestY = true;

					//conainerY[ProbeMineIter->GetY()/SPATIALGRID].erase(TargetIter);
					break;
				}
			}
		}

		if (HaveLockOnSoForgetTheRest || HaveLockOnSoForgetTheRestY) {
			ProbeMineIter->SetFrameSpeed(0.0f); // stop locking on anim
		}
		else {
			if ( ProbeMineIter->GetFrameSpeed() == 0.0f ) {
				ProbeMineIter->SetFrameSpeed(FrameSpeed);  // start anim - red flash
				ProbeMineIter->SetFrame( ProbeMineIter->GetFrameStart() );  // for anim logic look in GameDisplay::DisplayProbMines
			}
		}

		ProbeMineIter->SetGravityFactor(0.991f);
		ProbeMineIter->FactorVel();
		ProbeMineIter->AddVelToPos();

	}

	m_pWii->profiler_stop(&profile_ProbeMineLogic);
}

void GameLogic::GunShipLogic()
{
	m_pWii->profiler_start(&profile_GunShip);

	guVector ChaseTarget( m_CPUTarget_Miss ); 

	for (std::vector<Vessel>::iterator GunShipIter( m_GunShipContainer->begin()); GunShipIter!= m_GunShipContainer->end(); ++GunShipIter)
	{
		f32 Turn = GunShipIter->GetTurnDirection( &ChaseTarget );
		GunShipIter->AddFacingDirection( Turn * 0.01f );

		//if ( GunShipIter->ReduceFireRate() < 15) {
		Turn = GunShipIter->GetTurnDirectionForTurret( &m_CPUTarget );
		GunShipIter->AddTurrentDirection( Turn * 0.15f);
		//	}


		ChaseTarget = GunShipIter->GetPos();  // the next ship will now follow this one, and so on in the sequence.

		// gun ship firing		
		//if ( GunShipIter->GetFireRate() <= 0) {
		if ( GunShipIter->ReduceFireRate() <= 0) {
			GunShipIter->SetFireRate( 20 + rand()%120 );  // top up to fire again later

			float XToCheck(GunShipIter->GetX() - m_CPUTarget.x );
			float YToCheck(GunShipIter->GetY() - m_CPUTarget.y );
			float dist ( (XToCheck * XToCheck) + (YToCheck * YToCheck) );
			int Vol =  max( 0, (int) ( 128.0f - ((128.0f / 800000.0f) * dist) ) );
			//printf( "value = %d", Vol );
			m_pSoundManager->PlayRandomGunFire(Vol);

			Vessel Projectile = *GunShipIter;
			Projectile.SetSpeedFactor( 1.0f );
			Projectile.SetFrameGroup(m_pWii->GetFrameContainer(HashString::GunShipProjectileFrames),0.005f);
			Projectile.SetGravityFactor(1.0f);
			Projectile.SetSpin( 0.25f );

			// rotate around ship origin first
			Projectile.SetPos(
				Projectile.GetX() - ( sin(GunShipIter->GetFacingDirection()) * (m_pWii->GetXmlVariable(HashString::TurretNo2ForGunShipOriginX)) ) , 
				Projectile.GetY() - ( -cos(GunShipIter->GetFacingDirection()) * m_pWii->GetXmlVariable(HashString::TurretForGunShipOriginY) ) , 
				Projectile.GetZ()  );

			// first use direction relative to itself
			float dir = GunShipIter->GetTurrentDirection();
			float mx(sin( dir )* 32.0f);
			float my(cos( dir )* 32.0f);
			Projectile.AddPos(mx * 0.45f, -my * 0.45f, 0);	

			Projectile.SetID(2);
			Vessel Keep = Projectile;
			// TWO turrets per ship
			for (int i=0; i<2 ;i++) {
				// picks a gun port to fire from
				if ( i==0 ) {	
					Projectile.SetPos( 
						Projectile.GetX() - ( -sin(GunShipIter->GetFacingDirection()+(M_PI/2)) * (m_pWii->GetXmlVariable(HashString::TurretNo1ForGunShipOriginX)) ) , 
						Projectile.GetY() - ( -cos(GunShipIter->GetFacingDirection()+(M_PI/2)) * m_pWii->GetXmlVariable(HashString::TurretForGunShipOriginY) ) , 
						Projectile.GetZ()  );
				}
				else {
					Projectile = Keep;

					Projectile.SetPos( 
						Projectile.GetX() - ( sin(GunShipIter->GetFacingDirection()+(M_PI+(M_PI/2))) * (m_pWii->GetXmlVariable(HashString::TurretNo2ForGunShipOriginX)) ) , 
						Projectile.GetY() - ( -cos(GunShipIter->GetFacingDirection()+(M_PI+(M_PI/2))) * m_pWii->GetXmlVariable(HashString::TurretForGunShipOriginY) ) , 
						Projectile.GetZ()  );
				}	

				// velocity of each projectile
				f32 TurretLockOn = atan2( m_CPUTarget.x - Projectile.GetX(), Projectile.GetY() - m_CPUTarget.y  );
				float temp_mx(sin( TurretLockOn )* 3.6f);
				float temp_my(cos( TurretLockOn )* 3.6f);
				//Projectile.SetVel( temp_mx * GunShipIter->GetBulletSpeedFactor(), -temp_my * GunShipIter->GetBulletSpeedFactor(), 0);  
				Projectile.SetVel( temp_mx , -temp_my , 0);  
				m_ProjectileContainer->push_back(Projectile);	

				// barrel effect
				Projectile.SetFrameGroup( m_pWii->GetFrameContainer(HashString::SmokeTrail16x16x10), 0.8f );
				Projectile.FactorVel(1.65f);
				m_ExplosionsContainer->push_back(Projectile);	
			}
		}

		if (GunShipIter->GetSpeedFactor() > 0) {
			GunShipIter->SetVel( sin(GunShipIter->GetFacingDirection())*1.75, -cos(GunShipIter->GetFacingDirection())*1.75,0);
			GunShipIter->AddVelToPos();

			if (m_pWii->GetFrameCounter()&4)
			{
				Vessel Boom = *GunShipIter;
				Boom.SetFrameGroup( m_pWii->GetFrameContainer(HashString::SmokeTrail16x16x10), 0.5f );
				Boom.SetGravityFactor(0.995f);
				float dir = Boom.GetFacingDirection() + (5-rand()%11)*0.01;
				float mx(sin( dir )* -32.0f);
				float my(cos( dir )* -32.0f);
				Boom.SetPos( Boom.GetX() + mx, Boom.GetY() - my, Boom.GetZ()  );
				Boom.SetSpin( (1000 - (rand()%2000 )) * 0.00025f );
				Boom.AddVel( mx * 0.1f, -my *0.1f, 0);  // engine trail
				m_ExhaustContainer->push_back(Boom);	
			}
		}
	}

	m_pWii->profiler_stop(&profile_GunShip);
}


void GameLogic::BadShipsLogicForIntro()
{
	m_pWii->profiler_start(&profile_SmallEnemies);

	for (std::vector<Vessel>::iterator BadIter(m_SmallEnemiesContainer->begin()); BadIter!= m_SmallEnemiesContainer->end(); ++BadIter )
	{
		f32 Turn = BadIter->GetTurnDirection( &m_CPUTarget );
		BadIter->AddFacingDirection( Turn * 0.025f );

		if (rand()%5==0) 
		{
			float dir = BadIter->GetFacingDirection();
			float mx( sin( dir )*0.35 );
			float my( -cos( dir )*0.35 );
			BadIter->AddVel( mx, my, 0 );

			// add thrust trail
			Vessel Tail( *BadIter );  // copy the original's detials
			Tail.SetFrameGroup( m_pWii->GetFrameContainer(HashString::ExplosionSmoke2Type16x16x10),0.5f);
			Tail.AddPos( -mx*16.0 ,-my*16.0,	0);
			Tail.AddVel( -mx*10.2 ,-my*10.2,	0);
			Tail.SetGravityFactor(0.9825f);
			m_ExhaustContainer->push_back(Tail);  //1st puff of smoke
		}
		BadIter->FactorVel();
		BadIter->AddVelToPos(); 
	}

	m_pWii->profiler_stop(&profile_SmallEnemies);
}

void GameLogic::BadShipsLogic()
{
	m_pWii->profiler_start(&profile_SmallEnemies);

	float keep = FakLockOn_AngInc;

	for (std::vector<Vessel>::iterator BadIter(m_SmallEnemiesContainer->begin()); BadIter!= m_SmallEnemiesContainer->end(); ++BadIter )
	{

		//
		// experiment - looks ok
		//
		FakLockOn_AngInc+=0.15f;
		m_CPUTarget_Miss.x = GetPlrVessel()->GetX() + sin(FakLockOn_AngInc)*170.0f;
		m_CPUTarget_Miss.y = GetPlrVessel()->GetY() + cos(FakLockOn_AngInc)*170.0f;
		//
		f32 Turn = BadIter->GetTurnDirection( &m_CPUTarget_Miss );
		BadIter->AddFacingDirection( Turn * 0.025f );

		if (rand()%3==0) 
		{
			float dir = BadIter->GetFacingDirection();
			float mx( sin( dir )*0.35 );
			float my( -cos( dir )*0.35 );
			BadIter->AddVel( mx, my, 0 );

			Vessel Tail( *BadIter );  // copy the original's detials

			// add thrust trail
			Tail.SetFrameGroup( m_pWii->GetFrameContainer(HashString::ExplosionSmoke2Type16x16x10),0.5f);
			Tail.AddPos( -mx*16.0 ,-my*16.0,	0);
			Tail.AddVel( -mx*10.2 ,-my*10.2,	0);
			Tail.SetGravityFactor(0.9825f);
			m_ExhaustContainer->push_back(Tail);  //1st puff of smoke
		}

		if ( BadIter->ReduceFireRate() <= 0) {
			BadIter->SetFireRate(rand()%300 + (3*60) );

			Vessel Projectile =  *BadIter;
			Projectile.SetID(1);
			Projectile.SetSpeedFactor( 1.0f );
			Projectile.SetFrameGroup(m_pWii->GetFrameContainer(HashString::SmallShot16x16),0.0325f);
			Projectile.SetGravityFactor(1.0f);
			Projectile.SetSpin( 0.0f );
			//Projectile.FactorVel(1.65f);
			f32 TurretLockOn = atan2( m_CPUTarget.x - Projectile.GetX(), Projectile.GetY() - m_CPUTarget.y  );
			Projectile.SetFacingDirection(TurretLockOn);
			float temp_mx(sin( TurretLockOn )* 1.95f);
			float temp_my(cos( TurretLockOn )* 1.95f);
			//Projectile.SetVel( temp_mx * Projectile.GetBulletSpeedFactor(), -temp_my * Projectile.GetBulletSpeedFactor(), 0);  
			Projectile.SetVel( temp_mx, -temp_my, 0);  
			m_ProjectileContainer->push_back(Projectile);	
		}

		BadIter->FactorVel();
		BadIter->AddVelToPos(); 

	}

	FakLockOn_AngInc = keep;

	m_pWii->profiler_stop(&profile_SmallEnemies);
}

void GameLogic::DyingShipsLogic()
{
	m_pWii->profiler_start(&profile_DyingEnemies);

	for (std::vector<Vessel>::iterator BadIter(m_DyingEnemiesContainer->begin());
		BadIter!= m_DyingEnemiesContainer->end(); /* ++BadIter */ )
	{
		float spin = BadIter->GetSpin();
		BadIter->AddFacingDirection( spin );
		BadIter->SetSpin(spin*0.99975f);
		BadIter->SetAlpha( max( 0, 255 - ((int)BadIter->GetZ()/2) ) ); // fudge
		BadIter->FactorVel();
		BadIter->AddVelToPos();  

		if (BadIter->GetFuel()>0) {					
			if ( ( ( BadIter->GetFuel()&0x01 ) ==1) && ( (rand()%2) == 0) ) 	{
				Vessel Boom = *BadIter;
				Boom.SetVel(0,0,0);
				Boom.SetFrameGroup(m_pWii->GetFrameContainer(HashString::ExplosionFire2Type16x16x9),0.15f);
				Boom.SetSpin( (1000 - (rand()%2000 )) * 0.000075f );
				Boom.SetAlpha( max( 0, 255 - (int)(Boom.GetZ() * 1.5f )) ); // fudge
				m_ExhaustContainer->push_back(Boom);
			}
			BadIter->ReduceFuel();
			++BadIter;
		}
		else {
			*BadIter = m_DyingEnemiesContainer->back();		// erase
			m_DyingEnemiesContainer->pop_back();			//
		}
	}
	m_pWii->profiler_stop(&profile_DyingEnemies);

}

void GameLogic::AddPickUps(Vessel* Position, int Amount)
{
	Item3D PickUp;

	PickUp.SetPos( Position->GetX(), Position->GetY(), Position->GetZ());
	PickUp.SetScale( 0.5f, 0.5f, 0.5f);

	for (int i=0; i<Amount; ++i)
	{
		const static float r = 2.34f; 
		float ang = (float)i * ((M_PI*2.0f) / (float)Amount);
		float mx = sin(ang)*(r) ;
		float my = cos(ang)*(r) ;

		if (Amount>1)
			PickUp.SetVel(mx+Position->GetVelX(),my+Position->GetVelY(),0);
		else
			PickUp.SetVel(Position->GetVelX()/2,Position->GetVelY()/2,0);

		PickUp.SetRotateAmount(	0,0.005f,0.015f);
		m_pMaterialPickUpContainer->push_back(PickUp);
	}
}

void GameLogic::PickUpsLogic()
{
	Vessel* pPlayerShip = GetPlrVessel();
	for (std::vector<Item3D>::iterator iter(m_pMaterialPickUpContainer->begin()); iter!= m_pMaterialPickUpContainer->end(); /*nop*/ )
	{
		iter->MulScale(0.995f);
		iter->Rotate();

		if (iter->GetScaleX()<0.45f) {
			if (iter->GetScaleX()<0.05f) {
				*iter = m_pMaterialPickUpContainer->back();		// erase
				m_pMaterialPickUpContainer->pop_back();			//
				continue;
			}
			if (pPlayerShip->InsideRadius(iter->GetX(), iter->GetY(),200*200)) {
				float mx( pPlayerShip->GetX() - iter->GetX() );
				float my( pPlayerShip->GetY() - iter->GetY() );
				float dir( atan2( mx,my ) );
				float dx = sin( dir )*0.085f;
				float dy = cos( dir )*0.085f;
				iter->AddVel( dx, dy, 0 );

				if (pPlayerShip->InsideRadius(iter->GetX(), iter->GetY(),30*30)) {
					if ( GetPlrVessel()->IsShieldOk() ) {
						*iter = m_pMaterialPickUpContainer->back();		// erase
						m_pMaterialPickUpContainer->pop_back();			//	
						GetPlrVessel()->AddToPickUpTotal(iter->GetScaleX()*10.0f);	
						m_pSoundManager->PlaySound( HashString::Metal_pickup,112,112);
						continue;
					}
				}
			}
		}
		iter->AddVelToPos();
		iter->ReduceVel(0.99f);
		iter++;
	}
}

void GameLogic::AddHealthPickUps(Vessel* Position)
{
	Vessel PickUp(*Position);
	PickUp.SetGravityFactor(0.9965f);
	PickUp.SetZ(0.0f);
	PickUp.SetFrameGroup(m_pWii->GetFrameContainer(HashString::HealthPickUp32x32), 1);

	float r(rand()%2000);
	if ( r > 0.0f )
		PickUp.SetSpin( (r * 0.000075f) + 0.01f);
	else
		PickUp.SetSpin( (r * 0.000075f) - 0.01f);


	m_pHealthPickUpContainer->push_back(PickUp);
}

void GameLogic::HealthPickUpsLogic()
{
	Vessel* pPlayerShip = GetPlrVessel();
	for (std::vector<Vessel>::iterator iter(m_pHealthPickUpContainer->begin());
		iter!= m_pHealthPickUpContainer->end(); /*nop*/ )
	{
		iter->AddVelToPos();
		iter->FactorVel();
		iter->AddFacingDirection(iter->GetSpin());

		float spin( iter->GetSpin() );
		iter->SetSpin( spin * 0.9975f );
		if (spin < 0.001f) {
			*iter = m_pHealthPickUpContainer->back();   // erase(iter)
			m_pHealthPickUpContainer->pop_back();		//
			continue;
		}
		if (spin < 0.008f) {
			iter->SetAlpha(128);
			iter->SetVelZ(9.0f);
		}

		if (pPlayerShip->InsideRadius(*iter,30*30))  {
			if ( GetPlrVessel()->IsShieldOk() ) {
				*iter = m_pHealthPickUpContainer->back();	// erase(iter)
				m_pHealthPickUpContainer->pop_back();		//
				pPlayerShip->AddShieldLevel(8);	
				m_pSoundManager->PlaySound( HashString::PickUphealth,222,222);
				continue;
			}
		}
		iter++;
	}
}

void GameLogic::ClearBadContainer()
{ 
	m_SmallEnemiesContainer->clear(); 
}

// todo ... needs to support extra player modes, just one at the mo
void GameLogic::AddScore(Vessel* pVessel) 
{
	if ( GetPlrVessel()->IsShieldOk() ) { // only add score if players ship is still OK
		m_Score += pVessel->GetKillValue() ; 
		AddScorePing( pVessel );
	}
}

void GameLogic::AddScorePing(Vessel* pVessel)
{
	ScorePingVessel item(  Util::NumberToString(pVessel->GetKillValue())  );
	item.SetPos( pVessel->GetPos() );
	item.SetVel( (1000.0f-(rand()%2000)) * 0.0028 ,-(1000.0f-(rand()%2000)) * 0.0028,0 );
	item.SetGravityFactor(0.9925f);
	item.m_ReduceAlphaPerFrame = 3;
	item.SetAlpha(255);
	m_pScorePingContainer->push_back( item );
}

vector<Item3DChronometry>::iterator GameLogic::EraseItemFromShieldGeneratorContainer(vector<Item3DChronometry>::iterator iter)
{
	return m_ShieldGeneratorContainer->erase(iter);
}


Vessel* GameLogic::GetGunTurretTarget(TurretItem3D* pTurret)
{
	if ( m_GunShipContainer->empty() && m_SmallEnemiesContainer->empty() )
		return NULL;

	if (pTurret->GetLockOntoVesselType() == HashString::TurretTarget_SmallShip)
	{
		if (m_SmallEnemiesContainer->empty()) 
			pTurret->SetLockOntoVesselIndex( -1 );
	}
	else
	{
		if (m_GunShipContainer->empty()) 
			pTurret->SetLockOntoVesselIndex( -1 );
	}

	if (pTurret->GetLockOntoVesselIndex()==-1)
	{
		int r = rand()%2;
		if (m_GunShipContainer->empty()) 
			r=0;
		if (m_SmallEnemiesContainer->empty()) 
			r=1;

		if (r==0)
		{
			pTurret->SetLockOntoVesselIndex(rand()%m_SmallEnemiesContainer->size());
			pTurret->SetLockOntoVesselType(HashString::TurretTarget_SmallShip);
		}
		else
		{
			pTurret->SetLockOntoVesselIndex(rand()%m_GunShipContainer->size());
			pTurret->SetLockOntoVesselType(HashString::TurretTarget_GunShip);
		}
	}

	std::vector<Vessel>* pEnemy;
	if (pTurret->GetLockOntoVesselType() == HashString::TurretTarget_SmallShip)
		pEnemy = m_SmallEnemiesContainer;
	else
		pEnemy = m_GunShipContainer;

	std::vector<Vessel>::iterator TargetIter( pEnemy->begin() );
	advance( TargetIter, pTurret->GetLockOntoVesselIndex() );

	if (TargetIter == pEnemy->end())
	{
		TargetIter = pEnemy->begin();
	}

	return &(*TargetIter);
}

void GameLogic::GunTurretLogic()
{
	VectorOfPointXYZ Points = m_pWii->GetRender3D()->GetModelPointsFromLayer("SmallGunTurret[BONE]"); // layer 3 has the needed bone points
	for (std::vector<TurretItem3D>::iterator iter(m_pGunTurretContainer->begin()); iter!= m_pGunTurretContainer->end(); ++iter)
	{
		Vessel* pTarget( GetGunTurretTarget( &(*iter) ) );

		static float LockOnSpeed( 0.25f );
		static float CurrentLockOnSpeed( 0.085f ); // Damps the first set - reduces seeing a jumpy 3d object 
		if (pTarget==NULL)
		{
			iter->GetCurrentTarget().x += ( 0 - iter->GetCurrentTarget().x ) * CurrentLockOnSpeed;
			iter->GetCurrentTarget().y += ( 0 - iter->GetCurrentTarget().y ) * CurrentLockOnSpeed;
			iter->GetCurrentTarget().z += ( 0 - iter->GetCurrentTarget().z ) * CurrentLockOnSpeed;
			continue;		
		}

		// Gets the distance squared - no need to use sqrt here
		float Shot_dist (	(pTarget->GetX() - iter->GetX()) * (pTarget->GetX() - iter->GetX())  + 
			(pTarget->GetY() - iter->GetY()) * (pTarget->GetY() - iter->GetY())  +
			(pTarget->GetZ() - iter->GetZ()) * (pTarget->GetZ() - iter->GetZ())  );

		static const float ShotSpeed (9.0f);

		guVector LockOnto(pTarget->GetPos() );
		float dist ;
		{
			guVector ShotVelocity;
			guVecSub(&(iter->GetPos()),&(pTarget->GetPos()),&ShotVelocity); // get vec between end of gun and ship
			guVecNormalize(&ShotVelocity);
			guVecScale(&ShotVelocity, &ShotVelocity, ShotSpeed );

			dist =  ( (ShotVelocity.x * ShotVelocity.x) + 
				(ShotVelocity.y * ShotVelocity.y) + (ShotVelocity.z * ShotVelocity.z)  );
			float div = sqrt(Shot_dist / dist);

			LockOnto.x += pTarget->GetVelX() * div;
			LockOnto.y += pTarget->GetVelY() * div;
		}

		// use this for lock on
		iter->GetWorkingTarget().x += ( LockOnto.x - iter->GetWorkingTarget().x ) * LockOnSpeed;
		iter->GetWorkingTarget().y += ( LockOnto.y - iter->GetWorkingTarget().y ) * LockOnSpeed;
		iter->GetWorkingTarget().z += ( LockOnto.z - iter->GetWorkingTarget().z ) * LockOnSpeed;

		// use this for drawing angles
		iter->GetCurrentTarget().x += ( iter->GetWorkingTarget().x - iter->GetCurrentTarget().x ) * CurrentLockOnSpeed;
		iter->GetCurrentTarget().y += ( iter->GetWorkingTarget().y - iter->GetCurrentTarget().y ) * CurrentLockOnSpeed;
		iter->GetCurrentTarget().z += ( iter->GetWorkingTarget().z - iter->GetCurrentTarget().z ) * CurrentLockOnSpeed;

		// Get Direction Vector for the Turret
		guVector DirectionVector; 
		DirectionVector.x = (iter->GetCurrentTarget().x  - iter->GetX() ); 
		DirectionVector.y = (iter->GetCurrentTarget().y  - iter->GetY() );
		DirectionVector.z = (iter->GetCurrentTarget().z  - iter->GetZ() );
		guVecNormalize(&DirectionVector);

		float Roll  = (M_PI) + atan2( DirectionVector.y, sqrt( DirectionVector.x * DirectionVector.x + DirectionVector.z * DirectionVector.z) ); 
		float Pitch = -(M_PI/2) + atan2( DirectionVector.x,  DirectionVector.z); 
		iter->SetRotate( 0, Pitch, Roll );

		if ( iter->IsTimerDone()  )
		{
			iter->SetTimerMillisecs( (rand()%1000) + 500 );

			Mtx Model,mat,mat2;
			guMtxRotRad(mat,'y', Pitch );
			guMtxRotRad(mat2,'z', Roll );
			guMtxConcat(mat,mat2,Model);

			// use bone points for both turret barrels
			//for (VectorOfPointXYZ::iterator BoneIter(Points.begin()); BoneIter!= Points.end(); ++BoneIter)
			VectorOfPointXYZ::iterator BoneIter(Points.begin());
			{
				Item3D pShot = *iter;	// need a full copy

				pShot.SetPos(BoneIter->Getx(),BoneIter->Gety(), BoneIter->Getz());
				// Rotate Bone point to that of the models world 
				guVecMultiply(Model, &(pShot.GetPos()), &(pShot.GetPos()))	;
				// Place it in the world
				pShot.AddPos(iter->GetX(),iter->GetY(), iter->GetZ()); 


				// angle from bone point
				guVector ShotVelocity; 
				guVecSub(&LockOnto,&(pShot.GetPos()),&ShotVelocity); // get vec between end of gun and ship
				//	guVecNormalize(&ShotVelocity);

				float ShipDistPredict (	
					(pTarget->GetX() - iter->GetWorkingTarget().x) * (pTarget->GetX() - iter->GetWorkingTarget().x)  + 
					(pTarget->GetY() - iter->GetWorkingTarget().y) * (pTarget->GetY() - iter->GetWorkingTarget().y)  +
					(pTarget->GetZ() - iter->GetWorkingTarget().z) * (pTarget->GetZ() - iter->GetWorkingTarget().z)  );

				float OneVesselStep =  ( (pTarget->GetVelX() * pTarget->GetVelX()) + (pTarget->GetVelY() * pTarget->GetVelY()) );

				float step = sqrt(OneVesselStep / (ShipDistPredict) );
				guVecScale(&ShotVelocity, &ShotVelocity, step * 0.5f );  // no idea why 1/2 makes it work!!!!
				pShot.SetVel(ShotVelocity);

				pShot.SetRotateAmount(-0.1f,0,-0.075f);

				pShot.InitTimer();
				pShot.SetTimerMillisecs( 8500 );
				m_ShotForGunTurretContainer->push_back(pShot); // fire shot

				// barrel effect
				Vessel Boom(&pShot, 0.82f);
				Boom.SetFrameGroup( m_pWii->GetFrameContainer(HashString::SmokeTrail16x16x10), 0.5f );
				m_ExhaustContainer->push_back(Boom);	
			}
		}
	}
}

void GameLogic::GunTurretShotsLogic( std::vector<Vessel>* pEnemy )
{
	for (std::vector<Item3D>::iterator iter(m_ShotForGunTurretContainer->begin()); iter!= m_ShotForGunTurretContainer->end(); /*NOP*/  ) 
	{
		if ( iter->IsTimerDone() ) {
			*iter = m_ShotForGunTurretContainer->back();   // remove shot from list i.e optimised erase
			m_ShotForGunTurretContainer->pop_back();
			continue;
		}

		bool hit=false;
		iter->AddVelToPos();
		iter->Rotate();

		for (std::vector<Vessel>::iterator IterEnemy(pEnemy->begin()); IterEnemy!=pEnemy->end(); ++IterEnemy) {

			if ( iter->InsideRadius(IterEnemy->GetX(), IterEnemy->GetY(), IterEnemy->GetRadius() ) ) {

				AddScalingAnim(HashString::RedEdgeExplosion64x64, 
					&(*IterEnemy),
					0.015f,									// frame speed
					( 800 - ( rand()%1600) ) * 0.00025f,	// Spin Amount
					2.45f + ((rand()%25)*0.01f),			// Top Scale
					0.001f,									// Start Scale
					0.02f );								// Scale Factor

				AddScalingAnim(HashString::FireBallExplosion64x64, 
					&(*IterEnemy),
					0.022f,									// frame speed
					( 800 - ( rand()%1600) ) * 0.00025f,	// Spin Amount
					1.85f + ((rand()%25)*0.01f),			// Top Scale
					0.001f,									// Start Scale
					0.022f );								// Scale Factor

				IterEnemy->AddShieldLevel(-1);
				IterEnemy->AddVel(iter->GetVelX()*0.5f,iter->GetVelY()*0.5f,0);
				*iter = m_ShotForGunTurretContainer->back();  // remove shot from list i.e. erase(iter)
				m_ShotForGunTurretContainer->pop_back();
				hit=true;
				break;
			}
		}

		if (hit==false) {
			++iter;
		}
	}
}

void GameLogic::MoonRocksLogic( )
{
	for (std::vector<Item3D>::iterator iter(m_pMoonRocksContainer->begin()); iter!= m_pMoonRocksContainer->end(); ++iter) {
		iter->Rotate(); 
	}
}

void GameLogic::ScorePingLogic()
{
	for (std::vector<ScorePingVessel>::iterator Iter(GetScorePingContainerBegin()); Iter!= GetScorePingContainerEnd(); /*NOP*/)
	{	
		if ( Iter->GetAlpha() < Iter->m_ReduceAlphaPerFrame )
		{
			*Iter = m_pScorePingContainer->back();  
			m_pScorePingContainer->pop_back();
			continue;
		}

		Iter->AddVelToPos(); 
		Iter->AddAlpha( -Iter->m_ReduceAlphaPerFrame );

		Iter->FactorVel(); 
		++Iter;
	}
}

//
// AddEnemy section
//
void GameLogic::AddEnemy(float x, float y, HashLabel ShipType)
{
	AddEnemy( x,y, 0,1, ShipType);
}

void GameLogic::AddEnemy(int OriginX, int OriginY, int Amount, HashLabel ShipType, float Radius)
{
	for (int i=0; i<Amount; ++i){
		float ang = (float)i * ((M_PI*2) / (float)Amount);
		float addx = sin(ang)*(Radius);
		float addy = cos(ang)*(Radius); 
		AddEnemy(OriginX + addx, OriginY + addy, 0, 1 , ShipType);
	}
}

void GameLogic::AddEnemy(float x, float y, float Velx, float Vely, HashLabel ShipType)
{
	Vessel BadVessel;
	float dir( atan2(Velx, Vely) );
	BadVessel.SetFacingDirection( dir );
	BadVessel.SetPos( x , y,  0  );
	BadVessel.SetVel(Velx, Vely, 0);
	BadVessel.SetFrameGroup(m_pWii->GetFrameContainer(ShipType),1);
	BadVessel.SetFireRate( 200 + rand()%500 ); // not all Vessels use this value

	if (ShipType == HashString::GunShip) {
		BadVessel.SetKillValue(80);
		BadVessel.SetGravityFactor(1.0f);  // better prediction for turrets
		BadVessel.SetSpeedFactor( 1.0 );
		BadVessel.SetShieldLevel( 18 ); 
		BadVessel.SetRadius(46*46);
	}
	else if (ShipType==HashString::SmallRedEnemyShip16x16x2) {
		BadVessel.SetKillValue(10);
		BadVessel.SetGravityFactor(0.965f);
		BadVessel.SetSpeedFactor( 0.84f );
		BadVessel.SetShieldLevel( m_pWii->GetXmlVariable(HashString::BadShipType2MaxShieldLevel) );
		BadVessel.SetRadius(12*12);
	}
	else if (ShipType == HashString::SmallWhiteEnemyShip16x16x2) {
		BadVessel.SetKillValue(5);
		BadVessel.SetGravityFactor(0.975f);
		BadVessel.SetSpeedFactor( 0.90f );
		BadVessel.SetShieldLevel( m_pWii->GetXmlVariable(HashString::BadShipType1MaxShieldLevel) );
		BadVessel.SetRadius(12*12);
	}

	if (ShipType == HashString::GunShip)
		m_GunShipContainer->push_back(BadVessel);
	else
		m_SmallEnemiesContainer->push_back(BadVessel);

}

//
// Anim Section
//
void GameLogic::AddScalingAnim(HashLabel Frame, Vessel* pVessel, float FrameSpeed, float SpinAmount,
							   float TopScale, float ScaleStart, float ScaleFactor)
{
	Vessel Anim = *pVessel;
	Anim.SetScaleToFactor( TopScale );			// limit value
	Anim.SetCurrentScaleFactor( ScaleStart );	// start value
	Anim.SetScaleToFactorSpeed( ScaleFactor );	// speed factor
	Anim.SetFrameGroup( m_pWii->GetFrameContainer(Frame), FrameSpeed );
	Anim.SetSpin( SpinAmount );
	m_ExplosionsContainer->push_back(Anim);
}

void GameLogic::AddAnim(HashLabel Frame, Item3D* pVessel, float FrameSpeed, float SpinAmount)
{
	Vessel Anim; //= *pVessel;
	Anim.SetPos( pVessel->GetPos() );
	Anim.SetFrameGroup( m_pWii->GetFrameContainer(Frame), FrameSpeed );
	Anim.SetSpin( SpinAmount );
	m_ExplosionsContainer->push_back(Anim);
}

void GameLogic::AddAnim(HashLabel Frame, Vessel* pVessel, float FrameSpeed, float SpinAmount)
{
	Vessel Anim = *pVessel;
	Anim.SetFrameGroup( m_pWii->GetFrameContainer(Frame), FrameSpeed );
	Anim.SetSpin( SpinAmount );
	m_ExplosionsContainer->push_back(Anim);
}

void GameLogic::AddEnemySpawn(Vessel& rItem)
{
	float vel = 2.35f;
	for (int i=0; i < m_pWii->GetConfigValueWithDifficultyApplied(HashString::AmountBadShipsFromSpore)/2; ++i)
	{
		float ang = ( rand() % (314*2) ) * 0.1f;
		AddEnemy(rItem.GetX(),rItem.GetY(), sin(ang) * vel , cos(ang) * vel, HashString::SmallWhiteEnemyShip16x16x2);
		ang = ( rand() % (314*2) ) * 0.1f;
		AddEnemy(rItem.GetX(),rItem.GetY(), sin(ang) * vel , cos(ang) * vel, HashString::SmallRedEnemyShip16x16x2);
	}
}

void GameLogic::CelestialBodyLogic()
{
	for (std::vector<MoonItem3D>::iterator iter(m_CelestialBodyContainer->begin()); iter!= m_CelestialBodyContainer->end(); ++iter ) {
		iter->Rotate();
	}
}

bool GameLogic::IsEnemyDestroyed()
{
	return (	m_SporesContainer->empty() && 
		m_SmallEnemiesContainer->empty() && 
		m_GunShipContainer->empty() &&
		m_EnemySatelliteContainer->empty() &&
		m_ExplosionsContainer->empty()	);
}

bool GameLogic::IsSingleEnemyGunsipRemaining()
{
	return (m_GunShipContainer->size()==1);
}

bool GameLogic::IsSalvagedShiledSatellites()
{
	return (m_ShieldGeneratorContainer->empty());
}

bool GameLogic::IsJustOneShiledSatelliteLeftToSalvaged()
{
	return (m_ShieldGeneratorContainer->size()==1);
}


// NOTE: Amiming for quick compile times, if any optimisitions are lost I don't care.
// This lot here been placed here so I can forward declare 'class Vessel' and avoid using #include "Vessel.h" in the declaration
int GameLogic::GetSporesContainerSize() const {  return (int)m_SporesContainer->size(); }

int GameLogic::GetEnemySatelliteContainerSize() const {  return (int)m_EnemySatelliteContainer->size(); }

int GameLogic::GetTotalEnemiesContainerSize() {  return (int)(m_SmallEnemiesContainer->size() + m_GunShipContainer->size());  }
int GameLogic::GetSmallGunTurretContainerSize() { return (int)m_pGunTurretContainer->size(); }
int GameLogic::GetSmallEnemiesContainerSize() { return (int)m_SmallEnemiesContainer->size(); }
int GameLogic::GetShieldGeneratorContainerSize() { return (int)m_ShieldGeneratorContainer->size(); }
int GameLogic::GetMissileContainerSize() { return (int)m_MissileContainer->size(); }
int GameLogic::GetGunShipContainerSize() { return (int)m_GunShipContainer->size(); }
int GameLogic::GetProbeMineContainerSize() { return (int)m_ProbeMineContainer->size(); }
int GameLogic::GetAsteroidContainerSize() { return (int)m_AsteroidContainer->size(); }
int GameLogic::GetProjectileContainerSize() { return (int)m_ProjectileContainer->size(); }
int GameLogic::GetExhaustContainerSize() { return (int)m_ExhaustContainer->size(); }
int GameLogic::GetAimPointerContainerSise() { return (int)m_AimPointerContainer->size(); }
int GameLogic::GetMoonRocksContainerSize() { return (int)m_pMoonRocksContainer->size(); }
int GameLogic::GetShotForGunTurretContainerSize() { return (int)m_ShotForGunTurretContainer->size(); }
int GameLogic::GetMaterialPickUpContainerSize() { return (int)m_pMaterialPickUpContainer->size(); }
int GameLogic::GetExplosionsContainerSize() { return (int)m_ExplosionsContainer->size(); }
int GameLogic::GetCelestialBodyContainerSize() { return (int)m_CelestialBodyContainer->size(); }
int GameLogic::GetDyingEnemiesContainerSize() { return (int)m_DyingEnemiesContainer->size(); }
int GameLogic::GetHealthPickUpContainerSize() { return (int)m_pHealthPickUpContainer->size(); }

int GameLogic::GetScorePingContainerSize() { return (int)m_pScorePingContainer->size(); }



void GameLogic::InitialiseSmallGunTurret(int Amount, float Dist, float x1, float y1, float z1 , float StartingAngleOffset )
{
	m_pGunTurretContainer->clear();

	TurretItem3D Item;

	for (int i=0; i<Amount; ++i)
	{
		float ang = (float)i * ((M_PI*2) / (float)Amount) ;
		ang+=StartingAngleOffset;
		float x = sin(ang)* Dist + x1;
		float y = + y1;
		float z = cos(ang)* Dist + z1;
		Item.SetPos( x, y, z );
		Item.SetScale( 0.8,0.8,0.8); 
		//		Item.SetScale( 1,1,1); 
		Item.SetRotate(0,ang-(M_PI/2),M_PI);
		Item.SetRotateAmount(0,0,0);

		Item.InitTimer();
		Item.SetTimerMillisecs( rand()%5000 );

		//int index( rand()%GetSmallEnemiesContainerSize()  );
		Item.SetLockOntoVesselIndex( -1 );

		Item.GetWorkingTarget().x = 0;
		Item.GetWorkingTarget().y = 0;
		Item.GetWorkingTarget().z = 0;

		// missing this causes things like 142.300247 1.#QNAN0 112.880287 in calculations later!!!!
		Item.GetCurrentTarget().x = 0;  // using shorter ={0,0,0} gives a warning 
		Item.GetCurrentTarget().y = 0;
		Item.GetCurrentTarget().z = 0;

		m_pGunTurretContainer->push_back(Item);
	}
}

void GameLogic::InitialiseMoonRocks(int Amount, float RadiusFactor)
{
	m_pMoonRocksContainer->clear();

	float minvalue = 999999;
	float maxvalue = 0;

	Item3D Asteroid;
	for (int i=0; i<Amount; ++i)
	{
		float r = 250.0f + ((rand()%250000) * RadiusFactor); 
		float ang = (float)i * ((M_PI*2) / (float)Amount);
		float x = sin(ang)*(r) ;
		float y = (50-rand()%100)*2; // using coarse values to avoid overlapping 3d rocks
		float z = cos(ang)*(r) ;

		Asteroid.SetPos( x, y, z );
		//1/250 = 0.004   //r-=250.0;  //r*=0.004f
		float d = (r - 250.0f) * 0.0015f;
		Asteroid.SetScale( 	d+((rand()%10000)*0.000025f), 
			d+((rand()%10000)*0.000025f), 
			d+((rand()%10000)*0.000025f));

		float dist = (x*x) + (y*y) + (z*z);

		maxvalue = fmax(dist,maxvalue);
		minvalue = fmin(dist,minvalue);

		Asteroid.SetRotateAmount(	
			(5000-(rand()%10000)) * 0.000025f, 
			(5000-(rand()%10000)) * 0.000025f, 
			(5000-(rand()%10000)) * 0.000025f );

		m_pMoonRocksContainer->push_back(Asteroid);
	}

	// shuffle... now we can just pull rocks out in a squence and it will will be random (easy to use less amounts of rocks if needed)
	random_shuffle( m_pMoonRocksContainer->begin(), m_pMoonRocksContainer->end() ); 

	m_ClippingRadiusNeededForMoonRocks = sqrt( fmax(maxvalue, fabs(minvalue)) );
}


void GameLogic::InitialiseShieldGenerators(int Amount)
{
	Item3DChronometry ShieldGen;
	for (int i=0; i<Amount; ++i)
	{
		float r = 2350.0f; 
		float ang = (float)i * ((M_PI*2.0f) / (float)Amount);
		float x = sin(ang)*(r) ;
		float y = cos(ang)*(r) ;
		float z = 0;
		ShieldGen.SetPos( x, y, z );
		ShieldGen.SetScale( 0.25f,0.25f,0.25f );
		ShieldGen.SetRotateAmount(	
			(5000-(rand()%10000)) * 0.000015f, 
			(5000-(rand()%10000)) * 0.000015f,
			(5000-(rand()%10000)) * 0.000015f );
		m_ShieldGeneratorContainer->push_back(ShieldGen);
	}
}

void GameLogic::InitialiseEnermyAmardaArroundLastShieldGenerator(int Amount, float Distance)
{
	Vessel GunShip;
	GunShip.SetFrameGroup( m_pWii->GetFrameContainer(HashString::GunShip), 1);
	GunShip.SetShieldLevel( 18+6 ); 
	GunShip.SetSpeedFactor( 0 );
	GunShip.SetFireRate( 35-5 );
	//	GunShip.SetBulletSpeedFactor( 0.12 );
	GunShip.SetKillValue(120);

	std::vector<Item3DChronometry>::iterator iter = m_ShieldGeneratorContainer->begin(); //take the first one - should only be one left (Misson 2 - Alert stage)
	for (int i=0; i<Amount; ++i)
	{
		float ang = (float)i * ((M_PI*2) / (float)Amount);
		float x = sin(ang)*(Distance) + iter->GetX() ;
		float y = cos(ang)*(Distance) + iter->GetY(); 
		float z = 0;
		GunShip.SetPos( x, y, z );
		m_GunShipContainer->push_back(GunShip);
	}
}

void GameLogic::InitialiseIntro()
{
	//----------------------------------------
	m_AsteroidContainer->clear();
	m_CelestialBodyContainer->clear();
	//----------------------------------------

	static int MaxAmountOfRocks = 600+25;
	MoonItem3D Moon;
	Moon.SetPos(0,0,600-200);
	Moon.SetRotateAmount(0.005f,0.005f,0.005f);  //note: just RotateY is used in logic
	Moon.SetDetailLevel(Auto);
	Moon.SetAmountOfRocks(MaxAmountOfRocks);
	m_CelestialBodyContainer->push_back(Moon);
	//----------------------------------------
	Moon.SetPos(2000,-1700,4200);
	Moon.SetAmountOfRocks(75);
	Moon.SetDetailLevel(Low);
	m_CelestialBodyContainer->push_back(Moon);
	//----------------------------------------

	// rocks are shared across the moons - largest amount needed is initialised
	InitialiseMoonRocks(MaxAmountOfRocks, 0.003f);  //todo - use the largest value in m_CelestialBodyContainer

	static int Amount = 3;
	static float Distance = 800.0f;
	static float x = 0.0f;
	static float y = 120.0f; //220.0f;
	static float z = 600.0f;
	InitialiseSmallGunTurret( Amount, Distance, x, y, z, 3.14/3.0f );

	m_GunShipContainer->clear();
	m_SporesContainer->clear();

	m_EnemySatelliteContainer->clear();
}


void GameLogic::Intro()
{
	CollisionContainer.clear();

	for (std::vector<Vessel>::iterator BadIter(m_SmallEnemiesContainer->begin()); BadIter!=m_SmallEnemiesContainer->end(); ++BadIter)
	{
		CollisionContainer [ GetHash( &(*BadIter) ) ].push_back( &(*BadIter) );
	}


	WPAD_ScanPads();

	//m_pWii->GetInputDeviceManager()->Store();   // dont call this without using the other half to empty it!!!!!

	static f32 sx=0;
	sx+=0.01;
	m_pWii->GetCamera()->SetCameraView( 
		sin(sx)*80,  -25 + cos(sx)*40,  0 , 
		sin(sx)*140, -25 + sin(sx)*160, -(700.0f) - cos(sx)*320);

	{
		static float fff(0);
		fff+=0.01;
		m_CPUTarget.x = (sin(fff)*250);
		m_CPUTarget.y = -100;
	}

	if (m_ProbeMineContainer->empty())
	{
		//m_ProbeMineContainer->clear();
		for (std::vector<Vessel>::iterator BadIter(m_SmallEnemiesContainer->begin()); BadIter!=m_SmallEnemiesContainer->end(); ++BadIter)
		{
			BadIter->SetShieldLevel(0);
		}

		Vessel ProbeMine;
		ProbeMine.SetFrameGroup(m_pWii->GetFrameContainer(HashString::ProbeMine16x16x2),0);  // intro
		ProbeMine.SetGravityFactor(0.985f);
		ProbeMine.SetVel( 0, 1.15f, 0);
		ProbeMine.SetFrameSpeed(0.0f);  // don't anim intro mines (flashing red looks nasty from distance)


		// Scan in the Tiny logo for mine layout
		WiiManager::RawTgaInfo Info = m_pWii->m_RawTgaInfoContainer[(HashLabel)"TinyLogoForMineIntroLayout"];
		Tga::PIXEL* pData = Info.m_pPixelData;

		for (int y(0); y<Info.m_pTgaHeader.height; ++y)
		{
			for (int x(0); x<Info.m_pTgaHeader.width; ++x)
			{
				Tga::PIXEL Value( pData[x + (y*Info.m_pTgaHeader.width)] );
				if (Value.b==0)  // logo on white background, using any RBG value will work
				{
					ProbeMine.SetPos(x*9-240,y*9-300,0); 
					m_ProbeMineContainer->push_back(ProbeMine);
				}
			}
		}
	}

	static bool addmore = false;
	if ((int)m_SmallEnemiesContainer->size() <= 25 ){
		addmore = true;
	}

	if (addmore) {
		if ((int)m_SmallEnemiesContainer->size() <= 50 )
		{
			//at 60 fps this is going to happan in under a second - one at a time is fine
			// this why there are no sudden jump in CPU activity
			if ( (rand()%2) == 0)
				AddEnemy(800-(rand()%1600),255+150,HashString::SmallWhiteEnemyShip16x16x2);
			else
				AddEnemy(800-(rand()%1600),-(255+150),HashString::SmallWhiteEnemyShip16x16x2);
		}
		else {
			addmore = false;
		}
	}

	BadShipsLogicForIntro();
	FeoShieldLevelLogic();
	ExplosionLogic();
	ExhaustLogic();

	m_pWii->profiler_start(&profile_ShotAndGunTurret);
	GunTurretShotsLogic( m_SmallEnemiesContainer );
	GunTurretLogic();
	m_pWii->profiler_stop(&profile_ShotAndGunTurret);

	MoonRocksLogic();

	DyingShipsLogic(); 

	CelestialBodyLogic();

	//	ProjectileLogic(); 

	if (m_ProbeMineContainer->size() < 300)
		ProbeMineLogic(m_SmallEnemiesContainer, 0.0415f, 22.0f , 0); // fast mines
	else
		ProbeMineLogic(m_SmallEnemiesContainer, 0.0015f, 22.0f , 0); // slow mines - keeps logo intact for a while longer

	//ProbeMineCollisionLogic(m_SmallEnemiesContainer, 12.0f * 12.0f );
	ProbeMineCollisionLogic();


	//m_pWii->GetGameDisplay()->DisplayAllForIntro();
	m_pWii->GetIntroDisplay()->DisplayAllForIntro();
}


void GameLogic::InitMenu()
{
	m_CelestialBodyContainer->clear();
	MoonItem3D Moon;
	//Moon.SetPos(-202,-94,400);
	Moon.SetPos(0,0,400);
	Moon.SetRotateAmount(0.0015f,0.0015f,0.0015f);  //note: just RotateY is used in logic
	Moon.SetDetailLevel(High);
	Moon.SetAmountOfRocks(250);
	m_CelestialBodyContainer->push_back(Moon);
}

void GameLogic::InitialiseGame()
{
	m_pWii->GetCamera()->SetUpView(); // 3D View

	//	m_pImageManager = (m_pWii->GetImageManager());

	srand ( 2345725); //time(NULL) );

	//----------------------------------------------------------------------
	GetPlrVessel()->ClearPickUpTotal();
	GetPlrVessel()->SetFacingDirection( m_pWii->GetXmlVariable(HashString::PlayerFacingDirection) );
	GetPlrVessel()->SetPos( m_pWii->GetXmlVariable(HashString::PlayerStartingPointX),m_pWii->GetXmlVariable(HashString::PlayerStartingPointY),0.0f );

	m_pWii->GetCamera()->SetCameraView( GetPlrVessel()->GetX(),GetPlrVessel()->GetY());

	GetPlrVessel()->SetVel( m_pWii->GetXmlVariable(HashString::PlayerStartingVelocityX), m_pWii->GetXmlVariable(HashString::PlayerStartingVelocityY),0.0f );
	GetPlrVessel()->SetShieldLevel( m_pWii->GetXmlVariable(HashString::PlayerMaxShieldLevel) );

	GetPlrVessel()->SetFrameGroup(m_pWii->GetFrameContainer(HashString::PlayersShip32x32), 1.0f);
	GetPlrVessel()->SetGravityFactor( 0.995f );
	//GetPlrVessel()->SetGoingBoom(false);
	//----------------------------
	// empty containers
	m_ShieldGeneratorContainer->clear();
	m_AsteroidContainer->clear();
	m_SporesContainer->clear();

	m_EnemySatelliteContainer->clear();

	m_MissileContainer->clear();
	m_SmallEnemiesContainer->clear();
	m_ExplosionsContainer->clear();
	m_ProbeMineContainer->clear();
	m_GunShipContainer->clear();
	m_ExhaustContainer->clear();
	m_ProjectileContainer->clear();
	m_pMaterialPickUpContainer->clear();
	m_ShotForGunTurretContainer->clear();
	m_pGunTurretContainer->clear();
	m_DyingEnemiesContainer->clear();
	m_CelestialBodyContainer->clear();
	m_pHealthPickUpContainer->clear();

	m_pScorePingContainer->clear();

	//----------------------------
	//initialise bad ships
	AddEnemy(0,0,m_pWii->GetConfigValueWithDifficultyApplied(HashString::AmountOfGunShipsAtStartUp),HashString::GunShip,2000);
	AddEnemy(0,0,m_pWii->GetConfigValueWithDifficultyApplied(HashString::AmountBadShips),HashString::SmallWhiteEnemyShip16x16x2,1200);
	AddEnemy(0,0,m_pWii->GetConfigValueWithDifficultyApplied(HashString::AmountBadShips),HashString::SmallRedEnemyShip16x16x2,1400);



	//Satellites + spore things
	for (int i=0; i<m_pWii->GetConfigValueWithDifficultyApplied(HashString::AmountBadSatellites); ++i)
	{
		static const int Width (1000);  // the area to generate them in
		static const int Height(1000);
		Vessel thing;

		int x=Width - (rand()%(Width*2));
		int y=Height - (rand()%(Height*2));
		thing.SetPos( x,y,0 );
		thing.SetFrameGroupWithRandomFrame(m_pWii->GetFrameContainer(HashString::EnemySatellite80x64x2), 0.35f);
		thing.SetShieldLevel(9);
		thing.SetKillValue(800);
		thing.SetID(i);
		m_EnemySatelliteContainer->push_back(thing); // Tiny spore to circle Enemy Satellite
		guVector v = thing.GetPos();
		//
		for (int AddSpores=0; AddSpores < m_pWii->GetConfigValueWithDifficultyApplied(HashString::AmountBadSpores); AddSpores++) {
			//reuse thing here, the outer loop will refresh
			thing.SetFrameGroupWithRandomFrame( m_pWii->GetFrameContainer(HashString::SpinningSpore16x16x9), 0.35f);
			thing.SetShieldLevel(1);
			thing.SetKillValue(25);
			thing.SetGravityFactor(1.0f);
			thing.SetPos( x,y,0 );
			thing.SetDestinationPos(v);
			thing.SetVel( (100 - rand()%200) * 0.005f,
				(100 - rand()%200) * 0.005f, 0.0f );
			thing.SetID(i);  // each is given the satellite's ID it circles arround
			m_SporesContainer->push_back(thing);
		}
	}

	m_bDoEndGameEventOnce = true;
	SetEndLevelTrigger( false );

	SetScore(0);
	InitialiseMoonRocks(120);

	// Initialise Asteroids
	Item3D Asteroid;

	static const float ScaleUp(100.0f);
	static const float ScaleDown(1.0f/100.0f);
	int AsteroidWidth = m_pWii->GetXmlVariable(HashString::AsteroidWidthCoverage) * ScaleUp;
	int AsteroidHeight = m_pWii->GetXmlVariable(HashString::AsteroidHeightCoverage) * ScaleUp;
	static const float CleanArea( (m_ClippingRadiusNeededForMoonRocks*m_ClippingRadiusNeededForMoonRocks) * 1.75f );
	for (int i=0; i<m_pWii->GetXmlVariable(HashString::AsteroidTotal); ++i)
	{
		Asteroid.SetPos((AsteroidWidth  - (rand()%(AsteroidWidth *2))) * ScaleDown, 
			(AsteroidHeight - (rand()%(AsteroidHeight*2))) * ScaleDown, 
			((-500 * ScaleUp) + rand()%int(1000 * ScaleUp)) * ScaleDown );

		Asteroid.SetRotateAmount((5000-(rand()%10000)) * 0.0000065f, 
			(5000-(rand()%10000)) * 0.0000065f, 
			(5000-(rand()%10000)) * 0.0000065f );

		Asteroid.SetScale(	0.5f+((rand()%10000)*0.000035f), 
			0.5f+((rand()%10000)*0.000035f), 
			0.5f+((rand()%10000)*0.000035f));

		if ( !Asteroid.InsideRadius(0, 0, CleanArea ) )  // Not arround moon's spinning rocks
			m_AsteroidContainer->push_back(Asteroid);
	}

	m_IsBaseShieldOnline = false;

	MissionManager* pMissionManager(m_pWii->GetMissionManager());

	pMissionManager->Clear();

	pMissionManager->AddMissionData("Mission One", 
		"Extirpate enemy vessels in close vicinity to your base. \n \n" 
		"Tip:The enemy shields have moving parts that are vulnerable to a well timed shot.",
		"The close range threat has been removed.");

	pMissionManager->AddMissionData("Mission Two",
		"Locate & recover your missing terraforming satellites. \n \n"
		"Tip: Rendezvous with each for a short while to enable recovery.",
		"");

	pMissionManager->AddMissionData("Alert",
		"Your final shield satellite has been surrounded by the enemies Armada. \n"
		"Remove this threat and recover the last satellite to bring your terraforming process online.",
		"All satellites have been recovered \n \nYour base's terraforming shields are now online.");

	pMissionManager->AddMissionData("Mission Three",
		"Collect x85 scrap parts from your encounters with the enemy. \n \n"
		"Collecting scrap will enable a ring of defence turrets to be build around the perimeter of your base.",
		"You have collected enough scrap for the defensive built to start.");

	pMissionManager->AddMissionData("Alert",
		"A vast enemy fleet has been detected \n \n"
		"Delay this threat until your defensive power can repel them, collect a further 155 scrap.",
		"Your base's defensive perimeter is now complete.");

	// 6
	pMissionManager->AddMissionData("Mission Four",
		"Protect your moon base while the terraforming process creates a habitable atmosphere.\n",
		"Terraforming is now complete.");

	// 7 - survival mission - never ends 
	pMissionManager->AddMissionData("Mission Survival",
		"Protect your new habitat, this final mission never ends.\n",
		"Well done");

	MoonItem3D Moon;
	Moon.SetPos(0,0,600);
	Moon.SetRotateAmount(0.005f,0.005f,0.005f);
	Moon.SetDetailLevel(Low);
	Moon.SetAmountOfRocks(120);  // TODO - things above need to work with this
	m_CelestialBodyContainer->push_back(Moon);


	// clear any previous game timer, don't want message popping up
	GetPlrVessel()->m_PopUpMessageTimer.ResetTimer();

	m_TerraformingCounter = 0.0f;

	//--------------
#ifndef BUILD_FINAL_RELEASE
	//debug - remove for final release
	m_pWii->profiler_create(&profile_ProbeMineLogic, "Mines");
	m_pWii->profiler_create(&profile_Asteroid, "Asteroid");
	m_pWii->profiler_create(&profile_MoonRocks, "MoonRocks");
	m_pWii->profiler_create(&profile_SmallEnemies, "SmallEnemies");
	m_pWii->profiler_create(&profile_GunShip, "GunShip");
	m_pWii->profiler_create(&profile_Explosions, "Explosions");
	m_pWii->profiler_create(&profile_Spores, "Spores");
	m_pWii->profiler_create(&profile_Missile, "Missile");
	m_pWii->profiler_create(&profile_Exhaust, "Exhaust");
	m_pWii->profiler_create(&profile_Projectile, "Projectile");
	//	m_pWii->profiler_create(&profile_Mission, "Mission");
	m_pWii->profiler_create(&profile_ShotAndGunTurret, "ShotForGunTurret");
	m_pWii->profiler_create(&profile_DyingEnemies, "DyingEnemies");
	m_pWii->profiler_create(&profile_MissileCollisionLogic, "Missile Collision");

#endif
	//--------------




	// TEST AREA - NOT TO BE COMPILED AS PART OF RELEASE PACKAGE

	//static int Amount = 3;
	//static float Distance = 800.0f;
	//static float x = 0.0f;
	//static float y = 220.0f;
	//static float z = 600.0f;
	//InitialiseSmallGunTurret( Amount, Distance, x, y, z, 3.14/3.0f );


	//InitialiseShieldGenerators(3);

	//m_SmallEnemiesContainer->clear();
	//m_GunShipContainer->clear();
	//AddEnemy(0,0,1,HashString::GunShip,666);


	//AddEnemy(0,0,999,HashString::SmallRedEnemyShip16x16x2,1200);

	//InitialiseSmallGunTurret(4,700, 0,120,600, 3.14/4.0f );
}

