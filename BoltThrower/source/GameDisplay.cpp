#include <stdarg.h>   //  for things like va_list
#include <math.h>
#include <gccore.h>
#include <sstream>
#include <iomanip> //std::setw
#include <bitset> 

#include "Singleton.h"

#include "GameDisplay.h"
#include "WiiManager.h"
#include "Image.h"
#include "ImageManager.h"
#include "FontManager.h"
#include "Vessel.h"
#include "Util3D.h"
#include "HashString.h"
#include "mission.h"
#include "MessageBox.h"
#include "Camera.h"
#include "config.h"
#include "CullFrustum/FrustumR.h"
#include "Render3D.h"
#include "Thread.h"
#include "Timer.h"
#include "Draw_Util.h"
using namespace std;

GameDisplay::GameDisplay() : m_pWii(NULL), m_pGameLogic(NULL), m_pImageManager(NULL)
{
}

void GameDisplay::Init()
{
	m_pWii = Singleton<WiiManager>::GetInstanceByPtr();
	m_pGameLogic = m_pWii->GetGameLogic();
	m_pImageManager = m_pWii->GetImageManager();
	m_pFontManager = m_pWii->GetFontManager();
}

void GameDisplay::DisplayAllForIngame()
{
	Util::CalculateFrameRate();

	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
	m_pWii->GetSpaceBackground()->DrawImageXYZ(0,0, 12000 ,255, 0 ,28.0f );

	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	//m_pWii->GetCamera()->SetLightOn2();
	// 3D section

	
//	static float a=0;
//	a+=0.015f;
//	m_pWii->GetCamera()->SetLightAlpha( 128 - (sin(a)*128) );/


	if (m_pGameLogic->m_TerraformingCounter < 255.0f)
	{
		m_pWii->GetCamera()->SetLightAlpha(255);
		DisplayMoon(HashString::MoonHiRes);
	}
	//else
	//{
	//	printf("moon off");
	//}

	if (m_pGameLogic->m_TerraformingCounter > 0.0f)
	{
		m_pWii->GetCamera()->SetLightAlpha( m_pGameLogic->m_TerraformingCounter );
		DisplayMoon(HashString::Earth_medres);

//		printf("Terraforming %f",m_pGameLogic->m_TerraformingCounter);
	}


	m_pWii->GetCamera()->SetLightOn2();
	DisplayMoonShieldAndRocks();

	DisplayGunTurrets();
	DisplayShotForGunTurret();
	DisplayAsteroids();
	DisplayShieldGenerators();

	if ( m_pGameLogic->GetPlrVessel()->HasShieldFailed() )  
	{

		m_pWii->GetCamera()->SetLightOn(1,m_pWii->GetCamera()->GetCamX(),m_pWii->GetCamera()->GetCamY(),-1000);

		static const GXColor AmbientColour  = { 0xff, 0xff, 0xff, 0x1a }; 
		GX_SetChanAmbColor(GX_COLOR0A0,AmbientColour);
		m_pWii->GetCamera()->SetLightDiff(2, 
			(guVector) { m_pWii->GetCamera()->GetCamX(),m_pWii->GetCamera()->GetCamY(),-75.0f }, 
			20.0f,1.0f);

		DisplaySkull();
		m_pWii->GetCamera()->SetLightOff();
	}



	m_pWii->GetCamera()->SetLightOn();
	DisplayPickUps();

	m_pWii->GetCamera()->SetLightOff();
	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);

	DisplayExhaust();

	if ( m_pGameLogic->GetPlrVessel()->IsShieldOk() )  
	{
		//DisplayPlayer();

		// 2D section
		//our ship
		m_pImageManager->GetImage(m_pGameLogic->GetPlrVessel()->GetFrame())->DrawImageXYZ( 
			m_pGameLogic->GetPlrVessel()->GetX(), m_pGameLogic->GetPlrVessel()->GetY(),
			m_pGameLogic->GetPlrVessel()->GetZ(), 255, m_pGameLogic->GetPlrVessel()->GetFacingDirection()); 

		//red overloading shiled
		m_pImageManager->GetImage(HashString::ShieldRed)->DrawImageXYZ( m_pGameLogic->GetPlrVessel()->GetX(),
			m_pGameLogic->GetPlrVessel()->GetY(), m_pGameLogic->GetPlrVessel()->GetZ(), 100 - (m_pGameLogic->GetPlrVessel()->GetShieldLevel()), (rand()%(314*2)) * 0.01  );


		if (m_pGameLogic->GetPlrVessel()->GetHitCoolDownTimer()>0) {
			m_pImageManager->GetImage(HashString::ShieldBlue)->
				DrawImageXYZ( m_pGameLogic->GetPlrVessel()->GetX(), m_pGameLogic->GetPlrVessel()->GetY(), 0, 
				m_pGameLogic->GetPlrVessel()->GetHitCoolDownTimer(), (rand()%(314*2)) * 0.01 , 0.85f  );
		}

		//DEBUG
//		m_pImageManager->GetImage(HashString::ShieldBlue)->
//				DrawImageXYZ( m_pGameLogic->m_CPUTarget_Miss.x, m_pGameLogic->m_CPUTarget_Miss.y, 0, 
//				128 , (rand()%(314*2)) * 0.01 , 0.85f  );


		if ( (m_pGameLogic->m_TerraformingCounter>0) && m_pGameLogic->IsBaseShieldOnline() ) {
			Util3D::Trans(m_pWii->GetCamera()->GetCamX(), m_pWii->GetCamera()->GetCamY());
			m_pFontManager->DisplayTextCentre("Terraforming "+Util::NumberToString(m_pGameLogic->m_TerraformingCounter * 0.39215f )+"%", 0, 190,128 ,HashString::SmallFont);

		}
	}

	DisplayHealthPickUps();
	DisplayBadShips();
	DisplayGunShips();

	DisplaySporeThings(false);	// backgound
	DisplayEnemySatellite(); 
	DisplaySporeThings(true);	// foreground

	DisplayRadar();

	DisplayProbMines();
	DisplayProjectile();
	DisplayMissile();

	DisplayExplosions();

	DisplayScorePing();

	m_pWii->GetCamera()->StoreCameraView();
	m_pWii->GetCamera()->SetCameraView(m_pWii->GetScreenWidth()*0.5f, m_pWii->GetScreenHeight()*0.5f) ;
	m_pWii->GetPanelManager()->Show();
	m_pWii->GetCamera()->RecallCameraView();


	// Display Aim Pointer
	//	GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_FALSE);
	for (std::vector<guVector>::iterator iter(m_pGameLogic->GetAimPointerContainerBegin()); 
		iter!= m_pGameLogic->GetAimPointerContainerEnd(); ++iter)
	{
		m_pImageManager->GetImage( m_pWii->m_FrameEndStartConstainer[HashString::AimingPointer32x32].StartFrame )
			->DrawImageXYZ( iter->x,iter->y, 0, 255, m_pGameLogic->GetPlrVessel()->GetFacingDirection() );


		// SCREEN TEST - 640x480 visable?

		//		m_pImageManager->GetImage( m_pWii->m_FrameEndStartConstainer[HashString::AimingPointer32x32].StartFrame )
		//		->DrawImageXYZ( iter->x-320,iter->y-240, 0, 255, 0);
		//m_pImageManager->GetImage( m_pWii->m_FrameEndStartConstainer[HashString::AimingPointer32x32].StartFrame )
		//		->DrawImageXYZ( iter->x+320,iter->y+240, 0, 255, 0);
		//m_pImageManager->GetImage( m_pWii->m_FrameEndStartConstainer[HashString::AimingPointer32x32].StartFrame )
		//		->DrawImageXYZ( iter->x-320,iter->y+240, 0, 255, 0);
		//m_pImageManager->GetImage( m_pWii->m_FrameEndStartConstainer[HashString::AimingPointer32x32].StartFrame )
		//		->DrawImageXYZ( iter->x+320,iter->y-240, 0, 255, 0);
	}


	//
	// note to myself ... maybe use this for all messages 
	//
	if ( !m_pGameLogic->GetPlrVessel()->m_PopUpMessageTimer.IsTimerDone() ) {
		if ( m_pWii->GetFrameCounter()&32 ) {
			m_pFontManager->SetFontColour(255,0,0, 144);
			Util3D::Trans(m_pWii->GetCamera()->GetCamX(), m_pWii->GetCamera()->GetCamY());
			m_pFontManager->DisplayTextCentre("WARNING - Sheild Low", 0, 90,100 ,HashString::LargeFont);
			m_pFontManager->SetFontColour(255,255,255, 255);
		}
	}


	//	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);

	m_pWii->GetMessageBox()->DoMessageBox();

	if (m_pGameLogic->IsGamePausedByPlayer())
	{
		static float ccc(0);
		ccc+=0.0176;
		Util3D::TransRot(m_pWii->GetCamera()->GetCamX(), m_pWii->GetCamera()->GetCamY(), 0,(sin(ccc))*0.025f);
		Draw_Util::DrawRectangle( -200, -65, 400,  130, 128, 0,0,30 );
		m_pFontManager->DisplayTextCentre(m_pWii->GetText("GAME_PAUSED"), 0, -20,160+(cos(ccc*4)*50.0f),HashString::LargeFont);
		m_pFontManager->DisplayTextCentre(m_pWii->GetText("Press_PLUS_To_Play"),0,20+5, 255,HashString::SmallFont);
	}
	DebugInformation();
}

