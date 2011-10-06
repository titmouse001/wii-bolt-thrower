#include <math.h>
#include <gccore.h>
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



using namespace std;

GameDisplay::GameDisplay() : m_pWii(NULL), m_pGameLogic(NULL), m_pImageManager(NULL)
{
}

void GameDisplay::Init()
{
	m_pWii = Singleton<WiiManager>::GetInstanceByPtr();
	m_pGameLogic = m_pWii->GetGameLogic();
	m_pImageManager = m_pWii->GetImageManager();
}

void GameDisplay::DisplayAllForIntro()
{
	Util::CalculateFrameRate();

	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);

	static float bbb(0);
	m_pWii->GetSpaceBackground()->DrawImageXYZ(0,0, 9400,255,bbb*0.025,25.0f );
	bbb+=0.005f;
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	m_pWii->GetCamera()->SetLightOn2();

	{
		// Viper
		Mtx Model,mat;
		guMtxIdentity(Model);
		Util3D::MatrixRotateX(Model, bbb*4.33);
		Util3D::MatrixRotateY(mat, bbb*2.33);
		guMtxConcat(mat,Model,Model);
		guMtxScaleApply(Model,Model,0.35f,0.35f,0.35f);
		guMtxTransApply(Model,Model, 0, 0, 900.0f );
		Util3D::MatrixRotateY(mat, bbb);
		guMtxConcat(mat,Model,Model);
		guMtxTransApply(Model,Model, 0, -140, 475.0f );
		guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
		m_pWii->Render.RenderModelHardNorms(HashString::Viper, Model);
	}

	// 3D section
	DisplayMoon();
	
	//DisplayGunTurrets();
	DisplayShotForGunTurret();

//	DisplayAsteroids();
//	DisplayShieldGenerators();
//	DisplayPickUps();

	m_pWii->GetCamera()->SetLightOff();
	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);

//	DisplayGunShips();
//	DisplaySporeThings();
//	DisplayRadar();

	DisplayProbMines();

	DisplayProjectile();
//	DisplayMissile();
	DisplayExhaust();
	DisplayExplosions();
	DisplayBadShips();

	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	m_pWii->GetCamera()->SetLightOn2();
	DisplayGunTurrets();
	m_pWii->GetCamera()->SetLightOff();
	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);


	m_pWii->GetCamera()->SetCameraView(0,0);
	Util3D::TransRot(-204,-128,-3.14f/4.0f);
	m_pWii->GetFontManager()->DisplayLargeTextCentre(m_pWii->GetText("attract_mode"),0,0,fabs(sin(bbb)*80));

	Util3D::Identity();
	static float wobble	(0);
	wobble+=0.05;
	m_pWii->GetFontManager()->DisplayLargeTextCentre(m_pWii->GetText("PressButtonAToContinueMessage"),0, 145 + exp((sin(wobble)*2.8f)),110);

	DebugInformation();

	GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);
	m_pWii->SwapScreen();  // to clear zbuffer keep GX_SetZMode on until after this call 
}

