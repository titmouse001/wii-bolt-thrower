// todo list
// ADded this functionality to gamelib...m_pImageManager->GetImage(HashString::WiiMoteButtonA)->DrawImage(100,100);
// added debugprintf to jpeg stufff ... merge with Glib
// move sound fix - chan var is wrong .. should be holding Voice format, VOICE_MONO_16BIT... 

#include "GameLogic.h"
#include "Singleton.h"

#include <gccore.h>
#include <math.h>
#include <sstream>

#include "WiiManager.h"
#include "CullFrustum\FrustumR.h"
#include "Image.h"
#include "ImageManager.h"
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

//	 KEEP reminder... TargetIter  = m_GunShipContainer->begin(); //		pTarget = &(*m_GunShipContainer->begin());  !!! WARNING: NOT SURE WHY THIS METHOD DOES NOT WORK (gives very odd results) !!!


// TODO 
// 1 clear missiles after a level
// 2 blue pickup glow
// 3 probe mines - show red lock on lines

GameLogic::GameLogic():	//m_bAddMoreShipsFlag(false), 
m_bEndLevelTrigger(false), m_Score(0),
m_ZoomAmountForSpaceBackground(3.1f),m_ClippingRadiusNeededForMoonRocks(0),  
m_GamePaused(false),m_GamePausedByPlayer(false),m_IsBaseShieldOnline(false),m_LastChanUsedForSoundAfterBurn(NULL)
{
	m_Timer = new Timer;
	m_MyVessel = new PlayerVessel;
	m_AimPointerContainer = new vector<guVector>;

	m_ShieldGeneratorContainer = new std::vector<Item3DChronometry>;
	m_AsteroidContainer = new std::vector<Item3D>;
	m_SporesContainer = new std::vector<Vessel>;
	m_MissileContainer = new std::vector<Vessel>;
	m_SmallEnemiesContainer = new std::vector<Vessel>;
	m_ExplosionsContainer = new std::vector<Vessel>;
	m_ProbeMineContainer = new std::vector<Vessel>;
	m_GunShipContainer = new std::vector<Vessel>;
	m_ExhaustContainer = new std::vector<Vessel>;
	m_ProjectileContainer = new std::vector<Vessel>;
	m_pMoonRocksContainer = new std::vector<Item3D>;  // for intro
	m_pGunTurretContainer = new std::vector<TurretItem3D>;
	m_pMaterialPickUpContainer= new std::vector<Item3D>;
	m_ShotForGunTurretContainer = new std::vector<Item3D>;

	m_DyingEnemiesContainer = new std::vector<Vessel>;

	m_CelestialBodyContainer = new std::vector<MoonItem3D>;
}

void GameLogic::Init()
{
	m_pWii = Singleton<WiiManager>::GetInstanceByPtr();
	m_pSoundManager =  m_pWii->GetSoundManager();

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
			m_pSoundManager->PlaySound( HashString::FireMissle,255,255);

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
			Missile.SetGravity(1.0f);
			m_MissileContainer->push_back(Missile);
		}


		if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_B)  // use thrusters
		{
			if (m_LastChanUsedForSoundAfterBurn == NULL)
			{
				m_LastChanUsedForSoundAfterBurn = m_pSoundManager->PlaySound( HashString::AfterBurn,32,32,true);
			}

			GetPlrVessel()->AddVel( sin(GetPlrVessel()->GetFacingDirection() )*0.075,-cos( GetPlrVessel()->GetFacingDirection() )*0.075,0);

			// Thrust from back of ship  - todo need to make this far more simple to use, need more functionality
			Vessel Tail = *GetPlrVessel();
			Tail.SetFrameGroup( HashString::ExplosionThrust1Type16x16x10, 0.35f);
			Tail.SetGravity(1.0f);
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
				m_pSoundManager->StopSound(m_LastChanUsedForSoundAfterBurn);
				m_LastChanUsedForSoundAfterBurn = NULL;	
			}
		}

		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_DOWN)  // Mines
		{
			Vessel ProbeMine = *GetPlrVessel();
			ProbeMine.SetFrameGroup( HashString::ProbeMine16x16x5);
			float dir = ProbeMine.GetFacingDirection() - (((rand()%(314/16))-(314/32))*0.01);
			float v = (rand()%10) * 0.05f;
			ProbeMine.SetGravity(0.985f);
			ProbeMine.AddVel(sin( dir  )* -(1.75+v),-cos( dir )* -(1.75+v),0);
			ProbeMine.SetFacingDirection(0);
			m_ProbeMineContainer->push_back(ProbeMine);
			m_pSoundManager->PlaySound( HashString::DropMine,100,100 );
		}
	}
}