void GameDisplay::DisplayShieldGenerators() 
{
	if (m_pGameLogic->IsShieldGeneratorContainerEmpty())
		return;

	Vessel* pPlayerVessel( m_pGameLogic->GetPlrVessel() );
	bool bStillOneLeftToSalvaged( m_pGameLogic->IsJustOneShiledSatelliteLeftToSalvaged() );
	bool bGunShipContainerEmpty( m_pGameLogic->IsGunShipContainerEmpty() );
	bool bMessageBoxEnabled( m_pWii->GetMessageBox()->IsEnabled() );

	for (std::vector<Item3DChronometry>::iterator iter(m_pGameLogic->GetShieldGeneratorContainerBegin()); iter!= m_pGameLogic->GetShieldGeneratorContainerEnd() ; ++iter)
	{
		if (pPlayerVessel->InsideRadius(iter->GetX(), iter->GetY(),60*60))
		{
			if ( iter->GetVelZ()==0 ) 
			{
				if ( (bStillOneLeftToSalvaged) && (!bGunShipContainerEmpty) && (!bMessageBoxEnabled) )
				{
					// can't collect - enemy about
					Mission& MissionData( m_pWii->GetMissionManager()->GetMissionData() );
					// need something better than this... will work for the mo
					if (MissionData.GetCompleted() == 1)
					{
						MissionData.SetCompleted(2); 
						m_pWii->GetMessageBox()->SetUpMessageBox(m_pWii->GetText("ExplanationMark"),m_pWii->GetText("RemoveGunshipsBeforeRecoveringMessage"));
					}
				}
				else
				{
					// show a blue disc
					m_pWii->GetCamera()->SetLightOff();
					GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
					m_pImageManager->GetImage(m_pWii->m_FrameEndStartConstainer[HashString::ShieldBlue].StartFrame)->
						DrawImageXYZ( iter->GetX(), iter->GetY(), iter->GetZ(), 200 , (rand()%(314*2)) * 0.01 , 2.5f  );
					GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
					m_pWii->GetCamera()->SetLightOn2();
				}
			}
		}
	}

	m_pWii->GetRender3D()->RenderModelPreStage(HashString::Satellite);  // rock1 & rock2 use the same texture
	for (std::vector<Item3DChronometry>::iterator iter(m_pGameLogic->GetShieldGeneratorContainerBegin() ); iter!= m_pGameLogic->GetShieldGeneratorContainerEnd(); /*NOP*/)
	{
		// set Satellite
		iter->Rotate(); //TODO this is a logic only part
		iter->AddVelToPos();
		Mtx Model,mat;
		Util3D::MatrixRotateZ(Model, iter->GetRotateZ());
		Util3D::MatrixRotateY(mat, iter->GetRotateY());
		guMtxConcat(mat,Model,Model);
		guMtxScaleApply(Model,Model,iter->GetScaleX(),iter->GetScaleY(),iter->GetScaleZ());
		guMtxTrans(mat, iter->GetX(), iter->GetY(), iter->GetZ());
		guMtxConcat(mat,Model,Model);
		guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,mat);
		m_pWii->GetRender3D()->RenderModelMinimalHardNorms(HashString::Satellite, mat);

		if (pPlayerVessel->InsideRadius(iter->GetX(), iter->GetY(),60*60))
		{
			if ( bGunShipContainerEmpty ) // can't coolect while enemy are about
			{
				iter->DampRotation(0.985f);	
				if (iter->IsCountdownFinished())
				{
					iter->SetVelZ(-2.5f);
				}
			}
		}
		else
		{
			iter->SetCountdownSeconds(4);
		}

		if (iter->GetZ() < -485.0f )
			iter = m_pGameLogic->EraseItemFromShieldGeneratorContainer(iter);
		else
			++iter;
	}
}

void GameDisplay::DisplayPickUps()
{
	m_pWii->GetRender3D()->RenderModelPreStage(HashString::Material_PickUp); 

	for (std::vector<Item3D>::iterator iter(m_pGameLogic->GetMaterialPickUpContainerBegin()); 
		iter!= m_pGameLogic->GetMaterialPickUpContainerEnd(); ++iter)
	{
		Mtx Model,mat,m2;

		Util3D::MatrixRotateZ(Model, iter->GetRotateZ());
		Util3D::MatrixRotateY(mat, iter->GetRotateY());
		guMtxConcat(mat,Model,m2);
		guMtxScaleApply(m2,Model,iter->GetScaleX(),iter->GetScaleY(),iter->GetScaleZ());
		guMtxTransApply(Model,Model,iter->GetX(), iter->GetY(), iter->GetZ());
		guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,mat);
		m_pWii->GetRender3D()->RenderModelMinimalHardNorms(HashString::Material_PickUp, mat);
	}
}


void GameDisplay::DisplayHealthPickUps()
{
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );
	float Radius(m_pWii->GetXmlVariable(HashString::ViewRadiusForSprites));
	Radius*=Radius;

	for (std::vector<Vessel>::iterator Iter(m_pGameLogic->GetHealthPickUpContainerBegin()); 
		Iter!= m_pGameLogic->GetHealthPickUpContainerEnd(); ++Iter)
	{
		if ( Iter->InsideRadius(fCamX, fCamY, Radius ) )
		{
			m_pImageManager->GetImage(Iter->GetFrame())->DrawImage(*Iter);
		}
	}
}

