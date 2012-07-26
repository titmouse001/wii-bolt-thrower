//
// WiiManager - Singleton class
//
#include <stdarg.h>   //  for things like va_list
#include <vector>
#include <set>
#include <math.h>
#include <limits.h>
#include <iomanip> //std::setw
#include <wiiuse/wpad.h>
#include <string>
#include <sstream>
#include <grrmod.h>

#include "WiiManager.h"
#include "ImageManager.h"
#include "MenuManager.h"
#include "InputDeviceManager.h"
#include "SoundManager.h"
#include "FontManager.h"
#include "URIManager.h"
//#include "SetupGame.h"
#include "UpdateManager.h"
#include "TinyXML/TinyXML.h"
#include "Image.h"
#include "font.h"
#include "Util3D.h"
#include "Util.h"
#include "HashString.h"
#include "Menu.h"
#include "MenuScreens.h"
#include "GameLogic.h"
#include "Mission.h"
#include "MessageBox.h"
#include "GameDisplay.h"
#include "introDisplay.h"
#include "config.h"
#include "debug.h"
#include "oggplayer/oggplayer.h"
#include "camera.h"
#include "CullFrustum/FrustumR.h"
#include "Render3D.h"
#include "Panels3d.h"  // has manager
#include "Draw_Util.h"
#include "Vessel.h"

#include "Thread.h"
extern Thread MyThread;

#define COLOUR_FOR_WIPE		( COLOR_BLACK )  // crash at startup colour


WiiManager::WiiManager() :	m_MusicStillLeftToDownLoad(false),
							m_pMusicData(NULL),
							m_pGXRMode(NULL), 
							m_gp_fifo(NULL), 
							m_uScreenBufferId(eFrontScreenBuffer), 
							m_uFrameCounter(0),
							m_ImageManager(NULL), 
							m_SoundManager(NULL),
							m_Render3D(NULL),
							m_Frustum(NULL),
							m_Camera(NULL),		
							m_URLManager(NULL),
							m_UpdateManager(NULL),
							m_IngameMusicVolume(3),
							//m_SetUpGame(NULL),
							m_PanelManager(NULL),
							//m_ViewportX(0),
							//m_ViewportY(0),
							m_GameState(eIntro),
							m_Language("English"),
							m_bMusicEnabled(true),
							m_Difficulty("medium")
{ 
	// -----------------------------
	m_pFrameBuffer[0] = NULL;
	m_pFrameBuffer[1] = NULL;
	// -----------------------------
	
	// this lot is taking about 4 to 5 seconds to complete!!! See id I can know that down some, don't like blank see at startup

	m_ImageManager			= new ImageManager;			// low time overheads  (inside constructor)
	m_FontManager			= new FontManager;			// low
	m_InputDeviceManager	= new InputDeviceManager;	// high  ... ok, now done at init
	m_SoundManager			= new SoundManager;			// low
	m_Render3D				= new Render3D;				// low
	m_Frustum 				= new FrustumR;				// low
	m_Camera				= new Camera;				// low
	m_pMenuManager			= new MenuManager;			// low
	m_pGameLogic			= new GameLogic;			// high ... ok now
	m_pGameDisplay			= new GameDisplay;			// low

	m_pIntroDisplay			= new IntroDisplay;

	m_pMenuScreens			= new MenuScreens;			// low
	m_MissionManager		= new MissionManager;		// low
	m_MessageBox			= new MessageBox;			// low
	m_URLManager			= new URLManager;			// high calls init!  ... ok nw
	m_UpdateManager			= new UpdateManager;		// low
//	m_SetUpGame				= new SetUpGame;			// low
	m_PanelManager			= new PanelManager;			// low

	// -----------------------------
	m_pMusicData			= new FileMemInfo;
	m_pMusicData->pData = NULL;
	m_pMusicData->Size = 0;
	// -----------------------------
}

WiiManager::~WiiManager()
{
	// note: manager's do their own housekeeping
	// -----------------------------
	delete m_ImageManager;		
	delete m_FontManager;	
	delete m_InputDeviceManager;
	delete m_Render3D;
	delete m_Frustum;
	delete m_Camera;
	delete m_pMenuManager;	
	delete m_pGameLogic;
	delete m_pMenuScreens;		
	delete m_MissionManager;
	delete m_MessageBox;
	delete m_URLManager;
	delete m_UpdateManager;
	delete m_PanelManager;
	// -----------------------------
	delete m_pMusicData;
	// -----------------------------
}

void WiiManager::UnInitWii()
{
	// check we are using a valid GX buffer
	if (m_gp_fifo!=NULL)
	{
		GX_Flush();
		GX_AbortFrame();

		free(m_gp_fifo);
		m_gp_fifo = NULL;
	}

	// Each screen Buffer used 'MEM_K0_TO_K1' to allocate - now use the reverse when freeing
	if (m_pFrameBuffer[0] != NULL)
	{
		free(MEM_K1_TO_K0(m_pFrameBuffer[0]));
		m_pFrameBuffer[0] = NULL;
	}

	if (m_pFrameBuffer[1] != NULL)
	{
		free(MEM_K1_TO_K0(m_pFrameBuffer[1]));
		m_pFrameBuffer[1] = NULL;
	}

	GRRMOD_End();

	m_SoundManager->UnInit();  //call this last

}


void WiiManager::PreInitManagers()
{
	m_ImageManager->Init();
	m_pGameDisplay->Init();
	m_pMenuScreens->Init();
	m_MessageBox->Init();
	//m_SetUpGame->Init();
	m_UpdateManager->Init();
	m_SoundManager->Init();
	m_PanelManager->Init();
	m_Camera->Init();  // dependancy on GXRModeObj

	m_pIntroDisplay->Init();

	m_pMenuManager->Init();

}

void WiiManager::FinalInitManagers()
{	
	
	MyThread.m_Data.Message = "Initialising GameLogic...";
	m_pGameLogic->Init();			// maybe slow
	
	MyThread.m_Data.Message = "Initialising InputDeviceManager...";
	m_InputDeviceManager->Init();	// maybe slow
	
	MyThread.m_Data.Message = "Initialising URLManager...";
	m_URLManager->Init();			// is slow
}