void GameDisplay::DisplayAllForIngame()
{
	Util::CalculateFrameRate();


	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
	m_pWii->GetSpaceBackground()->DrawImageXYZ(0,0, 9400,255,0, 28.0f );

	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	m_pWii->GetCamera()->SetLightOn2();

	// 3D section
	DisplayMoon();

	DisplayGunTurrets();
	DisplayShotForGunTurret();

	DisplayAsteroids();
	DisplayShieldGenerators();

	if ( m_pGameLogic->GetPlrVessel()->HasShieldFailed() )  
		DisplaySkull();
	
	DisplayPickUps();

	m_pWii->GetCamera()->SetLightOff();
	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);

	if ( !m_pGameLogic->GetPlrVessel()->HasShieldFailed() )  
	{
		// 2D section
		//our ship
		m_pImageManager->GetImage(m_pGameLogic->GetPlrVessel()->GetFrame())->DrawImageXYZ( 
			m_pGameLogic->GetPlrVessel()->GetX(), m_pGameLogic->GetPlrVessel()->GetY(), m_pGameLogic->GetPlrVessel()->GetZ(), 255, m_pGameLogic->GetPlrVessel()->GetFacingDirection(), 1.25f );

		//red overloading shiled
		m_pImageManager->GetImage(m_pWii->m_FrameEndStartConstainer[HashString::ShieldRed].StartFrame)->DrawImageXYZ( m_pGameLogic->GetPlrVessel()->GetX(),
			m_pGameLogic->GetPlrVessel()->GetY(), m_pGameLogic->GetPlrVessel()->GetZ(), 128 - (m_pGameLogic->GetPlrVessel()->GetShieldLevel()*2), (rand()%(314*2)) * 0.01  );
	}


	DisplayBadShips();
	DisplayGunShips();
	DisplaySporeThings();
	DisplayRadar();
	DisplayProbMines();
	DisplayProjectile();
	DisplayMissile();
	DisplayExhaust();
	DisplayExplosions();



	m_pWii->GetCamera()->StoreCameraView();
	m_pWii->GetCamera()->SetCameraView(m_pWii->GetScreenWidth()*0.5f, m_pWii->GetScreenHeight()*0.5f) ;
	DisplayInformationPanels();
	m_pWii->GetCamera()->RecallCameraView();


	// Display Aim Pointer
//	GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_FALSE);
	for (std::vector<guVector>::iterator iter(m_pGameLogic->GetAimPointerContainerBegin()); 
		iter!= m_pGameLogic->GetAimPointerContainerEnd(); ++iter)
	{
		m_pImageManager->GetImage( m_pWii->m_FrameEndStartConstainer[HashString::AimingPointer].StartFrame )
			->DrawImageXYZ( iter->x,iter->y, 0, 255, m_pGameLogic->GetPlrVessel()->GetFacingDirection() );
	}

//	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);

	m_pWii->GetMessageBox()->DoMessageBox();

	if (m_pGameLogic->IsGamePausedByPlayer())
	{
		static float ccc(0);
		ccc+=0.015;
		Util3D::TransRot(m_pGameLogic->GetPlrVessel()->GetX(),m_pGameLogic->GetPlrVessel()->GetY(),0,(sin(ccc) - cos(ccc))*0.025f);
		m_pWii->DrawRectangle( -160, -60, 320,  75, 112, 0,0,50 );
		m_pWii->GetFontManager()->DisplayLargeTextCentre(m_pWii->GetText("GAME_PAUSED"),		0,-20,160+(cos(ccc*4)*44.0f));
		m_pWii->GetFontManager()->DisplaySmallTextCentre(m_pWii->GetText("Press_PLUS_To_Play"),	0,+40, 230);
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

	m_pWii->Render.RenderModelPreStage(HashString::Satellite);  // rock1 & rock2 use the same texture
	for (std::vector<Item3DChronometry>::iterator iter(m_pGameLogic->GetShieldGeneratorContainerBegin() ); iter!= m_pGameLogic->GetShieldGeneratorContainerEnd(); /*NOP*/)
	{
		iter->Rotate(); //TODO this is a logic only part
		iter->AddVelToPos();

		Mtx Model,mat;
		Util3D::MatrixRotateZ(Model, iter->GetRotateZ());
		Util3D::MatrixRotateY(mat, iter->GetRotateY());
//		guMtxRotRad(Model,'z',iter->GetRotateZ() ); 
//		guMtxRotRad(mat,'y',iter->GetRotateY() );
		guMtxConcat(mat,Model,Model);
		guMtxScaleApply(Model,Model,iter->GetScaleX(),iter->GetScaleY(),iter->GetScaleZ());
		guMtxTrans(mat, iter->GetX(), iter->GetY(), iter->GetZ());
		guMtxConcat(mat,Model,Model);
		guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
		m_pWii->Render.RenderModelMinimalHardNorms(HashString::Satellite, Model);

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

		if (iter->GetZ() < -450 )
			iter = m_pGameLogic->EraseItemFromShieldGeneratorContainer(iter);
		else
			++iter;
	}
}
	