void GameDisplay::DisplayMoonShieldAndRocks()
{
	extern profiler_t profile_MoonRocks;
	m_pWii->profiler_start(&profile_MoonRocks);

	FrustumR* pFrustum( m_pWii->GetFrustum() );

	for (std::vector<MoonItem3D>::iterator MoonIter(m_pGameLogic->GetCelestialBodyContainerBegin()); 
		MoonIter!= m_pGameLogic->GetCelestialBodyContainerEnd(); ++MoonIter )
	{
		Vec3 v( MoonIter->GetX(), MoonIter->GetY(), MoonIter->GetZ() );

// moon rocks
		if (pFrustum->sphereInFrustum(v,m_pGameLogic->GetClippingRadiusNeededForMoonRocks()) != FrustumR::OUTSIDE) {
			m_pWii->GetRender3D()->RenderModelPreStage(HashString::Rock1);  // rock1 & rock2 use the same texture
			std::vector<Item3D>::iterator IterEnd = m_pGameLogic->GetMoonRocksContainerBegin();
			std::advance(IterEnd,MoonIter->GetAmountOfRocks());

			for (std::vector<Item3D>::iterator iter(m_pGameLogic->GetMoonRocksContainerBegin());iter!=IterEnd;++iter)
			{
				Mtx Model,mat,m2;
				Util3D::MatrixRotateZ(Model, iter->GetRotateZ());
				Util3D::MatrixRotateY(mat, iter->GetRotateY());
				guMtxConcat(Model,mat, m2);
				guMtxScaleApply(m2, Model, iter->GetScaleX(),iter->GetScaleY(),iter->GetScaleZ());
				guMtxTransApply(Model, Model, iter->GetX(), iter->GetY(), iter->GetZ());

				Util3D::MatrixRotateY(mat, MoonIter->GetRotateY());  // spin around moon axis

				guMtxConcat(mat,Model,m2);
				guMtxTransApply(m2,m2, MoonIter->GetX(), MoonIter->GetY(), MoonIter->GetZ());
				guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),m2,Model);

				if (MoonIter->GetDetailLevel() == Low)
				{
					m_pWii->GetRender3D()->RenderModelMinimal(HashString::Rock2, Model);  //lowres
				}
				else if (MoonIter->GetDetailLevel() == High)
				{
					m_pWii->GetRender3D()->RenderModelMinimal(HashString::Rock1, Model); 
				}
				else if (MoonIter->GetDetailLevel() == Auto)
				{
					if (( fabs(iter->GetZ()) > 750)  ||( fabs(iter->GetX()) > 750))
						m_pWii->GetRender3D()->RenderModelMinimal(HashString::Rock1, Model);  //hires
					else
						m_pWii->GetRender3D()->RenderModelMinimal(HashString::Rock2, Model);  //lowres
				}
			}
		}


		if (m_pGameLogic->IsBaseShieldOnline()) {
			float r = m_pWii->GetRender3D()->GetDispayListModelRadius(HashString::MoonShield);  // just using MoonShield for anything inside it, i.e the moon
			if (pFrustum->sphereInFrustum(v,r) != FrustumR::OUTSIDE) {
				// setup same pos as each Celestial Body
				Mtx Model; //,mat;
				Util3D::MatrixRotateY(Model, MoonIter->GetRotateY());
				guMtxTransApply( Model, Model , MoonIter->GetX(), MoonIter->GetY(), MoonIter->GetZ() );  // distance back
				guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
			
				// moon shield
				GX_SetCullMode(GX_CULL_NONE);
				GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
				Util3D::MatrixRotateY(Model, MoonIter->GetRotateY()*8);
				guMtxTransApply(Model,Model, 0, 0, MoonIter->GetZ());
				guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
				m_pWii->GetRender3D()->RenderModelHardNorms(HashString::MoonShield, Model);
				GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
				GX_SetCullMode(GX_CULL_BACK);
			}
		}
		
	}

	m_pWii->profiler_stop(&profile_MoonRocks);
}


void GameDisplay::DisplayMoon(HashLabel ModelName)
{
//	extern profiler_t profile_MoonRocks;
//	m_pWii->profiler_start(&profile_MoonRocks);

	FrustumR* pFrustum( m_pWii->GetFrustum() );

	for (std::vector<MoonItem3D>::iterator MoonIter(m_pGameLogic->GetCelestialBodyContainerBegin()); 
		MoonIter!= m_pGameLogic->GetCelestialBodyContainerEnd(); ++MoonIter )
	{
		Vec3 v( MoonIter->GetX(), MoonIter->GetY(), MoonIter->GetZ() );
		float r = m_pWii->GetRender3D()->GetDispayListModelRadius(HashString::MoonShield);  // just using MoonShield for anything inside it, i.e the moon
		if (pFrustum->sphereInFrustum(v,r) != FrustumR::OUTSIDE) {
			Mtx Model;
			Util3D::MatrixRotateY(Model, MoonIter->GetRotateY());
			guMtxTransApply( Model, Model , MoonIter->GetX(), MoonIter->GetY(), MoonIter->GetZ() );  // distance back
			guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
			m_pWii->GetRender3D()->RenderModel( ModelName,  Model);
		}
	}

//	m_pWii->profiler_stop(&profile_MoonRocks);
}

void GameDisplay::Display3DInfoBar(float x , float y, std::string Message, float Tilt)
{
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	m_pWii->GetCamera()->DoDefaultLight(150, 75, -200);
	//--------------------------
	Mtx Model;//,mat;
	Util3D::MatrixRotateX( Model,Tilt);
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );
	float movex = Tilt *112.0f;
	guMtxTransApply(Model,Model,fCamX + x - movex ,fCamY + y, 0);
	guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
	m_pWii->GetRender3D()->RenderModelHardNorms(HashString::BarRightSmall, Model);
	//--------------------------
	m_pWii->GetCamera()->SetLightOff();
	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
	m_pFontManager->DisplayText( Message,-48,-11,144,HashString::SmallFont);
}


void GameDisplay::DisplayPlayer()
{
	PlayerVessel* pPlayerVessel( m_pGameLogic->GetPlrVessel() );

	//=========================
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	m_pWii->GetCamera()->SetVesselLightOn(pPlayerVessel->GetX(), pPlayerVessel->GetY(), pPlayerVessel->GetZ() - 100000);
	//--------------------------
	Mtx Model,mat;
	guMtxIdentity(Model);
	guMtxRotRad(Model,'x', -M_PI/2 );

	guMtxRotRad(mat,'y',0 );// -pPlayerVessel->GetLastValueAddedToFacingDirection()*8 );
	guMtxConcat(mat,Model,Model);

	guMtxRotRad(mat,'z', pPlayerVessel->GetFacingDirection() + M_PI/2 );
	guMtxConcat(mat,Model,Model);

	guMtxScaleApply(Model,Model,0.10,0.10,0.10);
	guMtxTrans(mat, pPlayerVessel->GetX(), pPlayerVessel->GetY(), pPlayerVessel->GetZ() );
	guMtxConcat(mat,Model,Model);
	guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
	m_pWii->GetRender3D()->RenderModelHardNorms(HashString::WiiMote, Model);
	//--------------------------
	m_pWii->GetCamera()->SetLightOff();
	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
	//=====================
}