void WiiManager::InitWii()
{
	//
	// WiiMote
	//
	Util::SetUpPowerButtonTrigger();

	//
	// File System
	//
	WiiFile::InitFileSystem();

	//
	//Screen specific
	//
	InitialiseVideo();  // internally calls "SetGXRMode()"
	InitGX();
	SetUp3DProjection();
	
	//
	// Managers - minimal setup for now, slow stuff down later on to avoid long wait (black screen) at start up
	//
	PreInitManagers();

	// XML configuration - this places sections of data into specificly named containers found in the code
	CreateSettingsFromXmlConfiguration(WiiFile::GetGamePath() + "GameConfiguration.xml");

	// The XML settings now loaded, so start using them...
	if (! m_VariablesContainer.empty()) {
		int Timeout( GetXmlVariable( HashString::WiiMoteIdleTimeoutInSeconds) );
		if ( Timeout > 0 ) {
			WPAD_SetIdleTimeout(Timeout); 
		}
	}
}

GXRModeObj* WiiManager::GetBestVideoMode()
{
	GXRModeObj* vmode = VIDEO_GetPreferredMode(NULL); // get default video mode
	if (CONF_GetAspectRatio() == CONF_ASPECT_16_9) {
		vmode->viWidth = 678;
		vmode->viXOrigin = (VI_MAX_WIDTH_NTSC - 678)/2;
	}
	return vmode;
}

// this function may be called more than once
void WiiManager::InitGX(u32 GXFifoBufferSize)
{
	// allocate & clear the GX queue, 
	if (m_gp_fifo==NULL)
	{
		m_gp_fifo = (u32*)memalign(32, GXFifoBufferSize);
		memset(m_gp_fifo, 0, GXFifoBufferSize);
		GX_Init(m_gp_fifo, GXFifoBufferSize);
		//printf ("using %d for GX Fifo",GXFifoBufferSize);
	}

	VIDEO_Flush(); // Apply hardware changes

	static const GXColor WIPE_COLOUR = (GXColor){0, 0, 0, 0xff};
	GX_SetCopyClear(WIPE_COLOUR, GX_MAX_Z24);   // set EFB to clear frames with this colour


	GX_SetDispCopyGamma(GX_GM_1_0);  // Darkest setting - looks ok to me

	VIDEO_Flush(); // Apply hardware changes

	GXRModeObj* pMode( GetGXRMode() );
	GX_SetViewport(0,0,pMode->fbWidth,pMode->efbHeight,0,1);  
	GX_SetScissor(0,0,pMode->fbWidth,pMode->efbHeight);

	VIDEO_Flush(); // Apply hardware changes

	GX_SetDispCopySrc(0,0,pMode->fbWidth,pMode->efbHeight);

	//	The stride of the XFB is set using GX_SetDispCopyDst()
	f32 YScaleFactor (GX_GetYScaleFactor(pMode->efbHeight,(float)pMode->xfbHeight));
	u32 xfbHeight (GX_SetDispCopyYScale(YScaleFactor));  // GX_SetDispCopySrc must be called first

	GX_SetDispCopyDst(pMode->fbWidth,xfbHeight);

	VIDEO_Flush(); // Apply hardware changes

	GX_SetCopyFilter(pMode->aa,pMode->sample_pattern,GX_TRUE,pMode->vfilter);

	VIDEO_Flush(); // Apply hardware changes

	if ( pMode->viHeight == (2*pMode->xfbHeight) )
		GX_SetFieldMode(pMode->field_rendering,GX_ENABLE );
	else
		GX_SetFieldMode(pMode->field_rendering,GX_DISABLE);

	VIDEO_Flush(); // Apply hardware changes

	if (pMode->aa)
		GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
	else
		GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	VIDEO_Flush(); // Apply hardware changes

}



GXRModeObj* WiiManager::InitialiseVideo()
{
	GXRModeObj* pScreenMode( GetBestVideoMode() );

	if (pScreenMode != NULL) {
		SetGXRMode(pScreenMode);

		// Initialise the video system
		VIDEO_Init();

		// incase InitialiseVideo is called multiple times - it may change to another display region later.
		if (m_pFrameBuffer[0] != NULL) { // Has the 1'st frame buffer already been set
			free(MEM_K1_TO_K0(m_pFrameBuffer[0]));
		}
		m_pFrameBuffer[0] = static_cast<u32*>(MEM_K0_TO_K1(SYS_AllocateFramebuffer(pScreenMode)));
		VIDEO_ClearFrameBuffer (pScreenMode, m_pFrameBuffer[0], COLOUR_FOR_WIPE);   

		//
		// DEBUG - ignored when in BUILD_FINAL_RELEASE
		//
		InitDebugConsole(); 

		VIDEO_Configure( pScreenMode );	// Setup the video registers with the chosen mode

		VIDEO_WaitVSync();												// for VI retrace
		VIDEO_SetNextFramebuffer(m_pFrameBuffer[m_uScreenBufferId]);	// Give H/W a starting point.
		VIDEO_SetBlack(FALSE);											// signal output - show our frame buffer
		VIDEO_Flush();  												// Apply the changes

		// 2nd frame buffer - doing this here should reduce screen starting flicker
		// trying to ready the first buffer before the VI displays it
		if (m_pFrameBuffer[1] != NULL) 
		{
			free(MEM_K1_TO_K0(m_pFrameBuffer[1]));
		}
		m_pFrameBuffer[1] = static_cast<u32*>(MEM_K0_TO_K1(SYS_AllocateFramebuffer(m_pGXRMode)));
		VIDEO_ClearFrameBuffer (pScreenMode, m_pFrameBuffer[1], COLOUR_FOR_WIPE );

		if ((pScreenMode->viTVMode) & VI_NON_INTERLACE)  
			VIDEO_WaitVSync(); // wait for 2nd interlace to finnish.. is this really needed?

		VIDEO_SetPostRetraceCallback(RetraceCallback);
	}

	return pScreenMode;
}