void GameDisplay::DisplayPickUps()
{
	m_pWii->Render.RenderModelPreStage(HashString::Material_PickUp); 

	for (std::vector<Item3D>::iterator iter(m_pGameLogic->GetMaterialPickUpContainerBegin()); 
			iter!= m_pGameLogic->GetMaterialPickUpContainerEnd(); ++iter)
	{
		Mtx Model,mat;
		
		Util3D::MatrixRotateZ(Model, iter->GetRotateZ());
		Util3D::MatrixRotateY(mat, iter->GetRotateY());
//		guMtxRotRad(Model,'z', iter->GetRotateZ() ); 
//		guMtxRotRad(mat,'y',iter->GetRotateY() );
		guMtxConcat(mat,Model,Model);
		guMtxScaleApply(Model,Model,iter->GetScaleX(),iter->GetScaleY(),iter->GetScaleZ());
		guMtxTrans(mat, iter->GetX(), iter->GetY(), iter->GetZ());
		guMtxConcat(mat,Model,Model);
		guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
		m_pWii->Render.RenderModelMinimalHardNorms(HashString::Material_PickUp, Model);
	}
}

void GameDisplay::DisplayMoon()
{
	for (std::vector<MoonItem3D>::iterator MoonIter(m_pGameLogic->GetCelestialBodyContainerBegin()); 
		MoonIter!= m_pGameLogic->GetCelestialBodyContainerEnd(); ++MoonIter )
	{
		Vec3 v( MoonIter->GetX(), MoonIter->GetY(), MoonIter->GetZ() );
		float r = m_pWii->Render.GetDispayListModelRadius(HashString::MoonShield);  // just using MoonShield for anything inside it, i.e the moon
		if (m_pWii->m_Frustum.sphereInFrustum(v,r) != FrustumR::OUTSIDE)
		{
			// moon
			Mtx Model,mat;
			Util3D::MatrixRotateY(Model, MoonIter->GetRotateY());
//			guMtxRotRad( Model,'y', MoonIter->GetRotateY() ) ;
			guMtxTrans( mat, MoonIter->GetX(), MoonIter->GetY(), MoonIter->GetZ() );  // distance back
			guMtxConcat(mat,Model,Model);
			guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
			if (MoonIter->GetDetailLevel() == Low)
				m_pWii->Render.RenderModel(HashString::MoonLowRess, Model);
			else
				m_pWii->Render.RenderModel(HashString::MoonHiRess, Model);


			if (m_pGameLogic->IsBaseShieldOnline())
			{
				// moon shield
				GX_SetCullMode(GX_CULL_NONE);
				GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
				Util3D::MatrixRotateY(Model, MoonIter->GetRotateY()*8);
//				guMtxRotRad(Model,'y', MoonIter->GetRotateY()*8) ;
				guMtxTrans(mat, 0, 0, MoonIter->GetZ());
				guMtxConcat(mat,Model,Model);
				guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
				m_pWii->Render.RenderModelHardNorms(HashString::MoonShield, Model);
				GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
				GX_SetCullMode(GX_CULL_BACK);
			}
		}

		// moon rocks
		if (m_pWii->m_Frustum.sphereInFrustum(v,m_pGameLogic->GetClippingRadiusNeededForMoonRocks()) != FrustumR::OUTSIDE)
		{
			float AmountOfRocksToDisplay( MoonIter->GetAmountOfRocks() );
			float Total( m_pGameLogic->GetMoonRocksContainerSize() );
			u32 Step(Total/AmountOfRocksToDisplay);
			u32 WorkingStep(Step);

			m_pWii->Render.RenderModelPreStage(HashString::Rock1);  // rock1 & rock2 use the same texture
			for (std::vector<Item3D>::iterator iter(m_pGameLogic->GetMoonRocksContainerBegin()); 
				iter!= m_pGameLogic->GetMoonRocksContainerEnd(); ++iter)
			{

						//IDEA- create rocks in random order then this bit is not needed!!!
		

			//can't just take the first amount we need as the rocks will clump - since they have been created in squence
			WorkingStep--;
			if (WorkingStep<=0)   // throw some away (rocks are shared across all moons - but some have less) 
				WorkingStep=Step; 
			else
				continue;



				Mtx Model,mat;
				Util3D::MatrixRotateZ(Model, iter->GetRotateZ());
				Util3D::MatrixRotateY(mat, iter->GetRotateY());

				//guMtxRotRad(Model,'z', iter->GetRotateZ() ); 
				//guMtxRotRad(mat,'y', iter->GetRotateY() );
				guMtxConcat(mat,Model, Model);
				guMtxScaleApply(Model, Model, iter->GetScaleX(),iter->GetScaleY(),iter->GetScaleZ());
				guMtxTransApply(Model, Model, iter->GetX(), iter->GetY(), iter->GetZ());

				Util3D::MatrixRotateY(mat, MoonIter->GetRotateY());
				//guMtxRotRad(mat,'y', MoonIter->GetRotateY() ); // spin around moon axis
				guMtxConcat(mat,Model,Model);
				guMtxTransApply(Model,Model, MoonIter->GetX(), MoonIter->GetY(), MoonIter->GetZ());
				guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);

				if (MoonIter->GetDetailLevel() == Low)
				{
					m_pWii->Render.RenderModelMinimal(HashString::Rock2, Model);  //lowress
				}
				else if (MoonIter->GetDetailLevel() == High)
				{
					m_pWii->Render.RenderModelMinimal(HashString::Rock1, Model); 
				}
				else if (MoonIter->GetDetailLevel() == Auto)
				{
					if (( fabs(iter->GetZ()) > 750)  ||( fabs(iter->GetX()) > 750))
						m_pWii->Render.RenderModelMinimal(HashString::Rock1, Model);  //hiress
					else
						m_pWii->Render.RenderModelMinimal(HashString::Rock2, Model);  //lowress
				}

			}
		}
	}
}