void GameDisplay::DisplaySkull()
{
	static const float dist = -45;
	static float bbb = 0;
	bbb+=0.025f;

	Mtx Model,mat;
	Util3D::MatrixRotateZ(Model, cos(bbb)*0.45);
	Util3D::MatrixRotateX(mat, (M_PI/10.0f) + sin(-bbb*4)*0.35);
	guMtxConcat(mat,Model,Model);
	Util3D::MatrixRotateY(mat, sin(bbb)*0.95);
	guMtxConcat(mat,Model,Model);
	//	guMtxTransApply(Model,Model, 
	//		m_pGameLogic->GetPlrVessel()->GetX(), 
	//		m_pGameLogic->GetPlrVessel()->GetY(),dist);


	guMtxTransApply(Model,Model,m_pWii->GetCamera()->GetCamX(),m_pWii->GetCamera()->GetCamY(),dist);

	guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);

	m_pWii->GetRender3D()->RenderModelHardNorms(HashString::Skull, Model);
}

void GameDisplay::DisplayRadar() // big and messy...needs a refactor
{
	Vessel* pPlayerShip( m_pGameLogic->GetPlrVessel() );

	static const float scale (0.02f);
	static const float scale2 (540.0f);
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );

	static float size;
	size+=0.0085f;
	if (size>1.0f)
		size=0;

	for (std::vector<Vessel>::iterator Iter(m_pGameLogic->GetEnemySatelliteContainerBegin()); Iter!= m_pGameLogic->GetEnemySatelliteContainerEnd(); ++Iter ){
		if ( Iter->InsideRadius(fCamX, fCamY, (120*120)*scale2 ) ){
			m_pWii->GetImageManager()->GetImage(HashString::YellowRadarPing32x32)
				->DrawImageXYZ(fCamX - (pPlayerShip->GetX()*scale) - (320-64) + (Iter->GetX()*scale), 
				fCamY - (pPlayerShip->GetY()*scale) - (240-64) + (Iter->GetY()*scale), 0, (255) - (size*255), 0, size );
		}
	}

	m_pWii->GetImageManager()->GetImage(HashString::RadarCircle)->DrawImageXYZ(fCamX - (320-64), fCamY - (240-64),0,128,0,2.0f);

	// this next bit is relative to last draw
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

	// HV lines
	GX_Begin( GX_LINES, GX_VTXFMT3,4 );	
	GX_Position3f32(-63, 0, 0);		
	GX_Color4u8(255,255,255,32); 
	GX_Position3f32(+63, 0, 0);		
	GX_Color4u8(255,255,255,32);  
	GX_Position3f32(0, -63, 0);		
	GX_Color4u8(255,255,255,32); 
	GX_Position3f32(0, +63, 0);		
	GX_Color4u8(255,255,255,32); 
	GX_End();

	Util3D::Trans(	fCamX - pPlayerShip->GetX()*scale - (320-64), fCamY - pPlayerShip->GetY()*scale - (240-64),0);

	{
		float square_dist = ((0-fCamX)*(0-fCamX) ) + ((0-fCamY)*(0-fCamY)) ;
		if ( fabs(square_dist) < ((128*128)*scale2) )
		{
			m_pWii->GetImageManager()->GetImage(HashString::MiniMoon16x16)->DrawImageTL(-8,-8,128);  // 16x16 image
		}
	}

	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);// the rest of this section uses this gx setup
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);


	// with any luck the hardware might just optimise some of these out as they use Alpha ZERO.
	// No point being clever here, still have the worst case so just make it ZERO ALPHA when outside the radar.

	GX_Begin( GX_POINTS, GX_VTXFMT3, m_pGameLogic->GetSmallEnemiesContainerSize() );	
	for (std::vector<Vessel>::iterator Iter(m_pGameLogic->GetSmallEnemiesContainerBegin()); 
		Iter!= m_pGameLogic->GetSmallEnemiesContainerEnd(); ++Iter )
	{
		int Alpha = 0; 
		if ( ( Iter->InsideRadius(fCamX, fCamY, (128*128)*scale2 ) ) && Iter->IsShieldOk() )
			Alpha=188; 

		GX_Position3f32(Iter->GetX()*scale, Iter->GetY()*scale, 0);		
		GX_Color4u8(230,230,230,Alpha);        
	}
	GX_End();

	{
		GX_Begin( GX_POINTS, GX_VTXFMT3,m_pGameLogic->GetSporesContainerSize() );	
		for (std::vector<Vessel>::iterator Iter(m_pGameLogic->GetSporesContainerBegin()); 
			Iter!= m_pGameLogic->GetSporesContainerEnd(); ++Iter )
		{
			GX_Position3f32(Iter->GetX()*scale, Iter->GetY()*scale, 0);		
			if ( Iter->InsideRadius(fCamX, fCamY, (128*128)*scale2 ) )
				GX_Color4u8(255,64,22,255);   
			else
				GX_Color4u8(255,64,22,0); 
		}
		GX_End();
	}


	{
		int AlphaFlash = 230;  
		if ( m_pWii->GetFrameCounter() & 0x10)  // make these flash on/off
			AlphaFlash = 100;  

		GX_Begin( GX_POINTS, GX_VTXFMT3,m_pGameLogic->GetEnemySatelliteContainerSize() * 2);	
		for (std::vector<Vessel>::iterator Iter(m_pGameLogic->GetEnemySatelliteContainerBegin()); 
			Iter!= m_pGameLogic->GetEnemySatelliteContainerEnd(); ++Iter )
		{
			u8 Alpha = 0;
			if ( Iter->InsideRadius(fCamX, fCamY, (128*128)*scale2 ) )
				Alpha=AlphaFlash;   // its inside the radar scope

			GX_Position3f32(Iter->GetX()*scale, Iter->GetY()*scale, 0);		
			GX_Color4u8(255,64,22,Alpha);   
			GX_Position3f32(Iter->GetX()*scale, Iter->GetY()*scale + 1, 0);		
			GX_Color4u8(255,64,22,Alpha);  
		}
		GX_End();
	}




	GX_Begin( GX_POINTS, GX_VTXFMT3,m_pGameLogic->GetProbeMineContainerSize() );	
	for (std::vector<Vessel>::iterator Iter(m_pGameLogic->GetProbeMineContainerBegin()); 
		Iter!= m_pGameLogic->GetProbeMineContainerEnd(); ++Iter )
	{
		int Alpha = 0;  
		if ( Iter->InsideRadius(fCamX, fCamY, (128*128)*scale2 ) )
			Alpha=88;   // its inside the radar scope

		GX_Position3f32(Iter->GetX()*scale, Iter->GetY()*scale, 0);		
		GX_Color4u8(0,255,0,Alpha);        
	}
	GX_End();

	GX_Begin( GX_POINTS, GX_VTXFMT3,m_pGameLogic->GetMissileContainerSize() );	
	for (std::vector<Vessel>::iterator Iter(m_pGameLogic->GetMissileContainerBegin()); 
		Iter!= m_pGameLogic->GetMissileContainerEnd(); ++Iter )
	{
		// with any luck the hardware might just optimise this out and not even bother drawing it
		int Alpha = 0;  // No point being cleaver here as we still have the worst case, so just make it invisable
		if ( Iter->InsideRadius(fCamX, fCamY, (128*128)*scale2 ) )
			Alpha=100;   // its inside the radar scope

		GX_Position3f32(Iter->GetX()*scale, Iter->GetY()*scale, 0);		
		GX_Color4u8(250,100,0,Alpha);        
	}
	GX_End();

	GX_Begin( GX_POINTS, GX_VTXFMT3,m_pGameLogic->GetShotForGunTurretContainerSize() );	
	for (std::vector<Item3D>::iterator Iter(m_pGameLogic->GetShotForGunTurretContainerBegin()); 
		Iter!= m_pGameLogic->GetShotForGunTurretContainerEnd(); ++Iter )
	{
		// with any luck the hardware might just optimise this out and not even bother drawing it
		int Alpha = 0;  // No point being cleaver here as we still have the worst case, so just make it invisable
		if ( Iter->InsideRadius(fCamX, fCamY, (128*128)*scale2 ) )
			Alpha=100;   // its inside the radar scope

		GX_Position3f32(Iter->GetX()*scale, Iter->GetY()*scale, 0);		
		GX_Color4u8(250,20,10,Alpha);        
	}
	GX_End();


	{ 

		GX_Begin( GX_POINTS, GX_VTXFMT3,m_pGameLogic->GetGunShipContainerSize()*2 );	  // *2 for showing two pixel dots
		for (std::vector<Vessel>::iterator Iter(m_pGameLogic->GetGunShipContainerBegin()); 
			Iter!= m_pGameLogic->GetGunShipContainerEnd(); ++Iter )
		{
			int Alpha = 0; 
			if ( Iter->InsideRadius(fCamX, fCamY, (128*128)*scale2 ) )
				Alpha=200;

			GX_Position3f32(Iter->GetX()*scale, Iter->GetY()*scale, 0);		
			GX_Color4u8(255,255,255,Alpha);        
			GX_Position3f32(Iter->GetX()*scale, Iter->GetY()*scale + 1, 0);		
			GX_Color4u8(255,255,255,Alpha);        
		}
		GX_End();
	}	


	GX_Begin( GX_POINTS, GX_VTXFMT3,m_pGameLogic->GetShieldGeneratorContainerSize() * 4 );	
	for (std::vector<Item3DChronometry>::iterator Iter(m_pGameLogic->GetShieldGeneratorContainerBegin()); 
		Iter!= m_pGameLogic->GetShieldGeneratorContainerEnd(); ++Iter )
	{
		int Alpha = 0;  
		if ( Iter->InsideRadius(fCamX, fCamY, (128*128)*scale2 ) )
			Alpha=220;   // its inside the radar scope

		GX_Position3f32(Iter->GetX()*scale, Iter->GetY()*scale, 0);		
		GX_Color4u8(255,255,0,Alpha);
		GX_Position3f32(Iter->GetX()*scale + 1, Iter->GetY()*scale + 1, 0);		
		GX_Color4u8(255,255,0,Alpha);
		GX_Position3f32(Iter->GetX()*scale , Iter->GetY()*scale + 1, 0);		
		GX_Color4u8(255,255,0,Alpha);
		GX_Position3f32(Iter->GetX()*scale + 1, Iter->GetY()*scale, 0);		
		GX_Color4u8(255,255,0,Alpha);
	}
	GX_End();
}