void GameLogic::StillAlive()
{
	TurnShipLogic();

	float ScrollFactor = 0.065f;
	if (IsGamePaused())
	{
		if (IsGamePausedByPlayer())
			ScrollFactor = 0.005f;	
		else
			ScrollFactor = 0.015f;	
	}
	// move Camera using factor
	m_pWii->GetCamera()->CameraMovementLogic(GetPlrVessel()->GetX(), GetPlrVessel()->GetY(), m_pWii->GetCamera()->GetCamZ(), ScrollFactor);

	MissionManager* pMissionManager( m_pWii->GetMissionManager() );
	Mission& MissionData( pMissionManager->GetMissionData() );
	if ( pMissionManager->IsCurrentMissionObjectiveComplete() )
	{

	//	printf( "MissionObjectiveComplete" );

		if ( (MissionData.GetCompleted()>0) && (!m_pWii->GetMessageBox()->IsEnabled()) )
		{
			if (!MissionData.GetMissionCompletedText().empty())
			{
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
				//InitialiseSmallGunTurret(5,300);

				InitialiseSmallGunTurret(4,700, 0,120,600, 3.14/3.0f );
				GetPlrVessel()->SetShieldLevel( m_pWii->GetXmlVariable(HashString::PlayerMaxShieldLevel) );
				break;
			case 6:
				GetPlrVessel()->SetPos(0,0,0);
				GetPlrVessel()->SetVel(0,0,0);
				m_ExhaustContainer->clear();
				m_ProbeMineContainer->clear();
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
			break;
		}
	}

	if ((MissionData.GetCompleted()==0) && (!m_pWii->GetMessageBox()->IsEnabled()) )
	{
		// next mission message
		m_pWii->GetMessageBox()->SetUpMessageBox(MissionData.GetMissionName(),MissionData.GetMissionText());
		MissionData.SetCompleted(1);  // 0% to 100%
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

void GameLogic::InGameLogic()
{
	if ( GetPlrVessel()->IsShieldOk() )
	{
		m_CPUTarget.x = GetPlrVessel()->GetX();
		m_CPUTarget.y = GetPlrVessel()->GetY();
	}
	else
	{
		static float ang(0.0f);
		ang+=0.0065f;
		m_CPUTarget.x = GetPlrVessel()->GetX() + sin(ang)*220;
		m_CPUTarget.y = GetPlrVessel()->GetY() + cos(ang)*220;
	}

	DoControls();

	if (!IsGamePaused())
	{
		GetPlrVessel()->VelReduce();
		GetPlrVessel()->AddVelToPos();
		PickUpsLogic();
		MissileLogic();
		ProbeMineLogic(m_SmallEnemiesContainer, 0.035f, 160.0f,22.0f, (12.0f * 12.0f) );
		ProbeMineLogic(m_GunShipContainer, 0.035f, 200.0f, 48.0f, (38.0f * 38.0f));

		BadShipsLogic();

		PlayerCollisionLogic();
		FeoShieldLevelLogic();
		MissileCollisionLogic();

		MoonRocksLogic();
		
		//	m_pWii->profiler_start(&myjob1);
		AsteroidsLogic();
		//	m_pWii->profiler_stop(&myjob1);
		SporesCollisionLogic();

		GunShipLogic();

		
		m_pWii->profiler_start(&profile_ShotAndGunTurret);
		GunTurretShotsLogic( m_GunShipContainer );
		GunTurretShotsLogic(m_SmallEnemiesContainer);  // before GunTurretLogic - so each shot leqaves the barrel correctly
		GunTurretLogic();
		m_pWii->profiler_stop(&profile_ShotAndGunTurret);


		ExhaustLogic();
		ProjectileLogic();
		ExplosionLogic();

		DyingShipsLogic();

		CelestialBodyLogic();
	}

	if (GetPlrVessel()->HasShieldFailed())
	{
		// player has died
		static float AmountOfexplosionsToAddEachFrame(20);
		m_pWii->GetCamera()->CameraMovementLogic(GetPlrVessel()->GetX(), GetPlrVessel()->GetY(),-579.4f - GetPlrVessel()->GetZ(), 0.065f );
		if (m_bDoEndGameEventOnce)
		{
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
			ProbeMine.SetGravity(0.995f);
			ProbeMine.SetFrameGroup( HashString::ProbeMine16x16x5);
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
		Util3D::TransRot(GetPlrVessel()->GetX()-600,GetPlrVessel()->GetY(),700,-M_PI/8);
		m_pWii->GetFontManager()->DisplayTextCentre("Visit",									344,-290,166,HashString::LargeFont);
		m_pWii->GetFontManager()->DisplayTextCentre("http://wiibrew.org/wiki/BoltThrower",		344,-250,166,HashString::LargeFont);
		m_pWii->GetFontManager()->DisplayTextCentre("to  add  your  ideas",					344,-210,166,HashString::LargeFont);

		if (IsEndLevelTrigger())
		{
			Util3D::Trans(GetPlrVessel()->GetX(),GetPlrVessel()->GetY(),0);
			m_pWii->GetFontManager()->DisplayTextCentre("PRESS A TO CONTINUE",exp(sin(wobble)*2.8f),220,128,HashString::LargeFont);
		}

		GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);

	//	GetPlrVessel()->SetVel(0,0,0.85f);
	//	GetPlrVessel()->SetGravity(0.99f);

		static const HashLabel names[] = { HashString::ExplosionThrust1Type16x16x10 , HashString::ExplosionSmoke1Type16x16x10 };

		Vessel Boom = *GetPlrVessel();
		Boom.SetGravity(1);

		if (AmountOfexplosionsToAddEachFrame > 1)
		{
			int Amount = (rand()%(int)AmountOfexplosionsToAddEachFrame);
			for (int i=0; i<Amount; i++) // add lots of random explosions
			{
				int nameindex = i&1;	
				Boom.SetFrameGroup(names[nameindex],0.15f + ((rand()%100)*0.0015f));
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

	m_pWii->GetGameDisplay()->DisplayAllForIngame();

	
	GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);
	m_pWii->SwapScreen();  // to clear zbuffer keep GX_SetZMode on until after this call 
	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
}

void GameLogic::AsteroidsLogic()
{
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );
	float Radius(640); //m_pWii->GetXmlVariable(HashString::ViewRadiusForAsteroids));
	Radius*=Radius;

	for (std::vector<Item3D>::iterator iter(m_AsteroidContainer->begin()); iter!= m_AsteroidContainer->end(); ++iter )
	{	
		/// timings before this  'InsideRadius' line: 1327 to 1428, now with this line: 87 to 192 ! 
		if ( iter->InsideRadius(fCamX, fCamY, Radius ) )  // fast clipping, sphere sits arround frustum
		{
			iter->Rotate();
			iter->SetEnable(true);
		}
		else
		{
			iter->SetEnable(false);
		}
	}
}

void GameLogic::ProjectileLogic()
{
	for (std::vector<Vessel>::iterator Iter(m_ProjectileContainer->begin()); Iter!= m_ProjectileContainer->end(); /*NOP*/)
	{	
		Iter->AddFrame();
		if ( (floor)(Iter->GetFrame()) <= Iter->GetEndFrame() )
		{
			Iter->AddFacingDirection( Iter->GetSpin() );
			Iter->AddVelToPos(); 
			Iter->VelReduce();
			++Iter;
		}
		else // end of projectile's life - this will add a final spent explosion at the end
		{
			AddScalingAnim(HashString::YellowEdgeExplosion64x64, 
				&(*Iter),
				0.05f, //frame speed
				( 800 - ( rand()%1600) ) * 0.00025f, // Spin Amount
				0.85f, //Top Scale
				0.001f, // Start Scale
				0.15f ); // Scale Factor

			Iter = m_ProjectileContainer->erase( Iter );
		}
	}
}

void GameLogic::ExhaustLogic()
{

	m_pWii->profiler_start(&profile_Exhaust);

	u8 fps = Util::CalculateFrameRate(true);
	if ((fps<60) && (m_ExhaustContainer->size() > 550))  //should donly kick in for the intro as it uses lots and drags the frame rate down!
	{
		u32 ReduceAmount = (60-fps)*5;  // best guess amount to drop
//		printf("%d %d,",fps,ReduceAmount);
		m_ExhaustContainer->erase(m_ExhaustContainer->begin(), (m_ExhaustContainer->begin() + ReduceAmount  ));
	}

	for (std::vector<Vessel>::iterator Iter(m_ExhaustContainer->begin()); Iter!= m_ExhaustContainer->end(); /*NOP*/)
	{	
		Iter->AddFrame();
		if ( (floor)(Iter->GetFrame()) <= Iter->GetEndFrame() )
		{
			Iter->AddFacingDirection( Iter->GetSpin() );
			Iter->AddVelToPos(); 
			Iter->VelReduce();
			++Iter;
		}
		else
		{
			Iter = m_ExhaustContainer->erase( Iter );
		}
	}

	
	m_pWii->profiler_stop(&profile_Exhaust);

}

void GameLogic::ExplosionLogic()
{
	m_pWii->profiler_start(&profile_Explosions);

	for (std::vector<Vessel>::iterator ExplosionIter(m_ExplosionsContainer->begin()); ExplosionIter!= m_ExplosionsContainer->end(); /*NOP*/)
	{	
		ExplosionIter->AddFrame();
		// round down current frame
		if ( (floor)(ExplosionIter->GetFrame()) <= ExplosionIter->GetEndFrame() )
		{
			if (ExplosionIter->GetScaleToFactor() == 1.0f)
			{
				ExplosionIter->SetAlpha( max( 0, 255 - ((int)ExplosionIter->GetZ()/2) ) ); // fudge ... for when ships fall into distance
			}
			else
			{
				float div = (ExplosionIter->GetEndFrame() - ExplosionIter->GetFrameStart()) + 1.0f;
				if (div <= 0.01f) 
				{	
					div=1.0;
				}

				float scale = 255.0f - ((ExplosionIter->GetFrame() - ExplosionIter->GetFrameStart()) * (255.0f / div) );
				ExplosionIter->SetAlpha( scale ); 
			}

			ExplosionIter->AddFacingDirection( ExplosionIter->GetSpin() );
			ExplosionIter->AddVelToPos(); 
			ExplosionIter->VelReduce();
			ExplosionIter->AddCurrentScaleFactor( 
				((ExplosionIter->GetScaleToFactor() - 
				ExplosionIter->GetCurrentScaleFactor()) * 
				ExplosionIter->GetScaleToFactorSpeed() ));	
			++ExplosionIter;
		}
		else
		{
			ExplosionIter = m_ExplosionsContainer->erase( ExplosionIter );
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
		f32 HalfScreenWidthIncludingOverrun  = m_pWii->GetScreenWidth() / 2;
		f32 HalfScreenHeightIncludingOverrun = m_pWii->GetScreenHeight() / 2;
		guVector PlrVessel;
		PlrVessel.x = m_pWii->GetCamera()->GetCamX() + WiiMote->x - HalfScreenWidthIncludingOverrun;
		PlrVessel.y = m_pWii->GetCamera()->GetCamY() + WiiMote->y - HalfScreenHeightIncludingOverrun;

		f32 Turn = GetPlrVessel()->GetTurnDirection( &PlrVessel );
		GetPlrVessel()->AddFacingDirection( Turn * 0.085f );
	}
}

void GameLogic::SporesCollisionLogic()
{
	for (std::vector<Vessel>::iterator MissileIter(m_MissileContainer->begin()); MissileIter!=m_MissileContainer->end(); ++MissileIter )
	{
		Vessel& rMissile(*MissileIter);
		for (std::vector<Vessel>::iterator ThingIter(m_SporesContainer->begin()); ThingIter!=m_SporesContainer->end();/* ++iter*/)
		{
			if ( ThingIter->InsideRadius(rMissile.GetX(), rMissile.GetY(), 10*10 ) )
			{
				AddScore(99);
				AddEnemySpawn(*ThingIter);
				rMissile.SetFuel(0);
				ThingIter = m_SporesContainer->erase( ThingIter );
				break;
			}
			else
			{
				++ThingIter;
			}
		}
	}
}

void GameLogic::MissileCollisionLogic()
{
	float fRocketCollisionRadius = 3*16;
	fRocketCollisionRadius *= fRocketCollisionRadius;

	for (std::vector<Vessel>::iterator MissileIter(m_MissileContainer->begin()); MissileIter!=m_MissileContainer->end(); ++MissileIter )
	{
		for (std::vector<Vessel>::iterator GunShipIter(m_GunShipContainer->begin()); GunShipIter!=m_GunShipContainer->end(); ++GunShipIter)
		{
			if ( GunShipIter->IsShieldOk() && (GunShipIter->InsideRadius(MissileIter->GetX(), MissileIter->GetY(), fRocketCollisionRadius )) )
			{
				GunShipIter->AddShieldLevel(-1);

				if ( GunShipIter->IsShieldOk() )  // hit something but their shileds are holding
				{
					MissileIter->SetGravity(0.975f);  // missile will be deleted ... we can do this
					MissileIter->VelReduce();

					AddScalingAnim(HashString::YellowEdgeExplosion64x64, 
						&(*MissileIter),
						0.05f,									// frame speed
						( 800 - ( rand()%1600) ) * 0.00025f,	// Spin Amount
						0.65f + ((rand()%25)*0.01f),				// Top Scale
						0.001f,									// Start Scale
						0.25f );								// Scale Factor

					m_pSoundManager->PlaySound( HashString::hitmetal,100,100);
				}

				MissileIter->SetFuel(0);
				break;
			}
		}

		float fRocketCollisionRadius = m_pWii->GetXmlVariable( HashString::RocketCollisionRadius );
		fRocketCollisionRadius *= fRocketCollisionRadius;

		for (std::vector<Vessel>::iterator BadIter(m_SmallEnemiesContainer->begin()); BadIter!=m_SmallEnemiesContainer->end(); ++BadIter)
		{
			if ( (BadIter->IsShieldOk()) && (BadIter->InsideRadius(MissileIter->GetX(), MissileIter->GetY(), fRocketCollisionRadius )) )
			{
				AddAnim(HashString::ExplosionThrust1Type16x16x10, &(*BadIter), 0.45f, (1000 - (rand()%2000 )) * 0.00005f );

				BadIter->AddShieldLevel(-1);
				if (BadIter->IsShieldOk())  // hit something but their shileds are holding
				{
					m_pSoundManager->PlaySound( HashString::hitmetal,100,100);
					BadIter->AddFacingDirection(M_PI*0.5);
					BadIter->AddVel( sin( MissileIter->GetFacingDirection() )*1.55,-cos( MissileIter->GetFacingDirection() )*1.55, 0 );
				}
				else
				{
					AddScalingAnim(HashString::YellowEdgeExplosion64x64, 
						&(*BadIter),
						0.02f,									// frame speed
						( 800 - ( rand()%1600) ) * 0.00025f,	// Spin Amount
						0.75f + ((rand()%65)*0.01f), 			// Top Scale
						0.01f,									// Start Scale
						0.05f );								// Scale Factor
				}
				MissileIter->SetFuel(0);
				break;
			}
		}
	}
}

void GameLogic::AddScalingAnim(HashLabel Frame, Vessel* pVessel, float FrameSpeed, float SpinAmount,
							   float TopScale, float ScaleStart, float ScaleFactor)
{
	Vessel Anim = *pVessel;
	Anim.SetScaleToFactor( TopScale );			// limit value
	Anim.SetCurrentScaleFactor( ScaleStart );	// start value
	Anim.SetScaleToFactorSpeed( ScaleFactor );	// speed factor
	Anim.SetFrameGroup( Frame, FrameSpeed );
	Anim.SetSpin( SpinAmount );
	m_ExplosionsContainer->push_back(Anim);
}

void GameLogic::AddAnim(HashLabel Frame, Item3D* pVessel, float FrameSpeed, float SpinAmount)
{
	Vessel Anim; //= *pVessel;
	Anim.SetPos( pVessel->GetPos() );
	Anim.SetFrameGroup( Frame, FrameSpeed );
	Anim.SetSpin( SpinAmount );
	m_ExplosionsContainer->push_back(Anim);
}

void GameLogic::AddAnim(HashLabel Frame, Vessel* pVessel, float FrameSpeed, float SpinAmount)
{
	Vessel Anim = *pVessel;
	Anim.SetFrameGroup( Frame, FrameSpeed );
	Anim.SetSpin( SpinAmount );
	m_ExplosionsContainer->push_back(Anim);
}


void GameLogic::FeoShieldLevelLogic()   // explode enemy ships
{
	// Check for Shield Level on bad ships
	for (std::vector<Vessel>::iterator BadIter(m_SmallEnemiesContainer->begin()); BadIter!=m_SmallEnemiesContainer->end(); /*++BadIter*/ )
	{
		if ( BadIter->HasShieldFailed() )
		{
			Vessel Boom(*BadIter);
			AddScore( 5 );
			AddPickUps(&Boom,1);

			Boom.SetFuel(100 + rand()%50);
			Boom.AddVel(0,0,0.95f);
			Boom.SetGravity(1.0f);
			m_DyingEnemiesContainer->push_back(Boom); // add to DyingEnemies

			m_pSoundManager->PlaySound( HashString::Explode01,100,100);
			BadIter = m_SmallEnemiesContainer->erase(BadIter); // remove from this list
		}
		else
		{
			++BadIter;
		}
	}

	for (std::vector<Vessel>::iterator GunShipIter(m_GunShipContainer->begin()); GunShipIter!=m_GunShipContainer->end(); /* ++GunShipIter */)
	{
		if ( GunShipIter->HasShieldFailed() )
		{
			Vessel Boom(*GunShipIter);
			AddScore( 45 );
			AddPickUps(&Boom,5);

			Boom.SetFuel(100 + rand()%50);
			Boom.AddVel(0,0,1.25f);
			Boom.SetGravity(1.0f);
			m_DyingEnemiesContainer->push_back(Boom); // Add to DyingEnemies

			m_pSoundManager->PlaySound( HashString::Explode01,100,100);
			GunShipIter = m_GunShipContainer->erase(GunShipIter); // remove from this list
		}
		else
		{
			++GunShipIter;
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
			if ( GetPlrVessel()->InsideRadius(BadIter->GetX(), BadIter->GetY(), fPlayerCollisionRadius) )
			{
				GetPlrVessel()->AddShieldLevel (-fReducePlayerShieldLevelByAmount);
				GetPlrVessel()->SetShieldLevel (std::max(0,GetPlrVessel()->GetShieldLevel() ));
				// step back so we are outside the hit zone
				BadIter->SetVel( -BadIter->GetVelX() + GetPlrVessel()->GetVelX(), 
					-BadIter->GetVelY() + GetPlrVessel()->GetVelY(), 0.0f );
				BadIter->AddVelToPos();
				BadIter->AddShieldLevel(-1);
			}
		}

		fPlayerCollisionRadius = 8;
		fPlayerCollisionRadius*=fPlayerCollisionRadius;

		for (std::vector<Vessel>::iterator ProjectileIter(m_ProjectileContainer->begin()); ProjectileIter!=m_ProjectileContainer->end(); /*nop*/ )
		{
			if ( GetPlrVessel()->InsideRadius(ProjectileIter->GetX(), ProjectileIter->GetY(), fPlayerCollisionRadius ) )
			{
				GetPlrVessel()->AddShieldLevel( -fReducePlayerShieldLevelByAmount );
				GetPlrVessel()->SetShieldLevel( std::max(0,GetPlrVessel()->GetShieldLevel() ) );

				Vessel Boom = *ProjectileIter;
				Boom.SetFrameGroup(HashString::ExplosionSolidType32x32x10,0.25f);
				Boom.SetSpin( (1000 - (rand()%2000 )) * 0.00005f );
				m_ExplosionsContainer->push_back(Boom);	// projectile explosion

				AddScalingAnim(HashString::YellowEdgeExplosion64x64, 
					&(*ProjectileIter),
					0.05f, //frame speed
					( 800 - ( rand()%1600) ) * 0.00025f, // Spin Amount
					0.75f + (rand()%75)*0.01 , //Top Scale
					0.5f, // Start Scale
					0.20f ); // Scale Factor

				m_pSoundManager->PlaySound( HashString::Explode01,100,100);
				ProjectileIter = m_ProjectileContainer->erase( ProjectileIter );
			}
			else
			{
				++ProjectileIter;
			}
		}
	}
}

void GameLogic::MissileLogic()
{
	m_pWii->profiler_start(&profile_Missile);

	int RandomValue = 1 + (rand()%2);
	for (std::vector<Vessel>::iterator MissileIter(m_MissileContainer->begin()); MissileIter!= m_MissileContainer->end(); /* NOP */)
	{
		if ( MissileIter->GetFuel() > 0 )
		{
			float dir( MissileIter->GetFacingDirection() );
			float mx(sin( dir ) *  0.15f);
			float my(cos( dir ) * -0.15f);

			if ( (MissileIter->GetFuel()&RandomValue) &&  // something random'ish 
				 (MissileIter->GetFuel() > (60*9) ) )  // ( 60fps x N ) don't bother drwing thrust after this limit.
			{
				Vessel thrust( *MissileIter ); // create some thrust flames coming out the back end of the rocket
				thrust.AddPos( -mx*2.0f, -my*2.0f, 0  );
				thrust.AddVel( -mx*32.0f, -my*32.0f, 0); 
				thrust.SetGravity(0.89f);
				AddAnim(HashString::ExplosionSmoke1Type16x16x10, &thrust, 0.5f, (1000 - (rand()%2000 )) * 0.00025f );
			}
			MissileIter->AddVel( mx, my, 0);
			MissileIter->AddVelToPos();
			MissileIter->ReduceFuel();
			++MissileIter;
		}
		else
		{
			MissileIter = m_MissileContainer->erase( MissileIter );
		}
	}
	
	m_pWii->profiler_stop(&profile_Missile);
}

void GameLogic::ProbeMineLogic(std::vector<Vessel>*  pVesselContainer, float ThrustPower, float ActiveRange, float ScanRange, float CraftSize)
{
	m_pWii->profiler_start(&profile_ProbeMineLogic);

	// Logic for Probe Mines - checks against all bad ships
	float ActiveRangeSquared( ActiveRange * ActiveRange );
	for (std::vector<Vessel>::iterator ProbeMineIter(m_ProbeMineContainer->begin()); ProbeMineIter!= m_ProbeMineContainer->end(); /*++ProbeMineIter*/ )
	{
		ProbeMineIter->SetFrame( ProbeMineIter->GetFrameStart() );   // reset frame - this frame value may get overwritten depending if thrust is used

		bool bMineDeleted(false);
		bool HaveLockOnSoForgetTheRest(false); // but still check for collisions
		for (std::vector<Vessel>::iterator BadIter(pVesselContainer->begin()); BadIter!= pVesselContainer->end(); ++BadIter )
		{
			if ( BadIter->HasShieldFailed() )  
				continue;
	
			// Only show what is inside the viewport
			if ( ProbeMineIter->InsideRadius(*BadIter, ActiveRangeSquared ) )
			{
				// Has the mine hit a ship?
				if ( ProbeMineIter->InsideRadius(*BadIter, CraftSize ) )
				{
					BadIter->AddShieldLevel( -3 );  // reduce shiled of the thing its hit - currently mines only effects one thing
					AddAnim(HashString::ExplosionSolidType32x32x10, &(*ProbeMineIter), 0.25f, 0 );
					AddScalingAnim(HashString::YellowEdgeExplosion64x64, 
						&(*ProbeMineIter),
						0.070f,									// frame speed
						( 100 - ( rand()%200) ) * 0.0015f,		// Spin Amount
						1.25f + (rand()%50)*0.01,				// Top Scale
						0.25,									// Start Scale
						0.10f );								// Scale Factor

					AddScalingAnim(HashString::YellowEdgeExplosion64x64, 
						&(*ProbeMineIter),
						0.080f,									// frame speed
						( 100 - ( rand()%200) ) * 0.0015f,		// Spin Amount
						2.2f + (rand()%75)*0.01,				// Top Scale
						0.0001,									// Start Scale
						0.15f );								// Scale Factor
					//-------------------

					ProbeMineIter = m_ProbeMineContainer->erase( ProbeMineIter );  // SAFE to erase - about to exit loop
					bMineDeleted = true;
					m_pSoundManager->PlaySound( HashString::Explode11,100,100 );
					break; // dont bother with checking the rest of the bad ships since the mine has exploded (only takes out one ship)
				}
				else if (!HaveLockOnSoForgetTheRest)
				{
					// TODO maybe look into a priority things on each mine, gets higher each timer its ignored!
					//if (m_pWii->GetFrameCounter()%4) //... hmmmmmm this counter  will do all of them in on go! rand() may help CPU be placing the load across a number of frames 
					//if (rand()%4==0)
					{
						// Lock onto ships passing on the X-axis
						if ( fabs( ProbeMineIter->GetX() - BadIter->GetX() ) < ScanRange )
						{
							Vessel ProbeMineTail = *ProbeMineIter;
							if (ProbeMineIter->GetY() > BadIter->GetY() )
							{
								ProbeMineIter->AddVel(0, -ThrustPower, 0);
								ProbeMineIter->SetFrame( ProbeMineIter->GetFrameStart() + 3 ); // use Bottom thruster
								ProbeMineTail.AddVel(0.5 - ((rand()%100)*0.01),2.0 + ((rand()%300)*0.01),0);
								ProbeMineTail.SetFrameGroup( HashString::ProbeMineDownThrust16x16x5, 1.0f );
							}
							else
							{
								ProbeMineIter->AddVel(0, ThrustPower, 0);
								ProbeMineIter->SetFrame( ProbeMineIter->GetFrameStart() + 1 ); // use Top thruster
								ProbeMineTail.AddVel(0.5 - ((rand()%100)*0.01),-2.0 - ((rand()%300)*0.01),0);
								ProbeMineTail.SetFrameGroup( HashString::ProbeMineUpThrust16x16x5, 1.0f );
							}
							if (rand()%8==0)
							{
								ProbeMineTail.SetGravity(0.975f);
								m_ExhaustContainer->push_back(ProbeMineTail);	
							}

							HaveLockOnSoForgetTheRest = true;  // this does not skip the collision check - might be in contact with somehting else at the same time as spotting new pray as it only pick the first thing it sees.
						}

						// Lock onto ships passing on the Y-axis
						if ( fabs( ProbeMineIter->GetY() - BadIter->GetY() ) < ScanRange )
						{
							// rocket tail
							Vessel ProbeMineTail = *ProbeMineIter;
							if (ProbeMineIter->GetX() > BadIter->GetX() )
							{
								ProbeMineIter->AddVel(-ThrustPower, 0, 0);
								ProbeMineIter->SetFrame( ProbeMineIter->GetFrameStart() + 2); // use Right thruster
								ProbeMineTail.AddVel(2.0 + ((rand()%300)*0.01),0.5 - ((rand()%100)*0.01),0);
								ProbeMineTail.SetFrameGroup( HashString::ProbeMineRightThrust16x16x5, 1.0f );
							}
							else
							{
								ProbeMineIter->AddVel(ThrustPower, 0, 0);
								ProbeMineIter->SetFrame( ProbeMineIter->GetFrameStart() + 4); // use Left thruster
								ProbeMineTail.AddVel(-2.0 - ((rand()%300)*0.01),0.5 - ((rand()%100)*0.01),0);
								ProbeMineTail.SetFrameGroup( HashString::ProbeMineLeftThrust16x16x5, 1.0f );
							}
							if (rand()%8==0)
							{
								ProbeMineTail.SetGravity(0.975f);
								m_ExhaustContainer->push_back(ProbeMineTail);	
							}

							HaveLockOnSoForgetTheRest = true;
						}
					}
				}
			}
		}

		if (!bMineDeleted)
		{
			ProbeMineIter->VelReduce();
			ProbeMineIter->AddVelToPos();
			++ProbeMineIter;
		}
	}

	m_pWii->profiler_stop(&profile_ProbeMineLogic);
}

void GameLogic::GunShipLogic()
{
	m_pWii->profiler_start(&profile_GunShip);


	guVector ChaseTarget(m_CPUTarget);
	for (std::vector<Vessel>::iterator GunShipIter( m_GunShipContainer->begin()); GunShipIter!= m_GunShipContainer->end(); ++GunShipIter)
	{
		f32 Turn = GunShipIter->GetTurnDirection( &ChaseTarget );
		GunShipIter->AddFacingDirection( Turn * 0.002f );

		Turn = GunShipIter->GetTurnDirectionForTurret( &m_CPUTarget );
		GunShipIter->AddTurrentDirection( Turn * 0.25f);

		ChaseTarget = GunShipIter->GetPos();  // the next ship will now follow this one, and so on in the sequence.

		// gun ship firing		
		if (rand()%GunShipIter->GetFireRate() == 0)
		{
			Vessel Boom = *GunShipIter;
			Boom.SetSpeedFactor( 1.0f );

			Boom.SetFrameGroup(HashString::GunShipProjectileFrames,0.005f);
			Boom.SetGravity(1.0f);
			Boom.SetSpin( 0.25f );

			// rotate around ship origin first
			Boom.SetPos(Boom.GetX() - ( sin(GunShipIter->GetFacingDirection()) * (m_pWii->GetXmlVariable(HashString::TurretNo2ForGunShipOriginX)) ) , 
				Boom.GetY() - ( -cos(GunShipIter->GetFacingDirection()) * m_pWii->GetXmlVariable(HashString::TurretForGunShipOriginY) ) , 
				Boom.GetZ()  );

			if ( m_pWii->GetFrameCounter() & 1 ) // picks a gun port to fire from
			{	
				Boom.SetPos( Boom.GetX() - ( -sin(GunShipIter->GetFacingDirection()+(M_PI/2)) * (m_pWii->GetXmlVariable(HashString::TurretNo1ForGunShipOriginX)) ) , 
					Boom.GetY() - ( -cos(GunShipIter->GetFacingDirection()+(M_PI/2)) * m_pWii->GetXmlVariable(HashString::TurretForGunShipOriginY) ) , 
					Boom.GetZ()  );
			}
			else
			{
				Boom.SetPos( Boom.GetX() - ( sin(GunShipIter->GetFacingDirection()+(M_PI+(M_PI/2))) * (m_pWii->GetXmlVariable(HashString::TurretNo2ForGunShipOriginX)) ) , 
					Boom.GetY() - ( -cos(GunShipIter->GetFacingDirection()+(M_PI+(M_PI/2))) * m_pWii->GetXmlVariable(HashString::TurretForGunShipOriginY) ) , 
					Boom.GetZ()  );
			}	
			float dir = GunShipIter->GetTurrentDirection();
			float mx(sin( dir )* 32.0f);
			float my(cos( dir )* 32.0f);
			Boom.AddPos(mx * 0.45f, -my * 0.45f, 0);		
			
			f32 TurretLockOn = atan2( m_CPUTarget.x - Boom.GetX(), Boom.GetY() - m_CPUTarget.y  );

			float temp_mx(sin( TurretLockOn )* 32.0f);
			float temp_my(cos( TurretLockOn )* 32.0f);
			Boom.SetVel( temp_mx * GunShipIter->GetBulletSpeedFactor(), -temp_my * GunShipIter->GetBulletSpeedFactor(), 0);  

			m_ProjectileContainer->push_back(Boom);	
		}

		GunShipIter->SetVel( sin(GunShipIter->GetFacingDirection())*1.75,
							-cos(GunShipIter->GetFacingDirection())*1.75,0);
		GunShipIter->AddVelToPos();

		if (m_pWii->GetFrameCounter()&4)
		{
			Vessel Boom = *GunShipIter;
			Boom.SetFrameGroup( HashString::SmokeTrail16x16x10, 0.5f );
			Boom.SetGravity(0.995f);
			float dir = Boom.GetFacingDirection() + (5-rand()%11)*0.01;
			float mx(sin( dir )* -32.0f);
			float my(cos( dir )* -32.0f);
			Boom.SetPos( Boom.GetX() + mx, Boom.GetY() - my, Boom.GetZ()  );
			Boom.SetSpin( (1000 - (rand()%2000 )) * 0.00025f );
			Boom.AddVel( mx * 0.1f, -my *0.1f, 0);  // engine trail
			m_ExhaustContainer->push_back(Boom);	
		}
	}

	m_pWii->profiler_stop(&profile_GunShip);
}

void GameLogic::BadShipsLogic()
{
	m_pWii->profiler_start(&profile_SmallEnemies);

	for (std::vector<Vessel>::iterator BadIter(m_SmallEnemiesContainer->begin()); BadIter!= m_SmallEnemiesContainer->end(); ++BadIter )
	{
		f32 Turn = BadIter->GetTurnDirection( &m_CPUTarget );
		BadIter->AddFacingDirection( Turn * 0.025f );
		BadIter->VelReduce();
		BadIter->AddVelToPos(); 

		if (rand()%20==0)  // todo
		{
			Vessel Tail( *BadIter );  // copy the original's detials

			float dir = BadIter->GetFacingDirection();
			float mx = sin( dir )*0.35;
			float my = -cos( dir )*0.35;
			BadIter->AddVel( mx, my, 0 );

			// add thrust trail
			Tail.SetFrameGroup(HashString::ExplosionSmoke2Type16x16x10,0.25f);
			Tail.AddPos( -mx*4.0 ,-my*4.0,	0);
			Tail.AddVel( -mx*8.2 ,-my*8.2,	0);
			Tail.SetGravity(0.95f);
			m_ExhaustContainer->push_back(Tail);  //1st puff of smoke
			Tail.SetFrameGroup(HashString::ExplosionSmoke2Type16x16x10,0.25f);
			Tail.AddVel( -mx*4.2 ,my*4.2,	0);
			Tail.SetGravity(0.85f);
			m_ExhaustContainer->push_back(Tail); // 2nd puff
		}
	}

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
		BadIter->VelReduce();
		BadIter->AddVelToPos();  

		if (BadIter->GetFuel()>0)
		{					
			if ( ( ( BadIter->GetFuel()&0x01 ) ==1) && ( (rand()%2) == 0) )
			{
				Vessel Boom = *BadIter;
				Boom.SetVel(0,0,0);
				Boom.SetFrameGroup(HashString::ExplosionFire2Type16x16x9,0.15f);
			//	Boom.SetGravity(0.95);
				Boom.SetSpin( (1000 - (rand()%2000 )) * 0.000075f );
				Boom.SetAlpha( max( 0, 255 - (int)(Boom.GetZ() * 1.5f )) ); // fudge
				m_ExhaustContainer->push_back(Boom);
			}
			BadIter->ReduceFuel();
			++BadIter;
		}
		else
		{
			BadIter = m_DyingEnemiesContainer->erase( BadIter );
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
		const static float r = 2.5f; 
		float ang = (float)i * ((M_PI*2.0f) / (float)Amount);
		float mx = sin(ang)*(r) ;
		float my = cos(ang)*(r) ;

		if (Amount>1)
			PickUp.SetVel(mx+Position->GetVelX(),my+Position->GetVelY(),0);
		else
			PickUp.SetVel(Position->GetVelX(),Position->GetVelY(),0);

		PickUp.SetRotateAmount(	0,0.005f,0.015f);
		m_pMaterialPickUpContainer->push_back(PickUp);
	}
}

void GameLogic::PickUpsLogic()
{
	Vessel* pPlayerShip = GetPlrVessel();
	for (std::vector<Item3D>::iterator iter(m_pMaterialPickUpContainer->begin()); iter!= m_pMaterialPickUpContainer->end(); /*nop*/ )
	{
		//iter->MulScale(0.9975f);
		iter->MulScale(0.995f);
		iter->Rotate();

		if (iter->GetScaleX()<0.45f)
		{
			if (iter->GetScaleX()<0.05f)
			{
				iter = m_pMaterialPickUpContainer->erase(iter);
				continue;
			}
			if (pPlayerShip->InsideRadius(iter->GetX(), iter->GetY(),200*200))
			{
				float mx( pPlayerShip->GetX() - iter->GetX() );
				float my( pPlayerShip->GetY() - iter->GetY() );
				float dir( atan2( mx,my ) );
				float dx = sin( dir )*0.085f;
				float dy = cos( dir )*0.085f;
				iter->AddVel( dx, dy, 0 );

				if (pPlayerShip->InsideRadius(iter->GetX(), iter->GetY(),30*30))  // this could be optimised... x2 calls to insideRadius
				{
					iter = m_pMaterialPickUpContainer->erase(iter);
					GetPlrVessel()->AddToPickUpTotal(iter->GetScaleX()*10.0f);
					AddScore(15);// todo ... for each player!!!!
					continue;
				}
			}
		}
		iter->AddVelToPos();
		iter->ReduceVel(0.99f);
		iter++;
	}
}

void GameLogic::ClearBadContainer()
{ 
	m_SmallEnemiesContainer->clear(); 
}

void GameLogic::AddScore(u32 Value) 
{
	if ( GetPlrVessel()->IsShieldOk() )
		m_Score += Value ; 
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
	VectorOfPointXYZ Points = m_pWii->Render.GetModelPointsFromLayer("SmallGunTurret[BONE]"); // layer 3 has the needed bone points
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
//		float TotalShotDist = sqrt(Shot_dist);		
		

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
			iter->SetTimerMillisecs( (rand()%1000) + 2000 );
		
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


				pShot.InitTimer();
				pShot.SetTimerMillisecs( 8500 );
				m_ShotForGunTurretContainer->push_back(pShot); // fire shot

				// barrel effect
				Vessel Boom(&pShot, 0.82f);
				Boom.SetFrameGroup( HashString::SmokeTrail16x16x10, 0.5f );
				m_ExhaustContainer->push_back(Boom);	
			}
		}
	}
}

void GameLogic::GunTurretShotsLogic( std::vector<Vessel>* pEnemy )
{
	for (std::vector<Item3D>::iterator iter(m_ShotForGunTurretContainer->begin()); iter!= m_ShotForGunTurretContainer->end(); /*NOP*/  )
	{
		iter->GetPos();

		if ( iter->IsTimerDone() )
		{
			iter = m_ShotForGunTurretContainer->erase(iter);  // remove shot from list
			continue;
		}
		
		bool hit=false;
		iter->AddVelToPos();

		//		for (std::vector<Vessel>::iterator GunShipIter(m_SmallEnemiesContainer->begin()); GunShipIter!=m_SmallEnemiesContainer->end(); ++GunShipIter)
		for (std::vector<Vessel>::iterator GunShipIter(pEnemy->begin()); GunShipIter!=pEnemy->end(); ++GunShipIter)
		{	
			float RadiusSquare (iter->GetSqaureRadius(GunShipIter->GetX(),GunShipIter->GetY()));
			if (RadiusSquare < GunShipIter->GetRadius() ) 
			{
				////Vessel Boom;  // to be fill from a Item3D... can't do a copy here
				////Boom.SetPos(iter->GetPos());
				////Boom.SetFrameGroup(HashString::RedEdgeExplosion64x64,0.05f);
				////Boom.SetVel(iter->GetVel());
				////Boom.SetGravity(0.92f);
				////Boom.SetSpin( (1000 - (rand()%2000 )) * 0.00025f );
				////m_ExplosionsContainer->push_back(Boom);

				AddScalingAnim(HashString::RedEdgeExplosion64x64, 
						&(*GunShipIter),
						0.015f,									// frame speed
						( 800 - ( rand()%1600) ) * 0.00025f,	// Spin Amount
						2.45f + ((rand()%25)*0.01f),				// Top Scale
						0.001f,									// Start Scale
						0.02f );								// Scale Factor


				GunShipIter->AddShieldLevel(-1);

				GunShipIter->AddVel(iter->GetVelX()*0.5f,iter->GetVelY()*0.5f,0);

				iter = m_ShotForGunTurretContainer->erase(iter); // remove shot from list
				iter->SetLockOntoVesselIndex(-1);
				hit=true;
				break;
			}
		}

		if (!hit)
		{
			++iter;
		}
	}
}

void GameLogic::MoonRocksLogic( )
{
	//m_pWii->profiler_start(&profile_MoonRocks);

	for (std::vector<Item3D>::iterator iter(m_pMoonRocksContainer->begin()); iter!= m_pMoonRocksContainer->end(); ++iter)
	{
		iter->Rotate(); 
	}

	//m_pWii->profiler_stop(&profile_MoonRocks);
}


void GameLogic::AddEnemy(float x, float y, HashLabel ShipType)
{
	AddEnemy( x,y, 0,1, ShipType);
}

void GameLogic::AddEnemy(int OriginX, int OriginY, int Amount, HashLabel ShipType, float Radius)
{
	for (int i=0; i<Amount; ++i)
	{
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
	BadVessel.SetFrameGroup(ShipType,1);
	BadVessel.SetFireRate( 35 ); // not all Vessels use this value

	if (ShipType == HashString::GunShip)
	{
		BadVessel.SetGravity(1.0f);  // better prediction for turrets
		BadVessel.SetSpeedFactor( 1.0 );
		BadVessel.SetShieldLevel( 18 ); 
		BadVessel.SetRadius(46*46);
	}
	else if (ShipType==HashString::SmallRedEnemyShip16x16x2)
	{
		BadVessel.SetGravity(0.99f);
		BadVessel.SetSpeedFactor( 0.80f );
		BadVessel.SetShieldLevel( m_pWii->GetXmlVariable(HashString::BadShipType2MaxShieldLevel) );
		BadVessel.SetRadius(12*12);
	}
	else if (ShipType == HashString::SmallWhiteEnemyShip16x16x2)
	{
		BadVessel.SetGravity(0.99f);
		BadVessel.SetSpeedFactor( 1.00f );
		BadVessel.SetShieldLevel( m_pWii->GetXmlVariable(HashString::BadShipType1MaxShieldLevel) );
		BadVessel.SetRadius(12*12);
	}
	
	if (ShipType == HashString::GunShip)
		m_GunShipContainer->push_back(BadVessel);
	else
		m_SmallEnemiesContainer->push_back(BadVessel);

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
	for (std::vector<MoonItem3D>::iterator iter(m_CelestialBodyContainer->begin()); iter!= m_CelestialBodyContainer->end(); ++iter )
	{
		iter->Rotate();
	}
}

bool GameLogic::IsEnemyDestroyed()
{
	return (m_SporesContainer->empty() && m_SmallEnemiesContainer->empty() && m_GunShipContainer->empty());
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

bool GameLogic::IsBaseShieldOnline()
{
	return m_IsBaseShieldOnline;
}

// NOTE: Amiming for quick compile times, if any optimisitions are lost I don't care.
// This lot here been placed here so I can forward declare 'class Vessel' and avoid using #include "Vessel.h" in the declaration
int GameLogic::GetSporesContainerSize() const {  return (int)m_SporesContainer->size(); }
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
//	m_bAddMoreShipsFlag = false;
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
		Asteroid.SetScale( d+((rand()%10000)*0.000025f), 
			d+((rand()%10000)*0.000025f), 
			d+((rand()%10000)*0.000025f));


		float dist = (x*x) + (y*y) + (z*z);

		maxvalue = fmax(dist,maxvalue);
		minvalue = fmin(dist,minvalue);

		Asteroid.SetRotateAmount(	(5000-(rand()%10000)) * 0.000025f, 
			(5000-(rand()%10000)) * 0.000025f, 
			(5000-(rand()%10000)) * 0.000025f );

		m_pMoonRocksContainer->push_back(Asteroid);
	}

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
	GunShip.SetFrameGroup(HashString::GunShip, 1);
	GunShip.SetShieldLevel( 18+6 ); 
	GunShip.SetSpeedFactor( 0 );
	GunShip.SetFireRate( 35 );
	GunShip.SetBulletSpeedFactor( 0.12 );

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
//	m_bAddMoreShipsFlag = false;

	//----------------------------------------
	m_AsteroidContainer->clear();
	m_CelestialBodyContainer->clear();
	//----------------------------------------

	MoonItem3D Moon;
	Moon.SetPos(0,0,600-200);
	Moon.SetRotateAmount(0.005f,0.005f,0.005f);  //note: just RotateY is used in logic
	Moon.SetDetailLevel(Auto);
	Moon.SetAmountOfRocks(700);
	m_CelestialBodyContainer->push_back(Moon);
	//----------------------------------------
	Moon.SetPos(2000,-1700,4200);
	Moon.SetAmountOfRocks(98);
	Moon.SetDetailLevel(Low);
	m_CelestialBodyContainer->push_back(Moon);
	//----------------------------------------

	InitialiseMoonRocks(700, 0.003f);  //todo - use the largest value in m_CelestialBodyContainer

	static int Amount = 3;
	static float Distance = 800.0f;
	static float x = 0.0f;
	static float y = 120.0f; //220.0f;
	static float z = 600.0f;
	InitialiseSmallGunTurret( Amount, Distance, x, y, z, 3.14/3.0f );

	m_GunShipContainer->clear();
	m_SporesContainer->clear();
}


void GameLogic::Intro()
{
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
		m_CPUTarget.x = (sin(fff)*200);
		m_CPUTarget.y = -100;
	}

	if (m_ProbeMineContainer->empty())
	{
		m_ProbeMineContainer->clear();
		for (std::vector<Vessel>::iterator BadIter(m_SmallEnemiesContainer->begin()); BadIter!=m_SmallEnemiesContainer->end(); ++BadIter)
		{
			BadIter->SetShieldLevel(0);
		}

		Vessel ProbeMine;
		ProbeMine.SetFrameGroup(HashString::ProbeMine16x16x5,1);
		ProbeMine.SetGravity(0.985f);
		ProbeMine.SetVel( 0, 1.15f, 0);

		// Scan in the Tiny logo for mine layout
		WiiManager::RawTgaInfo Info = m_pWii->m_RawTgaInfoContainer[(HashLabel)"TinyLogoForMineIntroLayout"];
		Tga::PIXEL* pData = Info.m_pTinyLogo;

		for (int y(0); y<Info.m_pTinyLogoHeader.height; ++y)
		{
			for (int x(0); x<Info.m_pTinyLogoHeader.width; ++x)
			{
				Tga::PIXEL Value( pData[x + (y*Info.m_pTinyLogoHeader.width)] );
				if (Value.b==0)  // logo on white background, using any RBG value will work
				{
					ProbeMine.SetPos(x*9-240,y*9-300,0); 
					m_ProbeMineContainer->push_back(ProbeMine);
				}
			}
		}
	}

	if ((int)m_SmallEnemiesContainer->size() <= 25 )
	{
		if ((int)m_SmallEnemiesContainer->size() <= 50)
		{
		//	for (int i=0; i< 5; ++i)  // add just a few
			//at 60 fps this is going to happan in under a second - one at a time is fine
			// this why there are no sudden jump in CPU activity
			if ( (rand()%2) == 0)
				AddEnemy(300-(rand()%600),255+150,HashString::SmallWhiteEnemyShip16x16x2);
			else
				AddEnemy(300-(rand()%600),-(255+150),HashString::SmallWhiteEnemyShip16x16x2);
		}
	}

	BadShipsLogic();
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



	if (m_ProbeMineContainer->size() < 240)
		ProbeMineLogic(m_SmallEnemiesContainer, 0.0450f, 380.0f); // fast mines
	else
		ProbeMineLogic(m_SmallEnemiesContainer, 0.0035f, 555.0f); // slow mines - keeps intact logo for a while

	m_pWii->GetGameDisplay()->DisplayAllForIntro();
}


void GameLogic::InitMenu()
{
	m_CelestialBodyContainer->clear();
	MoonItem3D Moon;
	Moon.SetPos(-202,-94,400);
	Moon.SetRotateAmount(0.0015f,0.0015f,0.0015f);  //note: just RotateY is used in logic
	Moon.SetDetailLevel(High);
	Moon.SetAmountOfRocks(250);
	m_CelestialBodyContainer->push_back(Moon);
}

void GameLogic::InitialiseGame()
{
	m_pWii->GetCamera()->InitialiseCamera(); // 3D View

	m_pImageManager = (m_pWii->GetImageManager());

	srand ( 2345725); //time(NULL) );

	//----------------------------------------------------------------------
	GetPlrVessel()->ClearPickUpTotal();
	GetPlrVessel()->SetFacingDirection( m_pWii->GetXmlVariable(HashString::PlayerFacingDirection) );
	GetPlrVessel()->SetPos( m_pWii->GetXmlVariable(HashString::PlayerStartingPointX),m_pWii->GetXmlVariable(HashString::PlayerStartingPointY),0.0f );
	m_pWii->GetCamera()->SetCameraView(GetPlrVessel()->GetX(),GetPlrVessel()->GetY());
	GetPlrVessel()->SetVel( m_pWii->GetXmlVariable(HashString::PlayerStartingVelocityX), m_pWii->GetXmlVariable(HashString::PlayerStartingVelocityY),0.0f );
	GetPlrVessel()->SetShieldLevel( m_pWii->GetXmlVariable(HashString::PlayerMaxShieldLevel) );

	GetPlrVessel()->SetFrameGroup(HashString::PlayersShip32x32, 1.0f);
	GetPlrVessel()->SetGravity( 0.995f );
	//GetPlrVessel()->SetGoingBoom(false);
	//----------------------------
	// empty containers
	m_ShieldGeneratorContainer->clear();
	m_AsteroidContainer->clear();
	m_SporesContainer->clear();
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

	//----------------------------
	//initialise bad ships
	AddEnemy(0,0,m_pWii->GetConfigValueWithDifficultyApplied(HashString::AmountOfGunShipsAtStartUp),HashString::GunShip,2000);
	AddEnemy(0,0,m_pWii->GetConfigValueWithDifficultyApplied(HashString::AmountBadShips),HashString::SmallWhiteEnemyShip16x16x2,1200);
	AddEnemy(0,0,m_pWii->GetConfigValueWithDifficultyApplied(HashString::AmountBadShips),HashString::SmallRedEnemyShip16x16x2,1400);



	//Things
	for (int i=0; i<m_pWii->GetConfigValueWithDifficultyApplied(HashString::AmountBadSpores); ++i)
	{
		static const int Width (1000);
		static const int Height(1000);
		Vessel thing;
		thing.SetPos( Width - (rand()%(Width*2)), Height - (rand()%(Height*2)), 0 );
		thing.SetFrameGroupWithRandomFrame( HashString::SpinningSpore16x16x9, 0.35f);
		m_SporesContainer->push_back(thing);
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
		"Extirpate enemy vessels in close vicinity to your base.",
		"The close range threat has been removed.");

	pMissionManager->AddMissionData("Mission Two",
		"Locate & recover your missing terraforming satellites. \n \n"
		"Tip: Rendezvous with each for a short while to enable recovery.",
		"");

	pMissionManager->AddMissionData("Alert",
		"Your final shield satellite has been surrounded by the enemies Armada. \n"
		"Remove this threat and recover the last satellite to bring your terraforming shields online.",
		"All satellites have been recovered \n \nYour base's terraforming shields are now online.");

	pMissionManager->AddMissionData("Mission Three",
		"Collect x85 scrap parts from your encounters with the enemy. \n \n"
		"Collecting scrap will enable a ring of defence turrets to be build around the perimeter of your base.",
		"You have collected enough scrap for the defensive built to start.");

	pMissionManager->AddMissionData("Alert",
		"A vast enemy fleet has been detected \n \n"
		"Delay this threat until your defensive power can repel them, collect a further 155 scrap.",
		"Your base's defensive perimeter is now complete.");

	pMissionManager->AddMissionData("Mission Four",
		"Protect your moon base while the terraforming process creates a habitable atmosphere. \n  THIS LEVEL IS STILL IN DEVELOPMENT - IT CURRENTLY DOES NOT END!",
		"Your base's terraforming is now complete.");

	MoonItem3D Moon;
	Moon.SetPos(0,0,600);
	Moon.SetRotateAmount(0.005f,0.005f,0.005f);
	Moon.SetDetailLevel(Low);
	Moon.SetAmountOfRocks(120);  // TODO - things above need to work with this
	m_CelestialBodyContainer->push_back(Moon);

	//--------------
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
	//AddEnemy(0,0,1,HashString::SmallRedEnemyShip16x16x2,666);
	//InitialiseSmallGunTurret(4,700, 0,120,600, 3.14/4.0f );
}

