#include "GameLogic.h"
#include <gccore.h>
#include <math.h>
#include "Singleton.h"
#include "WiiManager.h"
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
#include "ogc\lwp_watchdog.h"
#include "MenuScreens.h"
#include "Timer.h"

MenuScreens::MenuScreens() :  m_ZoomAmountForSpaceBackground(3.1f), m_pTimer(NULL)
{
}

void MenuScreens::Init()
{
	m_pWii = Singleton<WiiManager>::GetInstanceByPtr();
	m_pTimer = new Timer;
}

void MenuScreens::SetTimeOutInSeconds(int Value)
{
	m_pTimer->SetTimerSeconds(Value);
}

bool MenuScreens::HasMenuTimedOut() 
{ 
	return m_pTimer->IsTimerDone(); 
}

void MenuScreens::DoMenuScreen()
{
	WPAD_ScanPads();
	m_pWii->GetInputDeviceManager()->Store();
	Vtx* WiiMote( m_pWii->GetInputDeviceManager()->GetIRPosition() );

	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
	m_ZoomAmountForSpaceBackground+=0.001;
	m_pWii->GetSpaceBackground()->DrawImageXYZ(0,0, 9400,255,0,12.0f + (8.0f+(sin(m_ZoomAmountForSpaceBackground)*6.0f)) );

	static float spin=0.0f;
	spin+=0.01;
	m_pWii->GetSpaceBackground()->DrawImageXYZ(0,0, 9400,50,sin(spin)*0.15,13.0f + (8.0f+(sin(m_ZoomAmountForSpaceBackground)*6.0f)) );

	//=========================
	static float bbb = 0;
	bbb+=0.005f;
	//-------------------
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	m_pWii->GetCamera()->SetLightOn3();
	//--------------------------
	Mtx Model,mat;
	orient_t Orientation;
	WPAD_Orientation(0, &Orientation);
	Mtx mat2;
	Mtx mat3;

	static float yaw  = 0; 
	static float pitch =0; 
	static float roll  = 0;

	yaw += (DegToRad(Orientation.yaw) - yaw )*0.15f;
	pitch += (DegToRad(Orientation.pitch) - pitch)*0.15f;
	roll += (DegToRad(Orientation.roll) - roll)*0.15f;

	guMtxRotRad(Model,'y', -M_PI/2 + yaw) ;
	guMtxRotRad(mat2,'x', M_PI + M_PI/6 - pitch );
	guMtxRotRad(mat3,'z', M_PI - roll) ;

	guMtxConcat(mat3,Model,Model);
	guMtxConcat(mat2,Model,Model);

	guMtxTrans(mat, 0,0, 0);
	guMtxConcat(mat,Model,Model);
	guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);

	m_pWii->Render.RenderModelHardNorms(HashString::WiiMote, Model);
	//--------------------------

	m_pWii->GetCamera()->SetLightOff();
	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
	Util3D::Identity();
	m_pWii->DrawRectangle( -320, -200-4,  640,  4,	 112, 0,0,0 );
	m_pWii->DrawRectangle( -320, -200,	  640, 42,	  88, 0,0,0 );
	m_pWii->DrawRectangle( -320, -200+42, 640,  4,	 112, 0,0,0 );

	// "B O L T    T H R O W E R"
	m_pWii->GetFontManager()->DisplayLargeTextCentre( m_pWii->GetText("MainMenuScreenTopTitle") ,0,-180,190); 

	//----------------------------------------------------------
	Util3D::TransRot(320-50,240-50,-3.14f/4.0f);
	char Text[8];
	sprintf(Text,"%0d",m_pTimer->GetTimerSeconds());
	m_pWii->GetFontManager()->DisplayLargeTextCentre(Text,0,0,80);
	//----------------------------------------------------------

	m_pWii->GetMenuManager()->SetMenuGroup("MainMenu");
	m_pWii->GetMenuManager()->Draw();
	m_pWii->GetMenuManager()->MenuLogic();

	
	if (WiiMote!=NULL)
	{
		float PointerX = m_pWii->GetCamera()->GetCamX() + WiiMote->x - (m_pWii->GetScreenWidth()/2);
		float PointerY = m_pWii->GetCamera()->GetCamY() + WiiMote->y - (m_pWii->GetScreenHeight()/2);

		Util3D::TransRot(PointerX,PointerY,0, roll );
		m_pWii->DrawRectangle( -640, -3,  640*2,  6,	 32, 0,0,0 );
		m_pWii->DrawRectangle( -3, -480,  6,  512*2,	 32, 0,0,0 );

		for (int ix=-14 ; ix<14; ++ix)
		{
			if (ix&1)
			{
				m_pWii->DrawRectangle(  50*ix, -10,  2, 20, 32, 0,0,0 );
				m_pWii->DrawRectangle( -10, 50*ix, 20, 2, 32, 0,0,0 );
			}
			else
			{
				m_pWii->DrawRectangle(  50*ix, -15, 2, 30, 32, 0,0,0 );
				m_pWii->DrawRectangle( -15, 50*ix, 30, 2, 32, 0,0,0 );
			}		

		}
		m_pWii->GetImageManager()->GetImage( m_pWii->m_FrameEndStartConstainer[HashString::AimingPointer].StartFrame )
			->DrawImageXYZ( m_pWii->GetCamera()->GetCamX() + WiiMote->x - (m_pWii->GetScreenWidth()/2), 
			m_pWii->GetCamera()->GetCamY() + WiiMote->y - (m_pWii->GetScreenHeight()/2),
			WiiMote->z, 255, 0) ; //GetPlrVessel()->GetFacingDirection() );
	}
	GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);
	m_pWii->SwapScreen();  // to clear zbuffer keep GX_SetZMode on until after this call 
}