void GameDisplay::DisplayEnemySatellite()
{
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );

	float fInsideScreenViewFactor = m_pWii->GetXmlVariable(HashString::ViewRadiusForSprites);
	fInsideScreenViewFactor*=fInsideScreenViewFactor;

	static float size;
	size+=0.0085f;
	if (size>1.0f)
		size=0;


	for (std::vector<Vessel>::iterator iter(m_pGameLogic->GetEnemySatelliteContainerBegin()); iter!= m_pGameLogic->GetEnemySatelliteContainerEnd(); ++iter) {
			if ( iter->InsideRadius(fCamX, fCamY, fInsideScreenViewFactor) ) {

				m_pImageManager->GetImage(iter->GetFrame())->DrawImage(*iter);


				// show a blue disc

				if (iter->GetHitCoolDownTimer()>0) {
					m_pImageManager->GetImage(HashString::ShieldBlue)->
						DrawImageXYZ( iter->GetX(), iter->GetY(), iter->GetZ(), iter->GetHitCoolDownTimer(), (rand()%(314*2)) * 0.01 , 2.5f  );
				}
			}
	}
}


void GameDisplay::DisplaySporeThings(bool bDrawInBackground)
{
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );
	float Rad = m_pWii->GetXmlVariable(HashString::ViewRadiusForSprites);
	Rad*=Rad;

	for (std::vector<Vessel>::iterator iter(m_pGameLogic->GetSporesContainerBegin()); 
		iter!= m_pGameLogic->GetSporesContainerEnd(); ++iter)
	{
		if ( (iter->GetVelX() <= 0) == bDrawInBackground )  { // use as background flag hack
			continue;
		}

		if ( iter->InsideRadius(fCamX, fCamY, Rad) )
		{
			iter->AddFrame(iter->GetFrameSpeed());
			if (iter->GetFrame() >= iter->GetEndFrame())
				iter->SetFrame(iter->GetFrameStart());

			m_pImageManager->GetImage(iter->GetFrame())->DrawImage(*iter);

			if (iter->GetID() != -1) {
				if (m_pWii->GetFrameCounter() & 8) {
					m_pImageManager->GetImage(HashString::GreenLockOn)->DrawImage(*iter);
				}
			}
		}
	}
}


void GameDisplay::DisplayAsteroids()
{
	extern profiler_t profile_Asteroid;
	m_pWii->profiler_start(&profile_Asteroid);

	float Radius = 640; //(m_pWii->GetXmlVariable(HashString::ViewRadiusForAsteroids));
	Radius*=Radius;

	FrustumR* pFrustum( m_pWii->GetFrustum() );

	Mtx& cameraMatrix = m_pWii->GetCamera()->GetcameraMatrix();

	m_pWii->GetRender3D()->RenderModelPreStage(HashString::Rock1);  // rock1 & rock2 use the same texture
	for (std::vector<Item3D>::iterator iter(m_pGameLogic->GetAsteroidContainerBegin()); 
		iter!= m_pGameLogic->GetAsteroidContainerEnd(); ++iter)
	{
		if ( !iter->GetEnable() )  
			continue; 

		Vec3 v(iter->GetX(),iter->GetY(),iter->GetZ());
		if (pFrustum->sphereInFrustum(v,6) != FrustumR::OUTSIDE)  // !!!  dynamic size needed
		{
			Mtx Model,mat;

			Util3D::MatrixRotateY(Model, iter->GetRotateY());
			Util3D::MatrixRotateX(mat, iter->GetRotateX());

			//guMtxRotRad(Model,'y', iter->GetRotateX() ); 
			//guMtxRotRad(mat,'x', iter->GetRotateY() );

			guMtxConcat(mat,Model,Model);

			guMtxScaleApply(Model,Model,iter->GetScaleX(),iter->GetScaleY(),iter->GetScaleZ());

			guMtxTransApply(Model,Model, iter->GetX(), iter->GetY(), iter->GetZ());
			guMtxConcat(cameraMatrix,Model,Model);
			m_pWii->GetRender3D()->RenderModelMinimal(HashString::Rock1, Model);
		}
	}

	m_pWii->profiler_stop(&profile_Asteroid);
}

void GameDisplay::DisplayProbMines()
{
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );
	float Radius(m_pWii->GetXmlVariable(HashString::ViewRadiusForIntroSprites));
	Radius*=Radius;

	for (std::vector<Vessel>::iterator iter(m_pGameLogic->GetProbeMineContainerBegin()); 
		iter!= m_pGameLogic->GetProbeMineContainerEnd(); ++iter)
	{	
		if ( iter->InsideRadius(fCamX, fCamY, Radius ) )
		{
			iter->AddFrame(iter->GetFrameSpeed());
			if ( (floor)(iter->GetFrame()) > iter->GetEndFrame() )
				iter->SetFrame(iter->GetFrameStart());

			m_pImageManager->GetImage((floor)(iter->GetFrame()))->DrawImage(*iter); 
		}
	}
}