// Matrix tutorial:- http://www.gamedev.net/reference/articles/article877.asp
void	WiiManager::SetUp3DProjection()
{
	static float Fov = 45.0f; 
	static float aspect = (float)GetScreenWidth() / (float)GetScreenHeight();
	Mtx44 perspective;
	guPerspective(perspective, Fov, aspect, 1.0f, 50000.0f);
	GX_LoadProjectionMtx(perspective, GX_PERSPECTIVE);

	//------------------------------------------------------------
	GetFrustum()->SetFrustumView( GetScreenWidth() , GetScreenHeight() ); 	// setup culling logic 
	//------------------------------------------------------------

	GX_InvalidateTexAll();
	GX_InvVtxCache ();

	GX_ClearVtxDesc();							// WHY BOTHER DOING IT HERE?
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

	//GX_VTXFMT0 used for lines
	GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);   // Each call to a vertex attribute must match the order: Position, normal, color, texcoord. 
	GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);  //  used ??????
	GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

	//  GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
	//    GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

	//GX_VTXFMT1 used for ploy textures
	GX_SetVtxAttrFmt (GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
	GX_SetVtxAttrFmt (GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	//GX_VTXFMT2 used for ploy textures NO ALPHA
	GX_SetVtxAttrFmt (GX_VTXFMT2, GX_VA_POS, GX_POS_XY, GX_S16, 0);
	GX_SetVtxAttrFmt (GX_VTXFMT2, GX_VA_CLR0, GX_CLR_RGB, GX_RGB565, 0);
	GX_SetVtxAttrFmt (GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_U16, 0);

	//	GX_SetVtxAttrFmt (GX_VTXFMT2, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
	//	GX_SetVtxAttrFmt (GX_VTXFMT2, GX_VA_CLR0, GX_CLR_RGB, GX_RGB565, 0);
	////	GX_SetVtxAttrFmt (GX_VTXFMT2, GX_VA_CLR0, GX_CLR_RGB, GX_RGB8, 0);
	////    GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	//    GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_U16, 0);

	//GX_VTXFMT3 used for test colour polys
	GX_SetVtxAttrFmt (GX_VTXFMT3, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt (GX_VTXFMT3, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

	//GX_VTXFMT4 used for colour with normals + tex
	GX_SetVtxAttrFmt (GX_VTXFMT4, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);   // Each call to a vertex attribute must match the order: Position, normal, color, texcoord. 
	GX_SetVtxAttrFmt (GX_VTXFMT4, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt (GX_VTXFMT4, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt (GX_VTXFMT4, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	//GX_VTXFMT5 used for tex
	GX_SetVtxAttrFmt (GX_VTXFMT5, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt (GX_VTXFMT5, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt (GX_VTXFMT5, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	//GX_VTXFMT6 used for test colour polys
	GX_SetVtxAttrFmt (GX_VTXFMT6, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt (GX_VTXFMT6, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

	GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_TRUE);

	GX_SetNumChans(1);
	GX_SetNumTexGens(1);


	//    GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);  // solid fills
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

	GX_SetAlphaUpdate(GX_TRUE);  // does this have any effect?

	GX_SetCullMode(GX_CULL_BACK);
	// GX_SetCullMode(GX_CULL_NONE);
}

// Matrix tutorial:- http://www.gamedev.net/reference/articles/article877.asp

//------------------------------

u32* WiiManager::GetCurrentFrame() const 
{ 
	return m_pFrameBuffer[m_uScreenBufferId]; 
}


u32 WiiManager::GetScreenBufferId() const 
{ 
	return m_uScreenBufferId; 
}

void WiiManager::RetraceCallback(u32 retraceCnt) {

	// ??? hangs Util::DoResetSystemCheck();

	if ( (WPAD_ButtonsUp(0) & WPAD_BUTTON_HOME)!= 0 )
	{
 		Singleton<WiiManager>::GetInstanceByPtr()->SetGameState(WiiManager::eExit);
	}
}


void WiiManager::SwapScreen(int bState, bool WaitVBL)
{
	++m_uFrameCounter;
	++m_uScreenBufferId; 
	if (m_uScreenBufferId >= GetMaxScreenBuffers())  {
		m_uScreenBufferId = 0;
	}

	GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE); // clear zbuffer  
	
	//GX_SetColorUpdate(GX_TRUE); // EFB color-buffer updates 

	GX_CopyDisp( GetCurrentFrame() , bState); // mainly to clear the z buffer (don't think its possible to clear just the Z buffer ? )
	
	GX_DrawDone();  // wait for the immediate rendering - DO THIS BEFORE GX_CopyDisp or you will create a "screen tearing" effect

	VIDEO_SetNextFramebuffer( GetCurrentFrame() );

	VIDEO_Flush();

	if ( WaitVBL ) {
		VIDEO_WaitVSync();  // GET RID OF WAIT ???? MAYBE use RetraceCallback ...hmmm set flag and do update when ready???? 
	}
}

void WiiManager::CreateSettingsFromXmlConfiguration(std::string FileName)
{
	TiXmlDocument doc( FileName.c_str() );
	if ( doc.LoadFile() ) {
		TiXmlHandle docHandle( &doc );
		TiXmlHandle Data( docHandle.FirstChild( "Data" ) );

		// do we have a valid 'data' root
		if (Data.Element() != NULL) {
			// Catch things like;
			// <Variables><AmountStars>3000</AmountStars> ... </Variables>
			//

			TiXmlElement* pChild =  Data.FirstChild( "Variables" ).FirstChildElement().ToElement();
			for( TiXmlElement* pElem(pChild); pElem!=NULL; pElem=pElem->NextSiblingElement() ) {
				const char* const pKey(pElem->Value());
				const char* const pText(pElem->GetText());
				if (pKey && pText) {
					//printf("%s:%s",pKey,pText ); ///debug
					m_VariablesContainer[(HashLabel)pKey] = atof( pText ) ;
				}
			}

			// Catch things like;
			// <Graphics FileName="new.tga">
			//	 <AddImage Name="PlayersShip32x32"  StartX="0" StartY="0"	ItemWidth="32" ItemHeight="32" NumberItems="4"/>
			//	 ... </Graphics>
			//

			TiXmlElement* pChild2 =  Data.FirstChild( "Graphics" ).ToElement();	
			do {
				TiXmlElement* pGraphics =  pChild2->FirstChildElement(); //  ->FirstChildElement().ToElement();

				std::string IconSetFileName = pChild2->Attribute("FileName");

				//printf("_______%s_______",IconSetFileName.c_str());

				for( TiXmlElement* pGraphicsElem(pGraphics); pGraphicsElem!=NULL; pGraphicsElem=pGraphicsElem->NextSiblingElement() )
				{
					string Key(pGraphicsElem->Value());
					if (Key=="AddImage") {
						FrameInfo Info;
						string  Name =	pGraphicsElem->Attribute("Name");
						pGraphicsElem->Attribute("StartX",&Info.iStartX);
						pGraphicsElem->Attribute("StartY",&Info.iStartY);
						pGraphicsElem->Attribute("ItemWidth",&Info.iItemWidth);
						pGraphicsElem->Attribute("ItemHeight",&Info.iItemHeight);
						pGraphicsElem->Attribute("NumberItems",&Info.iNumberItems);

						const char* str = pGraphicsElem->Attribute("Direction");
						string Direction = "";
						if (str!=NULL){
							Direction = str;
						}

						if (Direction=="Down") {
							Info.eDirection = ImageManager::eDown;
						}
						else {
							Info.eDirection = ImageManager::eRight;
						}

						Info.Name = Name;
						Info.FileName = IconSetFileName;
						m_FrameinfoContainer[ (HashLabel)Name ] = Info;
					} 
				}
				pChild2 = pChild2->NextSiblingElement("Graphics");
			}while (pChild2 != NULL);


			// *** Sounds ***
			TiXmlElement* pSounds =  Data.FirstChild( "Sounds" ).FirstChildElement().ToElement();
			for( TiXmlElement* pElement(pSounds); pElement!=NULL; pElement=pElement->NextSiblingElement() )
			{
				string Key(pElement->Value());
				if (Key=="AddSound") {
					FileInfo Info(pElement->Attribute("FileName"),pElement->Attribute("Name"));
					m_SoundinfoContainer.push_back( Info );
				}
			}

			// *** Fonts ***
			TiXmlElement* pFonts =  Data.FirstChild( "Fonts" ).FirstChildElement().ToElement();
			for( TiXmlElement* pElement(pFonts); pElement!=NULL; pElement=pElement->NextSiblingElement() )
			{
				string Key(pElement->Value());
				if (Key=="AddFont") {
					bool Priority(false);
					pElement->QueryBoolAttribute("Priority", &Priority) ;
					FileInfo Info(pElement->Attribute("FileName"),pElement->Attribute("Name"), Priority  );
					m_FontinfoContainer.push_back( Info );
				}
			}

			// *** LWO ***
			TiXmlElement* pLwo =  Data.FirstChild( "LightWaveObjects" ).FirstChildElement().ToElement();
			for( TiXmlElement* pElement(pLwo); pElement!=NULL; pElement=pElement->NextSiblingElement() )
			{
				string Key(pElement->Value());
				if (Key=="AddLWO") {
					FileInfo Info(pElement->Attribute("FileName"),pElement->Attribute("Name"));
					int temp(0);
					if (pElement->Attribute("UseModelsNormalData",&temp) != NULL)
						Info.m_bNorms = (bool)temp;
					if (pElement->Attribute("IndexLayerForBones",&temp) != NULL)
						Info.m_IndexLayerForBones = temp;

					//printf("UseModelsNormalData %d",Info.m_bNorms);
					m_LwoinfoContainer.push_back( Info );
				}
			}

			// *** Downloads ***
			TiXmlElement* pOgg =  Data.FirstChild( "DownloadFiles" ).FirstChildElement().ToElement();
			for( TiXmlElement* pElement(pOgg); pElement!=NULL; pElement=pElement->NextSiblingElement() )
			{
				string Key(pElement->Value());
				if (Key=="AddURI") {
					if ( (pElement->Attribute("URI")!=0) && (pElement->Attribute("FullDownloadPath")!=0) ) {
						FileInfo Info( pElement->Attribute("URI"), Util::urlDecode( pElement->Attribute("URI") ) );
						Info.FullDownloadPath = pElement->Attribute("FullDownloadPath");
						m_DownloadinfoContainer.push_back( Info );
						//printf("\n%s\n%s\n",Info.FileName.c_str(),Info.LogicName.c_str());
					}
				}
			}

			// *** Raw tga's ***
			TiXmlElement* pRawTga =  Data.FirstChild( "RawTga" ).FirstChildElement().ToElement();
			for( TiXmlElement* pElement(pRawTga); pElement!=NULL; pElement=pElement->NextSiblingElement() ) {
				string Key(pElement->Value());
				if (Key=="AddRawTga") {
					FileInfo Info(pElement->Attribute("FileName"),pElement->Attribute("Name"));
					m_RawTgainfoContainer.push_back( Info );
				}
			}


			// *** Languages ***
			vector<string> WorkingTempLanguagesFoundContainer;
			TiXmlElement* pLanguages =  Data.FirstChild( "Languages" ).FirstChildElement().ToElement();
			for( TiXmlElement* pElement(pLanguages); pElement!=NULL; pElement=pElement->NextSiblingElement() ) {
				string Key(pElement->Value());
				if (Key=="AddLanguage") {
					std::string Text(pElement->Attribute("Name"));
					//printf("AddLanguage: %s ", Text.c_str());
					WorkingTempLanguagesFoundContainer.push_back( Text );
				}
			}

			// fill up the different language containers - but only with the those found above 
			for ( vector<string>::iterator LangIter(WorkingTempLanguagesFoundContainer.begin());LangIter!=WorkingTempLanguagesFoundContainer.end(); ++LangIter ) {
				for ( TiXmlElement* pElement(pLanguages); pElement!=NULL; pElement=pElement->NextSiblingElement() ) {
					if ( pElement->Value() == *LangIter ) {
						for ( TiXmlElement* pAddText(pElement->FirstChildElement()); pAddText!=NULL; pAddText=pAddText->NextSiblingElement() ) {
							string Value( pAddText->Value() );
							if (  Value == "AddText" ){
								std::string AttributeText( pAddText->FirstAttribute()->Value() );
								std::string NameText( pAddText->FirstAttribute()->Name() );
								
								map< string, string >*  ptr =  &m_SupportedLanguages[*LangIter];
								(*ptr)[NameText] = AttributeText;

						//		printf("Add %s Text,%s: %s", LangIter->c_str(),NameText.c_str(),(*ptr)[NameText].c_str() );
							}
						}
					}
				}
			}
			
			WorkingTempLanguagesFoundContainer.clear();

			//printf("XML Setttings complete");
		}
		//else
		//{
		//	if (docHandle.FirstChildElement().Element() == NULL)
		//		printf("The root label is missing");
		//	else
		//		printf("The root is not labeled <Data>");
		//}
	}

	// Check the XML for any obvious mistakes
	static const std::string ErrorString = "Misssing %s in the XML";
	if (m_VariablesContainer.empty()) ExitPrintf(ErrorString.c_str(),"variables");
	if (m_SoundinfoContainer.empty()) ExitPrintf(ErrorString.c_str(),"sound");
	if (m_FontinfoContainer.empty()) ExitPrintf(ErrorString.c_str(),"fonts");
	if (m_LwoinfoContainer.empty()) ExitPrintf(ErrorString.c_str(),"lwo's");
}

void WiiManager::TextBoxWithIcon(float x, float y, int w, int h, EAlign eAlign, HashLabel IconName, const char*  formatstring, ...)
{
	char buff[128];
	va_list args;
	va_start(args, formatstring);
	vsnprintf( buff, sizeof(buff), formatstring, args);
	TextBox(buff, x, y, w, h, eAlign);

	const static int Gap(4);
	Image* pImage = GetImageManager()->GetImage(IconName);

	pImage->DrawImageXYZ( Gap + x + (pImage->GetWidth()/2), y + (h/2), 0,255,0,1.0f);
}

void WiiManager::TextBox(const std::string& rText, float x, float y, EAlign eAlign)
{
	TextBox(rText,x,y,GetFontManager()->GetTextWidth(rText)+8,GetFontManager()->GetFont(HashString::SmallFont)->GetHeight()+6, eAlign);
}

void WiiManager::TextBox(const std::string& rText, float x, float y, int w, int h, EAlign eAlign)
{
	Util3D::Trans(x,y);
	Draw_Util::DrawRectangle(0,0,w,h,98,0,0,0);

	int TextWidth(GetFontManager()->GetTextWidth(rText,HashString::SmallFont));
	int TextHeight(GetFontManager()->GetFont(HashString::SmallFont)->GetHeight());

	// maybe add both aligns as enums later if needed?
	if (eAlign==eCentre)
	{
		GetFontManager()->DisplayTextCentre(rText, w/2,h/2, 222,HashString::SmallFont);
	}
	else if (eAlign==eRight)
	{
		int Diffx = w - TextWidth;
		int Diffy = (h - TextHeight)/2;
		GetFontManager()->DisplayText(rText, Diffx - 4, Diffy,222,HashString::SmallFont);
	}
	else if (eAlign==eLeft)
	{
		int Diffx = 0;
		int Diffy = (h - TextHeight)/2;
		GetFontManager()->DisplayText(rText, Diffx + 4, Diffy,222,HashString::SmallFont);
	}	
}

void WiiManager::TextBox(float x, float y, int w, int h, EAlign eAlign, const char*  formatstring, ...) 
{
	char buff[128];
	va_list args;
	va_start(args, formatstring);
	vsnprintf( buff, sizeof(buff), formatstring, args);
	TextBox(buff, x, y, w, h, eAlign);
}


void WiiManager::InitDebugConsole(int ScreenOriginX, int ScreenOriginY)
{	
#ifndef BUILD_FINAL_RELEASE

	// Initialise the console, required for printf (hopefully this can be called more than once, can't find any uninit?)
	console_init(	m_pFrameBuffer[0],
		ScreenOriginX,
		ScreenOriginY,
		m_pGXRMode->fbWidth,
		m_pGXRMode->xfbHeight,
		(m_pGXRMode->fbWidth) * VI_DISPLAY_PIX_SZ);

	VIDEO_ClearFrameBuffer (m_pGXRMode, m_pFrameBuffer[0], COLOUR_FOR_WIPE );  // put back the orginal wipe
	//printf("\x1b[4;1H");
	//printf("InitDebugConsole\n\n");
#endif
}

void WiiManager::InitGameResources()
{
	m_ImageManager->AddImage(WiiFile::GetGamePath() + "graphics/Cog_256x256.png");

	// *** fonts ***
	for ( vector<FileInfo>::iterator Iter( GetFontInfoBegin());	Iter !=  GetFontInfoEnd() ; ++Iter ) {
		if (Iter->Priority) {
			GetFontManager()->LoadFont(WiiFile::GetGamePath() + Iter->FileName, Iter->LogicName);	
			MyThread.m_Data.Message = Iter->FileName;
		}
	}

	//
	//  Display thread - safe to start we now have minimal gfx neededs
	//
	MyThread.Start(ThreadData::LOADING);  // thread drawing now uses all the fonts

	// Now load anything not marked as high priority
	for ( vector<FileInfo>::iterator Iter( GetFontInfoBegin());	Iter !=  GetFontInfoEnd() ; ++Iter ) {
		if ( ! Iter->Priority ) {
			GetFontManager()->LoadFont(WiiFile::GetGamePath() + Iter->FileName, Iter->LogicName);
			MyThread.m_Data.Message = Iter->FileName;
		}
	}

	// *** Add 3D Objects -  Lightwave 3d Objects, LWO ***
	for ( vector<FileInfo>::iterator Iter( GetLwoInfoBegin());	Iter !=  GetLwoInfoEnd() ; ++Iter ) {
		MyThread.m_Data.Message = Iter->FileName;

		GetRender3D()->Add3DObject( WiiFile::GetGamePath() + Iter->FileName, !Iter->m_bNorms ) 
							->SetName(Iter->LogicName);

		// ADDED THREAD AT LOADING, SO THIS NEED TO BE DONE LATER ON ONCE THE LOADING THREAD HAS COMPLETED 
		// MOVED ..... GetRender3D()->CreateDisplayList(Iter->LogicName);  

		// check if this model need bones, load up the same file again but with "[BONE]" appended.
		if (Iter->m_IndexLayerForBones != -1) {
			// We don't create a display list for this part, holds BONEs
			GetRender3D()->Add3DObject( WiiFile::GetGamePath() + Iter->FileName, !Iter->m_bNorms, Iter->m_IndexLayerForBones) 
					->SetName(Iter->LogicName+"[BONE]");
		}
	}

	//***  Sounds, WAV or OGG ***
	for ( vector<FileInfo>::iterator SoundInfoIter( GetSoundinfoBegin()); SoundInfoIter !=  GetSoundinfoEnd() ; ++SoundInfoIter ) {
		MyThread.m_Data.Message = SoundInfoIter->FileName;
		GetSoundManager()->LoadSound(WiiFile::GetGamePath()+SoundInfoIter->FileName,SoundInfoIter->LogicName);
	}

	// *** tga ***
	for ( vector<FileInfo>::iterator TgaIter( GetRawTgaInfoBegin()); TgaIter !=  GetRawTgaInfoEnd() ; ++TgaIter ) {
		MyThread.m_Data.Message = TgaIter->FileName;
		RawTgaInfo Info;
		Info.m_pPixelData = (Tga::PIXEL*) Tga::LoadTGA( WiiFile::GetGamePath() + TgaIter->FileName, Info.m_pTgaHeader ); 
		m_RawTgaInfoContainer[(HashLabel)TgaIter->LogicName] = Info;
	}

	//---------------------------------------------------------------------------
	// Collect the file names (unique) required for loading images
	std::set<std::string> ContainerOfUnqueFileNames;  // using "set" to store unique names
	for ( map<HashLabel,FrameInfo>::iterator FrameInfoIter( GetFrameinfoBegin() );FrameInfoIter !=  GetFrameinfoEnd() ; ++FrameInfoIter ) {
		ContainerOfUnqueFileNames.insert( FrameInfoIter->second.FileName );
	}

	ImageManager* pImageManager( GetImageManager() );
	for (std::set<std::string>::iterator NameIter(ContainerOfUnqueFileNames.begin()); NameIter != ContainerOfUnqueFileNames.end(); ++NameIter ) {
		
		// pick the one's that match the current file being looked at
		if (pImageManager->BeginGraphicsFile(WiiFile::GetGamePath() + *NameIter )) {

			// Cut-out sprite graphics into memory from graphic file
			for ( map<HashLabel,FrameInfo>::iterator FrameInfoIter( GetFrameinfoBegin() );FrameInfoIter !=  GetFrameinfoEnd() ; ++FrameInfoIter )
			{

				MyThread.m_Data.Message = (*NameIter) + " : " + FrameInfoIter->second.Name;

				FrameInfo& Info( FrameInfoIter->second );
				if (Info.FileName != *NameIter) // Skip anything that not in the current file being processed
					continue;

				int frameStart = pImageManager->AddImage(
					Info.iStartX,Info.iStartY,
					Info.iItemWidth,Info.iItemHeight,
					Info.iNumberItems,Info.eDirection );

				StartAndEndFrameInfo frameinfo = {frameStart, frameStart + Info.iNumberItems - 1 };
				m_FrameEndStartConstainer[(HashLabel)Info.Name] = frameinfo;  // Store new frame details

				//printf("%s x:%d y:%d (%dx%d) FrameStart:%d Frames:%d", 
				//	Info.Name.c_str(),Info.iStartX,Info.iStartY,
				//	Info.iItemWidth,Info.iItemHeight,frameStart,Info.iNumberItems );
			}
			pImageManager->EndGraphicsFile();
		}
	}
	
	m_pMenuManager->BuildMenus();

	ScanMusicFolder( true );
	PlayMusic();
	
	GetPanelManager()->Add( "score ", new Panels3DScore);
	GetPanelManager()->Add( "scrap ", new Panels3DScrap);
}

void WiiManager::FinalInitGameResources_NOT_DISPLAY_THREAD_SAFE() {
	
	// *** LWO's
	for ( vector<FileInfo>::iterator Iter( GetLwoInfoBegin());	Iter !=  GetLwoInfoEnd() ; ++Iter ) {
		GetRender3D()->CreateDisplayList(Iter->LogicName);  
	}

}

void WiiManager::ScanMusicFolder( bool ResetPlayQue )
{
	m_MusicFilesContainer.clear();
	WiiFile::GetFolderFileNames( WiiFile::GetGameMusicPath(), &m_MusicFilesContainer );

	if ( ! m_MusicFilesContainer.empty() )
	{
		m_MusicFilesContainer.begin()->b_ThisSlotIsBeingUsed = true; // tag one tune to start with
		if (ResetPlayQue)
		{
			LoadMusic();
		}
	}
}

void WiiManager::LoadMusic()
{
	FileInfo* pInfo( GetCurrentMusicInfo() );
	if (pInfo!=NULL)
	{
		if (m_pMusicData->pData != NULL)  // stop anything that's currently playing
		{
			SetMusicVolume( 0 );

			char* pTemp = new char[5];
			memset (pTemp,0,5);
			memcpy (pTemp,m_pMusicData->pData,4);
			string Header2 = pTemp;
			if (Header2 == "OggS")
			{
				GetSoundManager()->m_OggPlayer.Stop();
			}
			else
			{
			    GRRMOD_Unload();  // safe to call even if nothing is playing (uses a internal sndPlaying flag)
			}

			free(m_pMusicData->pData);
		}
 
		// Load the new music
		WiiFile::ReadFile( pInfo->FileName, m_pMusicData );  // + fillout info structure (holds... data,size)
	}
}

FileInfo* WiiManager::GetCurrentMusicInfo()
{
	for ( vector<FileInfo>::iterator Iter( m_MusicFilesContainer.begin() ); Iter != m_MusicFilesContainer.end() ; ++Iter )
	{
		if (Iter->b_ThisSlotIsBeingUsed == true)
			return &(*Iter);
	}
	return NULL;
}


void WiiManager::SetMusicVolume(int Volume)
{
	if (m_pMusicData->pData==NULL)
		return;

	char* pTemp = new char[5];
	memset (pTemp,0,5);
	memcpy (pTemp,m_pMusicData->pData,4);
	string Header2 = pTemp;


	if (Header2 == "OggS")
	{
		if ( Volume > 0) {
			GetSoundManager()->m_OggPlayer.SetVolume(Volume * (255/5));
			GetSoundManager()->m_OggPlayer.Pause(false);
		}
		else {
			GetSoundManager()->m_OggPlayer.Pause();
		}
	}
	else
	{
		if ( Volume > 0 ) {
			if (GRRMOD_GetPause()) {
				GRRMOD_Pause();
			}
			GRRMOD_SetVolume( Volume*51,  Volume*51 );
		}
		else {
			GRRMOD_Pause();
		}
	}
}


void WiiManager::PlayMusic()
{
	if (m_pMusicData->pData==NULL)
			return;

	char* pTemp = new char[5];
	memset (pTemp,0,5);
	memcpy (pTemp,m_pMusicData->pData,4);
	string Header2 = pTemp;
	if (Header2 == "OggS") {
		FileInfo* Info = GetCurrentMusicInfo();
		if (Info != NULL) {
			GetSoundManager()->m_OggPlayer.Play(m_pMusicData->pData, (s32)Info->Size, 255 );
		}
	}
	else {
		GRRMOD_SetMOD(m_pMusicData->pData, m_pMusicData->Size );
		static const int MenuMusicVolume(255);
		GRRMOD_Start( MenuMusicVolume );
	}
}


void WiiManager::NextMusic()
{
	for ( vector<FileInfo>::iterator Iter( m_MusicFilesContainer.begin() ); Iter != m_MusicFilesContainer.end() ; ++Iter )
	{
		if (Iter->b_ThisSlotIsBeingUsed == true)
		{
			Iter->b_ThisSlotIsBeingUsed = false;

			// get the next module in the list to play, it wraps back to the start when needed
			++Iter;
			if (Iter == m_MusicFilesContainer.end())
				Iter = m_MusicFilesContainer.begin();

			Iter->b_ThisSlotIsBeingUsed = true;

			LoadMusic();
			PlayMusic();

			break;
		}
	}
}

string WiiManager::GetNameOfCurrentMusic()
{
	for ( vector<FileInfo>::iterator Iter( m_MusicFilesContainer.begin() ); Iter != m_MusicFilesContainer.end() ; ++Iter )
	{
		if (Iter->b_ThisSlotIsBeingUsed)
			return Iter->LogicName;
	}
	return "nothing";
}

int WiiManager::GetConfigValueWithDifficultyApplied(HashLabel Name) 
{
	GetMenuManager()->SetMenuGroup("OptionsMenu");
	float Value = GetXmlVariable(Name);
	return ApplyDifficultyFactor( Value );
}

float WiiManager::ApplyDifficultyFactor(float Value) 
{
	int Index = GetMenuManager()->GetMenuItemIndex(HashString::DifficultySetting);
	if (Index==0)
		Value *= GetXmlVariable( (HashLabel)"easy" );
	else if (Index==1)
		Value *= GetXmlVariable( (HashLabel)"medium" );
	else if (Index==2)
		Value *= GetXmlVariable( (HashLabel)"hard" );

	return Value;
}

string WiiManager::GetText(string Name)
{
	if (m_SupportedLanguages.empty())
		return "-";

	map< string, string >* ptr( &m_SupportedLanguages[m_Language] );
	return (*ptr)[Name]; // todo  ... some checking needed here
}

Image* WiiManager::GetSpaceBackground() { 
	return GetImageManager()->GetImage(HashString::SpaceBackground01); 
}

//===================================================================

void WiiManager::MainLoop() 
{	
	SetGameState(WiiManager::eIntro);

	while (true)
	{
		SetMusicVolume( 5 ); // 0 to 5, 0 is off - 5 is max

		switch( (int)GetGameState() )
		{
		case WiiManager::eIntro:
			IntroLoop();
			break;
		case WiiManager::eMenu:
			MenusLoop();
			break;
		case WiiManager::ePlaying:
			PlayLoop();
			break;
		case WiiManager::eExit:
			return; // quit game
		}
	}
}

void WiiManager::IntroLoop()
{
	m_pGameLogic->InitialiseGame();
	GetCamera()->SetCameraView( 0, 0, -(579.4f));
	m_pGameLogic->ClearBadContainer();
	m_pGameLogic->InitialiseIntro();

	do 
	{	
		Util::DoResetSystemCheck();  // place this in scanpads_extra as as a wrapper
		
		m_pGameLogic->Intro();

		if (GetGameState() == WiiManager::eExit) {
			break;
		}
	} while( (WPAD_ButtonsUp(0) & (WPAD_BUTTON_A | WPAD_BUTTON_B) )== 0 );

	SetGameState(WiiManager::eMenu);
}

void WiiManager::PlayLoop()
{
	if (GetMusicEnabled())
		SetMusicVolume( GetIngameMusicVolume() );
	else
		SetMusicVolume( 0 );


	m_pGameLogic->InitialiseGame();

	WPAD_ScanPads();   // Do a read now, this will flush out anything old ready for a fresh start.
	
	while (IsGameStatePlaying()) 
	{
		m_pGameLogic->DoGameLogic();

		if ( m_pGameLogic->IsEndLevelTrigger() )  {
			if (WPAD_ButtonsUp(0) & WPAD_BUTTON_A) {
				SetGameState(WiiManager::eIntro);
			}
		}
		Util::DoResetSystemCheck();
	}
}

void WiiManager::MenusLoop()
{
	SetGameState(WiiManager::eMenu);
	GetCamera()->SetCameraView( 0, 0, -(579.4f));
	GetMenuScreens()->SetTimeOutInSeconds();
	
	m_pGameLogic->InitMenu();

	m_pMenuManager->BuildMenus( true );

	while (GetGameState() != WiiManager::eExit) 
	{	
		Util::DoResetSystemCheck();

		if (IsGameStateMenu())
		{
			GetMenuManager()->SetMenuGroup("MainMenu");

			m_pGameLogic->MoonRocksLogic();
			m_pGameLogic->CelestialBodyLogic();

			GetMenuScreens()->DoMenuScreen();
	
			if ( (WPAD_ButtonsUp(0) & WPAD_BUTTON_B) || (GetMenuScreens()->HasMenuTimedOut()) )  
			{
				SetGameState(WiiManager::eIntro);
				break;
			}

			if ( (WPAD_ButtonsUp(0) & WPAD_BUTTON_A) != 0) 
			{
				HashLabel Name = GetMenuManager()->GetSelectedMenu();


				if (Name == HashString::Options)
				{
					SetGameState(WiiManager::eOptions);
				}
				else if (Name == HashString::Start_Game)
				{
					SetGameState(WiiManager::ePlaying);
					break;
				}
				else if (Name == HashString::Intro)
				{
					SetGameState(WiiManager::eIntro);
					break;
				}
				else if ( Name ==  HashString::Change_Tune )
				{
					// The menu screen hs been set above, so now just set the message
					Util3D::CameraIdentity();
					Util3D::TransRot(-280,-150,0, M_PI *0.5f );
					GetFontManager()->DisplayTextCentre("Loading...", 0,0,200,HashString::SmallFont);
					SwapScreen();  
					//----------------------------------------------------------------------
					// next frame - now need to set the menu screen again before the text message
					GetMenuScreens()->DoMenuScreen();  // draw menu screen again
					Util3D::CameraIdentity();
					Util3D::TransRot(-280,-150,0, M_PI *0.5f );
					GetFontManager()->DisplayTextCentre("Loading...", 0,0,200,HashString::SmallFont);
					SwapScreen();  
					//---------------------------------------------------------------------
					NextMusic();

					continue; // don't do the loops screen swap since its already done
				}
				else if ( Name == HashString::download_extra_music ) {

					//
					// Download online files
					//
					GetUpdateManager()->DownloadFilesListedInConfiguration(false);

					m_MusicStillLeftToDownLoad = false;
					m_pMenuManager->BuildMenus(true);
				}
				else if (Name == HashString::Credits)
				{
					SetGameState(WiiManager::eCredits);
				}
				else if (Name == HashString::Controls)
				{
					SetGameState(WiiManager::eControls);
				}
			}

			SwapScreen();  
		}

		// ***************
		// *** CREDITS ***
		// ***************
		if (IsGameStateCredits())
		{
			GetMenuScreens()->DoCreditsScreen();
			if ( WPAD_ButtonsUp(0) & (WPAD_BUTTON_A|WPAD_BUTTON_B) )
			{
				//GetMenuScreens()->SetTimeOutInSeconds();
				SetGameState(WiiManager::eMenu);
			}
		}

		// ****************
		// *** Controls ***
		// ****************
		if (IsGameStateControls())
		{
			GetMenuScreens()->DoControlsScreen();
			if ( WPAD_ButtonsUp(0) & (WPAD_BUTTON_A|WPAD_BUTTON_B) )
			{
				//GetMenuScreens()->SetTimeOutInSeconds();
				SetGameState(WiiManager::eMenu);
			}
		}

		// ***************
		// *** OPTIONS ***
		// ***************
		if (IsGameStateOptions())
		{
			GetMenuScreens()->DoOptionsScreen();		
			GetMenuManager()->SetMenuGroup("OptionsMenu");

			if ( WPAD_ButtonsUp(0) & (WPAD_BUTTON_B) )
			{
				GetMenuScreens()->SetTimeOutInSeconds();
				SetGameState(WiiManager::eMenu);
				m_pMenuManager->BuildMenus( true );
			}

			if ( WPAD_ButtonsUp(0) & (WPAD_BUTTON_A) )
			{
				HashLabel Name = GetMenuManager()->GetSelectedMenu();		

				GetMenuManager()->SetMenuGroup("OptionsMenu");

				if (Name == (HashLabel)"Ingame_Music")
				{
					GetMenuManager()->AdvanceMenuItemText(HashString::IngameMusicState);
					SetMusicEnabled( GetMenuManager()->GetMenuItemIndex(HashString::IngameMusicState) );
				}
				else if (Name == (HashLabel)"Ingame_MusicVolume")
				{
					GetMenuManager()->AdvanceMenuItemText(HashString::IngameMusicVolumeState);
					SetIngameMusicVolume(GetMenuManager()->GetMenuItemIndex(HashString::IngameMusicVolumeState));

					//printf("%d",GetIngameMusicVolume());

					if ( (GetIngameMusicVolume()==0) && (GetMusicEnabled()) )
					{
						SetMusicEnabled(false);
						GetMenuManager()->AdvanceMenuItemText(HashString::IngameMusicState);
					}
					if ( (GetIngameMusicVolume()>0) && (!GetMusicEnabled()) )
					{
						SetMusicEnabled(true);
						GetMenuManager()->AdvanceMenuItemText(HashString::IngameMusicState);
					}
				}
				
				else if (Name == (HashLabel)"Difficulty_Level")
				{
					GetMenuManager()->AdvanceMenuItemText(HashString::DifficultySetting);
					SetDifficulty(GetMenuManager()->GetMenuItemText(HashString::DifficultySetting));
				}
				else if (Name == (HashLabel)"Set_Language")
				{
					GetMenuManager()->AdvanceMenuItemText(HashString::LanguageSetting);
					SetLanguage( GetMenuManager()->GetMenuItemText(HashString::LanguageSetting) );
				}
				else if ((Name == (HashLabel)"Back") )
				{
					GetMenuScreens()->SetTimeOutInSeconds();
					SetGameState(WiiManager::eMenu);
				}
				m_pMenuManager->BuildMenus( true );
			}
		}
	}
}


// ==================================================================================
// *** ProFile Section ***
// ==================================================================================
#if defined (BUILD_FINAL_RELEASE)
void WiiManager::profiler_create(profiler_t* pjob, std::string name) { pjob; name; } // STUB
void WiiManager::profiler_start(profiler_t* pjob) {pjob;} // STUB
void WiiManager::profiler_stop(profiler_t* pjob) {pjob;} // STUB
void WiiManager::profiler_reset(profiler_t* pjob) {pjob;} // STUB
string WiiManager::profiler_output(profiler_t* pjob) {pjob; return "";} // STUB
#else

void WiiManager::profiler_create(profiler_t* pjob, std::string name)
{
	profiler_reset(pjob);
	pjob->name = name;
}

void WiiManager::profiler_start(profiler_t* pjob)
{
	pjob->active = 1;
	pjob->start_time = Util::timer_gettime();
};

void WiiManager::profiler_stop(profiler_t* pjob)
{
	if(pjob->active)
	{
		u64 stop_time = Util::timer_gettime();

		pjob->duration = stop_time - pjob->start_time;
		pjob->total_time = pjob->duration;
		if (pjob->duration < pjob->min_time)
		{
			if (pjob->duration != 0)
				pjob->min_time = pjob->duration;
		}
		
		if (pjob->duration > pjob->max_time)
			pjob->max_time = pjob->duration;
		
		pjob->no_hits++;
		pjob->active = 0;
	}
};

void WiiManager::profiler_reset(profiler_t* pjob)
{
	pjob->active = 0;
	pjob->no_hits = 0;
	pjob->total_time = 0;
	pjob->min_time = ULONG_LONG_MAX;
	pjob->max_time = 0;
	pjob->start_time = 0;
};

string WiiManager::profiler_output(profiler_t* pjob)
{
	u64 min_us = Util::TicksToMicrosecs(pjob->min_time);
	u64 max_us = Util::TicksToMicrosecs(pjob->max_time);
	u64 dur_us = Util::TicksToMicrosecs(pjob->duration);
	
	std::stringstream ss;
	ss << "items, min:" << std::setw( 4 ) << std::setfill( '0' ) << min_us 
		<< " max:" << std::setw( 4 ) << std::setfill( '0' ) << max_us 
		<< " now:" << std::setw( 4 ) << std::setfill( '0' ) << dur_us 
		<< " " << pjob->name;

	return ss.str();
}
#endif
// ==================================================================================
// *** ProFile End ***
// ==================================================================================
