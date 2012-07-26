#include "IntroDisplay.h"
#include "GameDisplay.h"

#include <math.h>
#include <gccore.h>
#include <sstream>
#include <iomanip> //std::setw
#include <bitset> 

#include "Singleton.h"
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

using namespace std;

void IntroDisplay::Init()
{
	m_fIncrementor=0.0f;
	m_fMessagewobble=0.0f;

	GameDisplay::Init();
}

void IntroDisplay::DisplayAllForIntro()
{	
	Util::CalculateFrameRate();

	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
	m_pWii->GetSpaceBackground()->DrawImageXYZ(0,0, 14000,255,0,26.0f );

	//
	// 3d models
	//
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	m_pWii->GetCamera()->SetLightOn2();
	DisplayViper();
	DisplayMoon(HashString::MoonHiRes);
	DisplayShotForGunTurret();
	m_pWii->GetCamera()->SetLightOff();


	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);

	
	//
	// 2d sprites
	//
	DisplayProbMines();
	
	//DisplayProjectile();

	DisplayExhaust();
	DisplayExplosions();
	DisplayBadShips();

	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);


	//
	// more 3d models
	//
	m_pWii->GetCamera()->SetLightOn2();
	DisplayGunTurrets();
	m_pWii->GetCamera()->SetLightOff();

	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);

	//
	// On screen text
	//
	m_pWii->GetCamera()->SetCameraView(0,0);
	Util3D::TransRot(-204,-128,-3.14f/4.0f);
	m_pFontManager->DisplayTextCentre(m_pWii->GetText("attract_mode"),0,0,fabs(sin(m_fIncrementor)*80),HashString::LargeFont);

	Util3D::CameraIdentity();


	m_pFontManager->DisplayTextCentre(m_pWii->GetText("PressButtonAToContinueMessage"),0, 145 + exp((sin(m_fMessagewobble)*2.8f)),110,HashString::LargeFont);

	//
	// Debug
	//
	DebugInformation();

	m_fIncrementor += 0.005f; 
	m_fMessagewobble += 0.05f;

	m_pWii->SwapScreen();
}

void IntroDisplay::DisplayViper() 
{


	Mtx Model,mat,m2;
	guMtxIdentity(Model);
	Util3D::MatrixRotateX(Model, m_fIncrementor*4.33);
	Util3D::MatrixRotateY(mat, m_fIncrementor*2.33);
	guMtxConcat(mat,Model,m2);
	guMtxScaleApply(m2,Model,0.35f,0.35f,0.35f);
	guMtxTransApply(Model,m2, 0, 0, 900.0f );
	Util3D::MatrixRotateY(mat, m_fIncrementor);
	guMtxConcat(mat,m2,Model);
	guMtxTransApply(Model,m2, 0, -140, 475.0f );
	guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),m2,Model);
	m_pWii->GetRender3D()->RenderModelHardNorms(HashString::Viper, Model);
}