void GameDisplay::DisplayProjectile()
{
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );
	float Radius(m_pWii->GetXmlVariable(HashString::ViewRadiusForSprites));
	Radius*=Radius;

	for (std::vector<Vessel>::iterator iter(m_pGameLogic->GetProjectileContainerBegin());
		iter!= m_pGameLogic->GetProjectileContainerEnd(); ++iter)
	{	
		if ( iter->InsideRadius(fCamX, fCamY, Radius ) )
		{
			Image* pImage = m_pImageManager->GetImage( (floor)(iter->GetFrame()) );
			pImage->DrawImageXYZ(iter->GetX(),iter->GetY(),iter->GetZ(),iter->GetAlpha(),iter->GetFacingDirection(),iter->GetCurrentScaleFactor());
		}
	}
}

void GameDisplay::DisplayMissile()
{
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );
	float Radius(m_pWii->GetXmlVariable(HashString::ViewRadiusForSprites));
	Radius*=Radius;

	for (std::vector<Vessel>::iterator MissileIter(m_pGameLogic->GetMissileContainerBegin()); 
		MissileIter!= m_pGameLogic->GetMissileContainerEnd(); ++MissileIter)
	{
		if ( MissileIter->InsideRadius(fCamX, fCamY, Radius ) )
		{
			m_pImageManager->GetImage(MissileIter->GetFrame())->DrawImage(*MissileIter);
		}
	}
}




void GameDisplay::DisplayExhaust()
{
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );
	float Radius(m_pWii->GetXmlVariable(HashString::ViewRadiusForSprites));
	Radius*=Radius;

	for (std::vector<Vessel>::iterator iter(m_pGameLogic->GetExhaustContainerBegin()); iter!= m_pGameLogic->GetExhaustContainerEnd(); ++iter)
	{	
		if ( iter->InsideRadius(fCamX, fCamY, Radius ) )
		{
			Image* pImage = m_pImageManager->GetImage( (floor)(iter->GetFrame()) );
			pImage->DrawImageXYZ( 
				iter->GetX(),
				iter->GetY(),
				iter->GetZ(),
				iter->GetAlpha(),
				iter->GetFacingDirection(),
				iter->GetCurrentScaleFactor() );
		}
	}
}

void GameDisplay::DisplayExplosions()
{
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );
	float Radius(m_pWii->GetXmlVariable(HashString::ViewRadiusForSprites));
	Radius*=Radius;

	for (std::vector<Vessel>::iterator iter(m_pGameLogic->GetExplosionsContainerBegin());
		iter!= m_pGameLogic->GetExplosionsContainerEnd(); ++iter)
	{	
		if ( iter->InsideRadius(fCamX, fCamY, Radius ) )
		{
			Image* pImage = m_pImageManager->GetImage( (floor)(iter->GetFrame()) );
			pImage->DrawImageXYZ(iter->GetX(),iter->GetY(),iter->GetZ(),iter->GetAlpha(),iter->GetFacingDirection(),iter->GetCurrentScaleFactor());
		}
	}
}


void GameDisplay::DisplayScorePing()
{
	m_pFontManager->SetFontColour(255,255,255,255);

	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );
	float Radius(m_pWii->GetXmlVariable(HashString::ViewRadiusForSprites));
	Radius*=Radius;

	//Util3D::Trans(fCamX,fCamY);
	Util3D::CameraIdentity();

	for (std::vector<ScorePingVessel>::iterator iter(m_pGameLogic->GetScorePingContainerBegin());
		iter!= m_pGameLogic->GetScorePingContainerEnd(); ++iter)
	{	
		if ( iter->InsideRadius(fCamX, fCamY, Radius ) )
		{
			m_pFontManager->DisplayTextCentre( iter->GetText() , 
				iter->GetX(), iter->GetY(), iter->GetAlpha() , HashString::SmallFont);
		}
	}

	m_pFontManager->SetFontColour(255,255,255,255);
}

void GameDisplay::DisplayBadShips()
{
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );
	float Rad = m_pWii->GetXmlVariable(HashString::ViewRadiusForSprites);
	Rad *= Rad;

	for (std::vector<Vessel>::iterator BadIter(m_pGameLogic->GetSmallEnemiesContainerBegin()); 
		BadIter!= m_pGameLogic->GetSmallEnemiesContainerEnd(); ++BadIter )
	{

		// OPTISE this 

		// passing camera details!!!
		// getting frame again, and again!!!

		// maybe get inside radius to check on a counter, or cycle check the ships ???


		if ( BadIter->InsideRadius(fCamX, fCamY, Rad ) )
		{
			m_pImageManager->GetImage( BadIter->GetFrameStart() )->DrawImage(*BadIter);
		}
	}
}

void GameDisplay::DisplayGunShips()
{
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );
	float Rad = m_pWii->GetXmlVariable(HashString::ViewRadiusForSprites);
	Rad *= Rad;

	for (std::vector<Vessel>::iterator GunShipIter(m_pGameLogic->GetGunShipContainerBegin()); 
		GunShipIter!= m_pGameLogic->GetGunShipContainerEnd(); ++GunShipIter )
	{
		if ( GunShipIter->InsideRadius(fCamX, fCamY, Rad ) )
		{
			Util3D::TransRot(GunShipIter->GetX(),GunShipIter->GetY(),GunShipIter->GetZ(),GunShipIter->GetFacingDirection());
			Mtx FinalMatrix,TransMatrix;

			Util3D::MatrixRotateZ(TransMatrix, GunShipIter->GetFacingDirection() );
			//guMtxRotRad(TransMatrix,'Z',GunShipIter->GetFacingDirection());  // Rotage

			guMtxTransApply(TransMatrix,TransMatrix,GunShipIter->GetX(),GunShipIter->GetY(),GunShipIter->GetZ() );	// Position
			guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),TransMatrix,FinalMatrix);
			GX_LoadPosMtxImm (FinalMatrix, GX_PNMTX0); 

			float Alpha(0); //min(255, (14 * (18-GunShipIter->m_iShieldLevel)));

			if (GunShipIter->GetShieldLevel()<=20)
			{
				Alpha = 12.75f * (20-GunShipIter->GetShieldLevel());
				if (Alpha>255)
					Alpha=255;
			}
			m_pImageManager->GetImage(GunShipIter->GetFrameStart())->DrawImage(255);
			m_pImageManager->GetImage(GunShipIter->GetFrameStart()+1)->DrawImage( Alpha );

			float DirectionToFaceTarget = GunShipIter->GetTurrentDirection();

			int TurrentFrame = m_pWii->m_FrameEndStartConstainer[HashString::TurretForGunShip].StartFrame;
			if (GunShipIter->GetShieldLevel() < 6)
				TurrentFrame = m_pWii->m_FrameEndStartConstainer[HashString::BrokenTurretForGunShip].StartFrame;


			Image* pTurrentFrame( m_pImageManager->GetImage(TurrentFrame) );

			//-----------------------------------------------------------------------
			//
			// gun port 1
			//
			Util3D::MatrixRotateZ(FinalMatrix, DirectionToFaceTarget - GunShipIter->GetFacingDirection() );
			//	guMtxRotRad(FinalMatrix,'Z', DirectionToFaceTarget - GunShipIter->GetFacingDirection() );  // Rotage
			guMtxTrans(TransMatrix,	m_pWii->GetXmlVariable(HashString::TurretNo1ForGunShipOriginX),	m_pWii->GetXmlVariable(HashString::TurretForGunShipOriginY),0);
			guMtxConcat(TransMatrix,FinalMatrix,FinalMatrix);
			Util3D::MatrixRotateZ(TransMatrix, GunShipIter->GetFacingDirection() );
			//	guMtxRotRad(TransMatrix,'Z',GunShipIter->GetFacingDirection());  // Rotage
			guMtxConcat(TransMatrix,FinalMatrix,FinalMatrix);
			guMtxTransApply(FinalMatrix, FinalMatrix, GunShipIter->GetX(),	GunShipIter->GetY(),GunShipIter->GetZ());	// Position
			guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),FinalMatrix,FinalMatrix);
			GX_LoadPosMtxImm (FinalMatrix, GX_PNMTX0); 
			pTurrentFrame->DrawImage();
			//----------------------------------------------------------------------------------------
			//
			// gun port 2
			//
			Util3D::MatrixRotateZ(FinalMatrix, DirectionToFaceTarget - GunShipIter->GetFacingDirection() );
			//	guMtxRotRad(FinalMatrix,'Z',DirectionToFaceTarget - GunShipIter->GetFacingDirection());  // Rotage
			guMtxTrans(TransMatrix,	m_pWii->GetXmlVariable(HashString::TurretNo2ForGunShipOriginX),	m_pWii->GetXmlVariable(HashString::TurretForGunShipOriginY),0);
			guMtxConcat(TransMatrix,FinalMatrix,FinalMatrix);
			Util3D::MatrixRotateZ(TransMatrix, GunShipIter->GetFacingDirection() );
			//	guMtxRotRad(TransMatrix,'Z',GunShipIter->GetFacingDirection());  // Rotage
			guMtxConcat(TransMatrix,FinalMatrix,FinalMatrix);
			guMtxTransApply(FinalMatrix, FinalMatrix, GunShipIter->GetX(),	GunShipIter->GetY(),GunShipIter->GetZ());	// Position
			guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),FinalMatrix,FinalMatrix);
			GX_LoadPosMtxImm (FinalMatrix, GX_PNMTX0); 
			pTurrentFrame->DrawImage();
		}
	}
}