void MenuScreens::DoControlsScreen()
{
	WPAD_ScanPads();
	m_pWii->GetInputDeviceManager()->Store();

	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
	m_ZoomAmountForSpaceBackground+=0.001;
	m_pWii->GetSpaceBackground()->DrawImageXYZ(0,0, 9400,255,0,10.0f + (8.0f+(sin(m_ZoomAmountForSpaceBackground)*6.0f)) );

	static float spin=0.0f;
	spin+=0.01;
	m_pWii->GetSpaceBackground()->DrawImageXYZ(0,0, 9400,50,sin(spin)*0.15,11.0f + (8.0f+(sin(m_ZoomAmountForSpaceBackground)*6.0f)) );

	//=========================

	static float bbb = 0;
	bbb+=0.025f;
		//-------------------
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	m_pWii->GetCamera()->SetLightOn3();
	//--------------------------
	Mtx Model,mat;
	//--------------------------
	guMtxRotRad(Model,'x', bbb);
	guMtxRotRad(mat,'y',M_PI/8 + sin(bbb) * 0.25f) ;
	guMtxConcat(mat,Model,Model);
	guMtxRotRad(mat,'z',  M_PI/2 + (cos(bbb) * 0.25f)) ;
	guMtxConcat(mat,Model,Model);
	guMtxTrans(mat, -100, 0, 0);
	guMtxConcat(mat,Model,Model);
	guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);

	m_pWii->Render.RenderModelHardNorms(HashString::WiiMote, Model);

	//--------------------------

	m_pWii->GetCamera()->SetLightOff();
	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
	//=====================
	Util3D::Identity();
	m_pWii->DrawRectangle( -320, -200-4,  640,  4,	 112, 0,0,0 );
	m_pWii->DrawRectangle( -320, -200,	  640, 42,	  88, 0,0,0 );
	m_pWii->DrawRectangle( -320, -200+42, 640,  4,	 112, 0,0,0 );

	// "C O N T R O L S"
	m_pWii->GetFontManager()->DisplayLargeTextCentre( m_pWii->GetText("ControlsMenuScreenTopTitle"),0,-180,190);


	ImageManager* pImageManager = m_pWii->GetImageManager();

	Util3D::Identity();
	m_pWii->DrawRectangle(  0, -120,  42, 272,	 128, 255,255,255 );
	m_pWii->DrawRectangle( 42, -120, 250, 272,	 64, 0,0,0 );
	
	int y= -150;
	int x= 21;
	int step = 55;

	pImageManager->GetImage(HashString::WiiMoteButtonA)->DrawImage(x,y+=step);
	m_pWii->GetFontManager()->DisplaySmallTextVertCentre(m_pWii->GetText("WiiMoteButtonA"),32,0,200); //"Fire Missile"

	pImageManager->GetImage(HashString::WiiMoteButtonB)->DrawImage(x,y+=step);
	m_pWii->GetFontManager()->DisplaySmallTextVertCentre(m_pWii->GetText("WiiMoteButtonB"),32,0,200);  // "Thrusters"

	pImageManager->GetImage(HashString::WiiMoteDirectionDownMarkedRed)->DrawImage(x,y+=step);
	m_pWii->GetFontManager()->DisplaySmallTextVertCentre(m_pWii->GetText("WiiMoteDirectionDown"),32,0,200); // "Drop Mine"

	pImageManager->GetImage(HashString::WiiMoteInfraRedPointer)->DrawImage(x,y+=step);
	m_pWii->GetFontManager()->DisplaySmallTextVertCentre(m_pWii->GetText("WiiMoteInfraRedPointer"),32,0,200); //"Aim"

	pImageManager->GetImage(HashString::WiiMoteButtonHome)->DrawImage(x,y+=step);
	m_pWii->GetFontManager()->DisplaySmallTextVertCentre(m_pWii->GetText("WiiMoteButtonHome"),32,0,200);  //"Quit"

	Util3D::Identity();
	{
		static float wobble	(0);
		wobble+=0.015;
		// "PRESS A TO CONTINUE" 
		m_pWii->GetFontManager()->DisplayLargeTextCentre(m_pWii->GetText("PressButtonAToContinueMessage"),0,200.0f,50 + fabs(cos(wobble)*60.0f));
	}
	
	GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);
	m_pWii->SwapScreen();  // to clear zbuffer keep GX_SetZMode on until after this call 
}