void GameDisplay::DisplayInformationPanels()
{
	// summary of baddies left
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );
	float BoxWidth = 46;
	float BoxHeight = 26;
	float x = (  -m_pWii->GetScreenWidth()*0.065f )  + ( m_pWii->GetScreenWidth() / 2) - BoxWidth;
	float y = ( -m_pWii->GetScreenHeight()*0.075f) + ( m_pWii->GetScreenHeight() / 2) - BoxHeight;

	m_pWii->TextBoxWithIcon( fCamX + x, fCamY + y, BoxWidth, BoxHeight,
		WiiManager::eRight, HashString::ThingFrames,
		"%02d",m_pGameLogic->GetSporesContainerSize(), 
		m_pWii->GetMissionManager()->GetCurrentMission() );

	m_pWii->TextBoxWithIcon( fCamX + x - BoxWidth -2, fCamY + y, BoxWidth, BoxHeight,
		WiiManager::eRight, HashString::Bad1Frames,
		"%02d", m_pGameLogic->GetTotalEnemiesContainerSize() );

	// score	
	BoxWidth = 86;
	BoxHeight = 26;
	x = ( m_pWii->GetScreenWidth()* 0.065f);
	y = ( m_pWii->GetScreenHeight()* (1.0f - 0.075f) ) - BoxHeight;
	m_pWii->TextBox( x, y, BoxWidth, BoxHeight, WiiManager::eCentre, "%08d", m_pGameLogic->GetScore() );

	// message at game startup
	if (m_pGameLogic->IsGamePausedByPopUp() && 
		(m_pWii->GetMissionManager()->GetCurrentMission()==1) )
	{
		m_pWii->GetFontManager()->DisplaySmallText(m_pWii->GetText("Your_Score"),100,0,200);  // relative coords to last trans
		m_pWii->GetFontManager()->DisplaySmallText(m_pWii->GetText("Press_PLUS_To_Pause_Game"),100,-100,200);  // relative coords to last trans
	}

	// Scrap parts
	x = ( m_pWii->GetScreenWidth()* 0.065f);
	y = ( m_pWii->GetScreenHeight()* (1.0f - 0.075f) ) - (BoxHeight*2.10f);
	m_pWii->TextBox( x, y ,BoxWidth,BoxHeight,WiiManager::eCentre, "%06d", m_pGameLogic->GetPlrVessel()->GetPickUpTotal() );

	if (m_pGameLogic->IsGamePausedByPopUp() && (m_pWii->GetMissionManager()->GetCurrentMission()==1) )
		m_pWii->GetFontManager()->DisplaySmallText(m_pWii->GetText("ScrapPartsCollected"),100,0,200);
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
	guMtxTransApply(Model,Model, 
		m_pGameLogic->GetPlrVessel()->GetX(), 
		m_pGameLogic->GetPlrVessel()->GetY(),dist);

	guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
	m_pWii->Render.RenderModelHardNorms(HashString::Skull, Model);
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

	for (std::vector<Vessel>::iterator Iter(m_pGameLogic->GetSporesContainerBegin()); Iter!= m_pGameLogic->GetSporesContainerEnd(); ++Iter )
	{
		//if ( ( Iter->InsideRadius(fCamX, fCamY, (120*120)*scale2 ) ) && !Iter->GetGoingBoom() )
		if ( Iter->InsideRadius(fCamX, fCamY, (120*120)*scale2 ) )
		{
			m_pWii->GetImageManager()->GetImage(HashString::YellowCircleWithHole)
				->DrawImageXYZ(fCamX - (pPlayerShip->GetX()*scale) - (320-64) + (Iter->GetX()*scale), 
						   fCamY - (pPlayerShip->GetY()*scale) - (240-64) + (Iter->GetY()*scale), 0,(200)-(size*200),0,size );
		}
	}

	m_pWii->GetImageManager()->GetImage(HashString::RadarCircle)->DrawImageXYZ(fCamX - (320-64), fCamY - (240-64),0,128);

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
		m_pWii->GetImageManager()->GetImage(HashString::MiniMoon)->DrawImageTL(-8,-8,128);  // 16x16 image
	}
	}

	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);// the rest of this section uses this gx setup
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);


	// with any luck the hardware might just optimise some of these out as they use Apha ZERO.
	// No point being cleaver here, still have the worst case so just make it ZERO ALPHA when outside the radar.

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
		int AlphaFlash = 230;  
		if ( m_pWii->GetFrameCounter() & 0x10)  // make these flash on/off
			AlphaFlash = 100;  

		GX_Begin( GX_POINTS, GX_VTXFMT3,m_pGameLogic->GetSporesContainerSize() );	
		for (std::vector<Vessel>::iterator Iter(m_pGameLogic->GetSporesContainerBegin()); 
			Iter!= m_pGameLogic->GetSporesContainerEnd(); ++Iter )
		{
			u8 Alpha = 0;
			if ( Iter->InsideRadius(fCamX, fCamY, (128*128)*scale2 ) )
				Alpha=AlphaFlash;   // its inside the radar scope

			GX_Position3f32(Iter->GetX()*scale, Iter->GetY()*scale, 0);		
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

// TODO ... logic mixed in here
void GameDisplay::DisplaySporeThings()
{
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );

	float Rad = m_pWii->GetXmlVariable(HashString::ViewRadiusForSprites);
	Rad*=Rad;

//	Image* pRed = m_pImageManager->GetImage(HashString::ShieldRed);
	
	static float size;
	size+=0.0085f;
	if (size>1.0f)
		size=0;


	for (std::vector<Vessel>::iterator iter(m_pGameLogic->GetSporesContainerBegin()); 
		iter!= m_pGameLogic->GetSporesContainerEnd(); ++iter)
	{
		if ( iter->InsideRadius(fCamX, fCamY, Rad) )
		{

			m_pWii->GetImageManager()->GetImage(HashString::YellowCircleWithHole)
				->DrawImageXYZ( (iter->GetX()), (iter->GetY()), 0,(200)-(size*200),0,size *10.0F );



			iter->AddFrame(iter->GetFrameSpeed());
			if (iter->GetFrame() >= iter->GetEndFrame())
				iter->SetFrame(iter->GetFrameStart());

			m_pImageManager->GetImage(iter->GetFrame())->DrawImage(*iter);
		}
	}
}