void GameDisplay::DisplayGunTurrets()
{

	m_pWii->GetRender3D()->RenderModelPreStage(HashString::SmallGunTurret); 
	for (std::vector<TurretItem3D>::iterator iter(m_pGameLogic->GetSmallGunTurretContainerBegin()); iter!=m_pGameLogic->GetSmallGunTurretContainerEnd(); ++iter)
	{
		Mtx Model,mat,mat2;
		////Util3D::MatrixRotateY(mat,  iter->m_Pitch );
		////Util3D::MatrixRotateZ(mat2, iter->m_Roll );

		Util3D::MatrixRotateY(mat,  iter->GetRotateY() );
		Util3D::MatrixRotateZ(mat2, iter->GetRotateZ() );

		guMtxConcat(mat,mat2,Model);
		guMtxScaleApply(Model,Model,iter->GetScaleX(),iter->GetScaleY(),iter->GetScaleZ());
		guMtxTransApply(Model, Model, iter->GetX(), iter->GetY(), iter->GetZ());

		guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,mat2);
		m_pWii->GetRender3D()->RenderModelMinimalHardNorms(HashString::SmallGunTurret, mat2);
	}
}

void GameDisplay::DisplayShotForGunTurret()
{
	m_pWii->GetRender3D()->RenderModelPreStage(HashString::Shot); 
	for (std::vector<Item3D>::iterator iter(m_pGameLogic->GetShotForGunTurretContainerBegin()); 
		iter!= m_pGameLogic->GetShotForGunTurretContainerEnd(); ++iter )
	{
		Mtx Model,FinalResult,mat,mat2;
		Util3D::MatrixRotateX(mat,  iter->GetRotateX() );
		Util3D::MatrixRotateZ(mat2, iter->GetRotateZ() );
		guMtxConcat(mat,mat2,Model);
		guMtxTransApply(Model,Model, iter->GetX(), iter->GetY(), iter->GetZ());
		guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,FinalResult); // note: by not using Model for the output we can avoid an extra internal copy	
		m_pWii->GetRender3D()->RenderModelMinimalHardNorms(HashString::Shot, FinalResult);
	}
} 



void GameDisplay::DisplaySmallSimpleMessageForThread(ThreadData* pData)
{
	// This section is for game startup

	static float fAngle = -M_PI;
	fAngle +=0.085f;


	m_pWii->GetCamera()->SetCameraView( 0, 0 );
	Util3D::Trans(0,0,250);

	Draw_Util::DrawRectangle(-320*2,-240*2,640*2,480*2,255,10,0,0,0,0,30);

	m_pFontManager->DisplayText(pData->Message, -370, 260, 255,HashString::SmallFont);

	if (pData->WorkingBytesDownloaded!=0) {
		if (pData->WorkingBytesDownloaded==-1)
			m_pFontManager->DisplayText("saving file...", -370, 260-30, 255,HashString::SmallFont);
		else
			m_pFontManager->DisplayText(Util::NumberToString(pData->WorkingBytesDownloaded), -370, 260-30, 255,HashString::SmallFont);
	}

	static vector<u16> NumberContainer;
	if ( (pData->State == ThreadData::LOADING) ||  (pData->State == ThreadData::ONLINEDOWNLOAD_EXTRAFILES) ||  (pData->State == ThreadData::ONLINEDOWNLOAD_UPDATE)) {
		if (NumberContainer.empty()) {
			for (int i=0; i<8 ; i++){
				NumberContainer.push_back(rand()%65535);
			}
		} else {
			NumberContainer.erase( NumberContainer.begin() );
			NumberContainer.push_back(rand()%65535);
		}
	}

	int i = 0 ;
	for ( vector<u16>::iterator iter(NumberContainer.begin()); iter!=NumberContainer.end(); iter++ ){
		std::stringstream stream; 
		stream << std::setw( 16 ) << std::setfill( '0' )  <<  bitset< 16 >( *iter );
		m_pFontManager->DisplayTextCentre( stream.str(), 232,180 + (i*16), 255 - ((i%2)*85) ,HashString::SmallFont);
		i++;
	}

	m_pImageManager->GetImage(0)->DrawImageXYZ(320-45,240-45-10,0,128,fAngle/3);
	m_pImageManager->GetImage(0)->DrawImageXYZ(128-50-30,128+100,0,128,-fAngle/3);
	m_pImageManager->GetImage(0)->DrawImageXYZ(0,0,0,128,fAngle/3);


	Util3D::Trans(0,0);

	
	if (pData->State == ThreadData::LOADING) {
		m_pFontManager->DisplayTextCentre("Loading, please wait...", 0,0,255 - (abs(sin(fAngle)*200)),HashString::SmallFont);
	} else if (pData->State == ThreadData::UPDATE_COMPLETE_RESET) {
		m_pFontManager->DisplayTextCentre("Upate complete, please reload...", 0,0,255,HashString::SmallFont);
	}else if (pData->State == ThreadData::ONLINEDOWNLOAD_UPDATE) {
		m_pFontManager->DisplayTextCentre("Downloading update, please wait...", 0,0,255 - (abs(sin(fAngle)*200)),HashString::SmallFont);
	}else if (pData->State == ThreadData::ONLINEDOWNLOAD_EXTRAFILES) {
		m_pFontManager->DisplayTextCentre("Downloading extra files, please wait...", 0,0,255 - (abs(sin(fAngle)*200)),HashString::SmallFont);
	}else 	if (pData->State == ThreadData::QUESTION) {
		GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
		Util3D::Trans(m_pWii->GetScreenWidth()/2.0f, m_pWii->GetScreenHeight()/2.0f);
		m_pWii->GetMessageBox()->DisplayMessageBox(500,300);
	}else 	if (pData->State == ThreadData::QUIT) {
		m_pFontManager->DisplayTextCentre("Toodeloo...", 0,0,255,HashString::SmallFont);
	}

	m_pWii->SwapScreen(GX_TRUE);  
}
//
//void GameDisplay::DisplaySimpleMessage(std::string Text, float fAngle)
//{
//	m_pWii->GetCamera()->SetCameraView( 0, 0 );
//	for (int i=0 ;i<2; ++i)
//	{	
//		Util3D::TransRot(0,0,0);
//		m_pWii->DrawRectangle(-320,-240,640,480,255,0,0,0,0,0,40);
//
//		Util3D::TransRot(0,0,fAngle);
//		m_pFontManager->DisplayTextCentre(Text, 0,0,255,HashString::LargeFont);
//
//		m_pWii->SwapScreen(); 
//	}
//}