void MenuScreens::DoCreditsScreen()
{
	WPAD_ScanPads();
	m_pWii->GetInputDeviceManager()->Store();
	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);

	m_ZoomAmountForSpaceBackground+=0.001;
	m_pWii->GetSpaceBackground()->DrawImageXYZ(0,0, 9400,255,0,10.0f + (8.0f+(sin(m_ZoomAmountForSpaceBackground)*6.0f)) );

	static float spin=0.0f;
	spin+=0.01;
	m_pWii->GetSpaceBackground()->DrawImageXYZ(0,0, 9400,50,sin(spin)*0.15,11.0f + (8.0f+(sin(m_ZoomAmountForSpaceBackground)*6.0f)) );

	//=========================
	static float bbb = 0;
	bbb+=0.025f;
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);

	m_pWii->GetCamera()->SetLightOn3();

	Mtx Model,mat;
	guMtxRotRad(Model,'x',M_PI );
	guMtxRotRad(mat,'z', sin(bbb)*0.55);
	guMtxConcat(mat,Model,Model);
	guMtxRotRad(mat,'y', M_PI + bbb*0.3f);
	guMtxConcat(mat,Model,Model);
	guMtxTrans(mat, 0,0, 0);
	guMtxConcat(mat,Model,Model);
	guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
	m_pWii->Render.RenderModelHardNorms(HashString::Viper, Model);
	m_pWii->GetCamera()->SetLightOff();
	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
	//=====================

	Util3D::Identity();
	m_pWii->DrawRectangle( -320, -200-4,  640,  4,	 112, 0,0,0 );
	m_pWii->DrawRectangle( -320, -200,	  640, 42,	  88, 0,0,0 );
	m_pWii->DrawRectangle( -320, -200+42, 640,  4,	 112, 0,0,0 );

	//"C R E D I T S"
	m_pWii->GetFontManager()->DisplayLargeTextCentre(m_pWii->GetText("CreditsMenuScreenTopTitle"),0,-180,190);

	Util3D::TransRot(0,-160,0,0);
	int y(0);
	for (int i=0; i<99; ++i)  // could use a infinite loop, 99 just to play it safe!
	{
		// 'GetText' - all text comes from the GameConfiguration.xml file, i.e get text labeled 'Credits01'
		string Message(m_pWii->GetText( "Credits" + Util::NumberToString(i)) );
		if (Message != "TAG-END")
			m_pWii->GetFontManager()->DisplaySmallTextCentre( Message,0,y+=19,200);
		else
			break;
	} 

	////m_pWii->GetFontManager()->DisplaySmallTextCentre("-=Music=- 'Space Debris' from the Amiga by Captain/Image",0,y+=19,200);
	////m_pWii->GetFontManager()->DisplaySmallTextCentre("Visit http://modarchive.org/ for a distinctive collection of modules",0,y+=19,180);
	////y+=20;
	////m_pWii->GetFontManager()->DisplaySmallTextCentre("-=3D Models=  WiiMote by Patrick Grubb",0,y+=19,200);
	////m_pWii->GetFontManager()->DisplaySmallTextCentre("Viper(MK2) by Karl Stocker (UV textures by Titmouse)",0,y+=19,200);
	////y+=20;
	////m_pWii->GetFontManager()->DisplaySmallTextCentre("-=2D Art=- Most by Lee Marks / few by Danc - www.lostgarden.com",0,y+=19,200);
	////y+=20;
	////m_pWii->GetFontManager()->DisplaySmallTextCentre("-=Programmer=- Paul Overy, alias TitMouse",0,y+=19,200);
	////y+=20;
	////m_pWii->GetFontManager()->DisplaySmallTextCentre("-= Testers=- Tom, Harry, Mr C",0,y+=19,200);
	////y+=20;
	////m_pWii->GetFontManager()->DisplaySmallTextCentre("Release v0.60 | August 2011",0,y+=19,200);
	////m_pWii->GetFontManager()->DisplaySmallTextCentre("Built using libs: -lfat -lpng -lz -lmodplay -lwiiuse -lbte -lasnd -logc -lm",0,y+=19,160);

	{
		static float wobble	(0);
		wobble+=0.05;
		m_pWii->GetFontManager()->DisplayLargeTextCentre(m_pWii->GetText("PressButtonAToContinueMessage"),0,exp(sin(wobble)*2.8f)+330.0f,128);
	
	}

	
	GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);
	m_pWii->SwapScreen();  // t	o clear zbuffer keep GX_SetZMode on until after this call 
}