void GameDisplay::DisplayAsteroids()
{
	float Radius = 640; //(m_pWii->GetXmlVariable(HashString::ViewRadiusForAsteroids));
	Radius*=Radius;

	m_pWii->Render.RenderModelPreStage(HashString::Rock1);  // rock1 & rock2 use the same texture
	for (std::vector<Item3D>::iterator iter(m_pGameLogic->GetAsteroidContainerBegin()); 
		iter!= m_pGameLogic->GetAsteroidContainerEnd(); ++iter)
	{
		if ( !iter->GetEnable() )  
			continue; 

		Vec3 v(iter->GetX(),iter->GetY(),iter->GetZ());
		if (m_pWii->m_Frustum.sphereInFrustum(v,6) != FrustumR::OUTSIDE)  // !!!  dynamic size needed
		{
			Mtx Model,mat;

			Util3D::MatrixRotateY(Model, iter->GetRotateY());
			Util3D::MatrixRotateX(mat, iter->GetRotateX());

			//guMtxRotRad(Model,'y', iter->GetRotateX() ); 
			//guMtxRotRad(mat,'x', iter->GetRotateY() );

			guMtxConcat(mat,Model,Model);

			guMtxScaleApply(Model,Model,iter->GetScaleX(),iter->GetScaleY(),iter->GetScaleZ());

			guMtxTransApply(Model,Model, iter->GetX(), iter->GetY(), iter->GetZ());
			guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
			m_pWii->Render.RenderModelMinimal(HashString::Rock1, Model);
		}
	}
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
			m_pImageManager->GetImage(iter->GetFrame())->DrawImage(*iter); 
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
			pImage->DrawImageXYZ(iter->GetX(),iter->GetY(),iter->GetZ(),iter->GetAlpha(),iter->GetFacingDirection(),iter->GetCurrentScaleFactor());
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

void GameDisplay::DisplayBadShips()
{
	float fCamX( m_pWii->GetCamera()->GetCamX() );
	float fCamY( m_pWii->GetCamera()->GetCamY() );
	float Rad = m_pWii->GetXmlVariable(HashString::ViewRadiusForSprites);
	Rad *= Rad;

	for (std::vector<Vessel>::iterator BadIter(m_pGameLogic->GetSmallEnemiesContainerBegin()); 
		BadIter!= m_pGameLogic->GetSmallEnemiesContainerEnd(); ++BadIter )
	{
		if ( BadIter->InsideRadius(fCamX, fCamY, Rad ) )
		{
			m_pImageManager->GetImage(BadIter->GetFrameStart())->DrawImage(*BadIter);
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

			float DirectionToFaceTarget = GunShipIter->GetTurrentDirection(); // atan2( m_CPUTarget.x - GunShipIter->GetX(),GunShipIter->GetY() - m_CPUTarget.y  );

			int TurrentFrame = m_pWii->m_FrameEndStartConstainer[HashString::TurretForGunShip].StartFrame;
			if (GunShipIter->GetShieldLevel() < 6)
				TurrentFrame = m_pWii->m_FrameEndStartConstainer[HashString::BrokenTurretForGunShip].StartFrame;


			Image* pTurrentFrame( m_pImageManager->GetImage(TurrentFrame) );

			//---

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

			//---

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



	////for (std::vector<TurretItem3D>::iterator iter(m_pGameLogic->GetSmallGunTurretContainerBegin()); iter!=m_pGameLogic->GetSmallGunTurretContainerEnd(); ++iter)
	////{
	//////	std::vector<Vessel>::iterator TargetIter = m_pGameLogic->GetSmallEnemiesContainerBegin();
	//////	advance( TargetIter, iter->GetLockOntoVesselIndex() );

	//////	Util3D::Identity();
	//////	m_pWii->GetImageManager()->GetImage( m_pWii->m_FrameEndStartConstainer[HashString::AimingPointer].StartFrame )
	//////	->DrawImage(*TargetIter);

	////	Util3D::Identity();
	////	m_pWii->GetImageManager()->GetImage( m_pWii->m_FrameEndStartConstainer[HashString::AimingPointer].StartFrame )
	////	->DrawImageXYZ( iter->WorkingTarget.x,iter->WorkingTarget.y, iter->WorkingTarget.z, 255, 0 );
	////}

}

void GameDisplay::DisplayGunTurrets()
{

	m_pWii->Render.RenderModelPreStage(HashString::SmallGunTurret); 
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

		guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
		m_pWii->Render.RenderModelMinimalHardNorms(HashString::SmallGunTurret, Model);
	}
}

void GameDisplay::DisplayShotForGunTurret()
{
	m_pWii->Render.RenderModelPreStage(HashString::Shot); 
	for (std::vector<Item3D>::iterator iter(m_pGameLogic->GetShotForGunTurretContainerBegin()); 
		iter!= m_pGameLogic->GetShotForGunTurretContainerEnd(); ++iter )
	{
		Mtx Model;
		guMtxTrans(Model, iter->GetX(), iter->GetY(), iter->GetZ());
		guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
		m_pWii->Render.RenderModelMinimalHardNorms(HashString::Shot, Model);
	}
} 

void GameDisplay::DisplaySimpleMessage(std::string Text)
{
	m_pWii->GetCamera()->SetCameraView( 0, 0, -(579.4f*0.75f));
	Util3D::TransRot(0,0,-3.14f/12.0f);
	for (int i=0 ;i<2; ++i)
	{	
		m_pWii->DrawRectangle(-320,-240,640,480,255,0,0,80,40,0,0);
		m_pWii->GetFontManager()->DisplayLargeTextCentre(Text, 0,0,255);
		GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);
		m_pWii->SwapScreen();  // to clear zbuffer keep GX_SetZMode on until after this call 
	}
}

void GameDisplay::DebugInformation()
{
	 

#ifndef LAUNCH_VIA_WII

	static int DroppedFrames(0);
	int y=-180;
	int x=0;

	u8 FPS( Util::CalculateFrameRate(true) );
	if (FPS<60) ++DroppedFrames;

	m_pWii->Printf(x,y+=22,"DroppedFrames: %d",DroppedFrames);
	m_pWii->Printf(x,y+=22,"FPS: %d",FPS);

	m_pWii->Printf(x,y+=22,"Asteroids: %d",m_pGameLogic->GetAsteroidContainerSize());
	m_pWii->Printf(x,y+=22,"Moon Rocks: %d",m_pGameLogic->GetMoonRocksContainerSize());
	m_pWii->Printf(x,y+=22,"Small Ships: %d",m_pGameLogic->GetSmallEnemiesContainerSize());
	m_pWii->Printf(x,y+=22,"Gun Ships: %d",m_pGameLogic->GetGunShipContainerSize());
	m_pWii->Printf(x,y+=22,"Probe Mines: %d",m_pGameLogic->GetProbeMineContainerSize());
	m_pWii->Printf(x,y+=22,"Explosions: %d",m_pGameLogic->GetExplosionsContainerSize());
	m_pWii->Printf(x,y+=22,"Spores: %d",m_pGameLogic->GetSporesContainerSize());
	m_pWii->Printf(x,y+=22,"Missiles: %d",m_pGameLogic->GetMissileContainerSize());
	m_pWii->Printf(x,y+=22,"Exhaust Trails: %d",m_pGameLogic->GetExhaustContainerSize());
	m_pWii->Printf(x,y+=22,"Projectiles: %d",m_pGameLogic->GetProjectileContainerSize());
	m_pWii->Printf(x,y+=22,"CurrentMission: %d",m_pWii->GetMissionManager()->GetCurrentMission() );
	m_pWii->Printf(x,y+=22,"Turret shots: %d",m_pGameLogic->GetShotForGunTurretContainerSize() );
	
#endif

	// for checking the view is correct - should fit snug inside the view port
	//	Util3D::Identity();
	//	m_pWii->DrawRectangle(-320, -240,640, 480, 100);

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
////////		m_pWii->Render.RenderModelHardNorms("Viper", Model);
////////		//--------------------------
////////		m_pWii->GetCamera()->SetLightOff();
////////		GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
////////		//=====================
////////
////////#endif