void GameDisplay::Printf(int x, int y, const char* pFormat, ...)
{
#ifndef BUILD_FINAL_RELEASE
	static const u32 BufferSize(128);
	va_list tArgs;
	va_start(tArgs, pFormat);
	char Buffer[BufferSize+1];
	vsnprintf(Buffer, BufferSize, pFormat, tArgs);	
	va_end(tArgs);
	Util3D::Trans(m_pWii->GetCamera()->GetCamX(), m_pWii->GetCamera()->GetCamY());
	m_pWii->GetFontManager()->DisplayText(Buffer,x,y,200,HashString::SmallFont);
#endif
}


void GameDisplay::DebugInformation()
{
#ifndef BUILD_FINAL_RELEASE

	extern profiler_t profile_ProbeMineLogic;
	extern profiler_t profile_Asteroid ;
	extern profiler_t profile_MoonRocks;
	extern profiler_t profile_SmallEnemies;
	extern profiler_t profile_GunShip;
	extern profiler_t profile_Explosions;
	extern profiler_t profile_Spores;
	extern profiler_t profile_Missile;
	extern profiler_t profile_Exhaust;
	extern profiler_t profile_Projectile;
	//extern profiler_t profile_Mission;
	extern profiler_t profile_ShotAndGunTurret;
	extern profiler_t profile_DyingEnemies;

	//static u8 LastFPS(0);
	static int DroppedFrames(0);
	int y=-240;
	int x=-320;

	u8 FPS( Util::CalculateFrameRate(true) );
	if (FPS<60) ++DroppedFrames;

	Printf(x,y+=32,"%02dfps %ddropped",FPS,DroppedFrames/60);

	return;

	if (m_pGameLogic->GetAsteroidContainerSize()!=0)
		Printf(x,y+=22,"%03d %s",m_pGameLogic->GetAsteroidContainerSize(), m_pWii->profiler_output(&profile_Asteroid).c_str());
	if (m_pGameLogic->GetMoonRocksContainerSize()!=0)
		Printf(x,y+=22,"%03d %s",m_pGameLogic->GetMoonRocksContainerSize(), m_pWii->profiler_output(&profile_MoonRocks).c_str());
	
	if (m_pGameLogic->GetSmallEnemiesContainerSize()!=0)
		Printf(x,y+=22,"%03d %s",m_pGameLogic->GetSmallEnemiesContainerSize(), m_pWii->profiler_output(&profile_SmallEnemies).c_str());
	if (m_pGameLogic->GetGunShipContainerSize()!=0)
		Printf(x,y+=22,"%03d %s",m_pGameLogic->GetGunShipContainerSize(), m_pWii->profiler_output(&profile_GunShip).c_str() );
	if (m_pGameLogic->GetProbeMineContainerSize()!=0)
		Printf(x,y+=22,"%03d %s",m_pGameLogic->GetProbeMineContainerSize(), m_pWii->profiler_output(&profile_ProbeMineLogic).c_str());
	if (m_pGameLogic->GetExplosionsContainerSize()!=0)
		Printf(x,y+=22,"%03d %s",m_pGameLogic->GetExplosionsContainerSize(), m_pWii->profiler_output(&profile_Explosions).c_str() );
	if (m_pGameLogic->GetSporesContainerSize()!=0)
		Printf(x,y+=22,"%03d %s",m_pGameLogic->GetSporesContainerSize(), m_pWii->profiler_output(&profile_Spores).c_str());
	if (m_pGameLogic->GetMissileContainerSize()!=0)
		Printf(x,y+=22,"%03d %s",m_pGameLogic->GetMissileContainerSize(), m_pWii->profiler_output(&profile_Missile).c_str());
	if (m_pGameLogic->GetExhaustContainerSize()!=0)
		Printf(x,y+=22,"%03d %s",m_pGameLogic->GetExhaustContainerSize(), m_pWii->profiler_output(&profile_Exhaust).c_str());
	if (m_pGameLogic->GetProjectileContainerSize()!=0)
		Printf(x,y+=22,"%03d %s",m_pGameLogic->GetProjectileContainerSize(), m_pWii->profiler_output(&profile_Projectile).c_str());
	if (m_pGameLogic->GetShotForGunTurretContainerSize()!=0)
		Printf(x,y+=22,"%03d %s",m_pGameLogic->GetShotForGunTurretContainerSize(), m_pWii->profiler_output(&profile_ShotAndGunTurret).c_str()  );
	if (m_pGameLogic->GetDyingEnemiesContainerSize()!=0)
		Printf(x,y+=22,"%03d %s",m_pGameLogic->GetDyingEnemiesContainerSize(), m_pWii->profiler_output(&profile_DyingEnemies).c_str() );

	//	Printf(x,y+=22,"CurrentMission: %d (%s)",m_pWii->GetMissionManager()->GetCurrentMission(), m_pWii->profiler_output(&profile_Mission).c_str() );

#endif

}




//code for 3d player ship - works but looks crap from a distance
////////#if 1
////////		//our ship
////////		m_pImageManager->GetImage(GetPlrVessel()->m_fFrame)->DrawImageXYZ( 
////////			GetPlrVessel()->GetX(), GetPlrVessel()->GetY(), GetPlrVessel()->GetZ(), 
////////			255, GetPlrVessel()->GetFacingDirection(), 1.25f );
////////#else
////////		//=========================
////////		GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
////////		m_pWii->GetCamera()->SetVesselLightOn(GetPlrVessel()->GetX(), GetPlrVessel()->GetY(), GetPlrVessel()->GetZ() - 100000);
////////		//--------------------------
////////		Mtx Model,mat;
////////		guMtxIdentity(Model);
////////		guMtxRotRad(Model,'x', -M_PI/2 );
////////
////////		guMtxRotRad(mat,'y', -GetPlrVessel()->GetLastValueAddedToFacingDirection()*8 );
////////		guMtxConcat(mat,Model,Model);
////////
////////		guMtxRotRad(mat,'z', GetPlrVessel()->GetFacingDirection() );
////////		guMtxConcat(mat,Model,Model);
////////
////////		guMtxScaleApply(Model,Model,12,12,12);
////////		guMtxTrans(mat, GetPlrVessel()->GetX(), GetPlrVessel()->GetY(), GetPlrVessel()->GetZ() );
////////		guMtxConcat(mat,Model,Model);
////////		guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
////////		m_pWii->GetRender3D()->RenderModelHardNorms("Viper", Model);
////////		//--------------------------
////////		m_pWii->GetCamera()->SetLightOff();
////////		GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
////////		//=====================
////////
////////#endif