void MenuScreens::DoOptionsScreen()
{
	WPAD_ScanPads();
	m_pWii->GetInputDeviceManager()->Store();

	Vtx* WiiMote( m_pWii->GetInputDeviceManager()->GetIRPosition() );

	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);
	m_ZoomAmountForSpaceBackground+=0.001;
	m_pWii->GetSpaceBackground()->DrawImageXYZ(0,0, 9400,255,0,10.0f + (8.0f+(sin(m_ZoomAmountForSpaceBackground)*6.0f)) );
	static float spin=0.0f;
	spin+=0.01;
	m_pWii->GetSpaceBackground()->DrawImageXYZ(0,0, 9400,50,sin(spin)*0.15,11.0f + (8.0f+(sin(m_ZoomAmountForSpaceBackground)*6.0f)) );
	
	//=========================
	static float bbb = 0;
	bbb+=0.025f;
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);

	m_pWii->GetCamera()->SetLightOn3();

	Mtx Model,mat;
	guMtxRotRad(Model,'x',cos(bbb*0.5));
	guMtxRotRad(mat,'z', sin(bbb)*0.55);
	guMtxConcat(mat,Model,Model);
	guMtxRotRad(mat,'y', M_PI + bbb*0.3f);
	guMtxConcat(mat,Model,Model);
	guMtxConcat(m_pWii->GetCamera()->GetcameraMatrix(),Model,Model);
	//Wii.Render.RenderModelHardNorms("SaturnV", Model);
	m_pWii->Render.RenderModelHardNorms( HashString::Satellite,Model);

	m_pWii->GetCamera()->SetLightOff();
	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_FALSE);

	//=========================
	Util3D::Identity();
	m_pWii->DrawRectangle( -320, -200-4,  640,  4,	 112, 0,0,0 );
	m_pWii->DrawRectangle( -320, -200,	  640, 42,	  88, 0,0,0 );
	m_pWii->DrawRectangle( -320, -200+42, 640,  4,	 112, 0,0,0 );
	// "O P T I O N S"
	m_pWii->GetFontManager()->DisplayLargeTextCentre(m_pWii->GetText("OptionsMenuScreenTopTitle"),0,-180,190);
		//=========================

	m_pWii->GetMenuManager()->SetMenuGroup("OptionsMenu");
	
	m_pWii->GetMenuManager()->MenuLogic();
	
	m_pWii->GetMenuManager()->Draw();


	// Draw aim pointer
	m_pWii->GetImageManager()->GetImage( m_pWii->m_FrameEndStartConstainer[HashString::AimingPointer].StartFrame )
			->DrawImageXYZ( m_pWii->GetCamera()->GetCamX() + WiiMote->x - (m_pWii->GetScreenWidth()/2), 
			m_pWii->GetCamera()->GetCamY() + WiiMote->y - (m_pWii->GetScreenHeight()/2),
			WiiMote->z, 255, 0) ; //GetPlrVessel()->GetFacingDirection() );

	GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);
	m_pWii->SwapScreen();  // t	o clear zbuffer keep GX_SetZMode on until after this call 
}






	////////////// ... check the view is correct - should fit snug inside the view port
	////////////Util3D::Identity();
	////////////Wii.DrawRectangle(-310, -230,640-20, 480-20, 100,255,255,255);
