//
// WiiManager - Singleton class
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <malloc.h>
//#include "MapManager.h"
//#include "HTTP/HTTP_Util.h"
//#include "profiler/timer.h"

#include <stdarg.h>   //  for things like va_list
#include <vector>
#include <set>
#include <math.h>
#include <limits.h>
#include <wiiuse/wpad.h>

#include "WiiManager.h"
#include "ImageManager.h"
#include "MenuManager.h"
#include "InputDeviceManager.h"
#include "SoundManager.h"
#include "FontManager.h"
#include "URIManager.h"
#include "SetupGame.h"
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
#include "config.h"
#include "debug.h"
#include <string>
#include <sstream>
//#include <dirent.h>
//#include <sys/stat.h>
#include "oggplayer/oggplayer.h"

#define DEBUGCONSOLESTATE	( eDebugConsoleOn )  // it's ignored (off) in final release
#define COLOUR_FOR_WIPE		( COLOR_BLACK )  // crash at startup colour

WiiManager::WiiManager() :	
							m_pGXRMode(NULL), 
							m_gp_fifo(NULL), 
							m_uScreenBufferId(eFrontScreenBuffer), 
							m_uFrameCounter(0),
							m_ImageManager(NULL), 
							m_MapManager(NULL), 
							m_SoundManager(NULL),
							m_Camera(NULL),		
							m_URLManager(NULL),
							m_UpdateManager(NULL),
							m_SetUpGame(NULL),
							m_ViewportX(0),
							m_ViewportY(0),
							m_GameState(eIntro),
							m_Language("English"),
							m_bMusicEnabled(true),
							m_Difficulty("medium"),
							m_MusicStillLeftToDownLoad(false),
							m_IngameMusicVolume(3),
							m_pModuleTrackerData(NULL)
{ 
	m_pFrameBuffer[0] = NULL;
	m_pFrameBuffer[1] = NULL;
	m_ImageManager			= new ImageManager;
	m_FontManager			= new FontManager;
	m_InputDeviceManager	= new InputDeviceManager;
	m_MapManager			= new MapManager;
	m_SoundManager			= new SoundManager;
	m_Camera				= new Camera;
	m_pMenuManager			= new MenuManager;
	m_pGameLogic			= new GameLogic;
	m_pGameDisplay			= new GameDisplay;
	m_pMenuScreens			= new MenuScreens;
	m_MissionManager		= new MissionManager;
	m_MessageBox			= new MessageBox;
	m_URLManager			= new URLManager;
	m_UpdateManager			= new UpdateManager;
	m_SetUpGame				= new SetUpGame;

}

WiiManager::~WiiManager()
{
	// note: manager's do their own housekeeping
	delete m_ImageManager;		
	delete m_FontManager;	
	delete m_InputDeviceManager;
	delete m_MapManager;	
	delete m_Camera;
	delete m_pMenuManager;	
	delete m_pGameLogic;
	delete m_pMenuScreens;		
	delete m_MissionManager;
	delete m_MessageBox;
	delete m_URLManager;
	delete m_UpdateManager;
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

	if (m_ModuleTrackerPlayerInterface.mod.modraw!=NULL)
	{
		MODPlay_Stop( &m_ModuleTrackerPlayerInterface );
		MODPlay_Unload( &m_ModuleTrackerPlayerInterface );
	}
}

void WiiManager::InitWii()
{
	m_pGameLogic->Init();
	m_pGameDisplay->Init();
	m_pMenuScreens->Init();
	m_MessageBox->Init();
	m_SetUpGame->Init();
	m_UpdateManager->Init();

	Util::SetUpPowerButtonTrigger();

	WiiFile::InitFileSystem();

	// try to do everything after this - that way we get to see the debug console
	//maybe my ExitPrintf() should check is the display is up and running - creating one when needed for user error messages???
	// do this after the display is setup - its a debug dependancy thing

	//Screen specific
	SetViewport(0.0f,0.0f);
	InitialiseVideo();
	InitGX();
	SetUp3DProjection();

	// XML configuration - this places sections of data into specificly named containers found in the code
	CreateSettingsFromXmlConfiguration(WiiFile::GetGamePath() + "GameConfiguration.xml");

	FinaliseInputDevices();  // maybe do this first? but has dependancy on screen size
	//maybe do kind of part1 and part2 of this, that way the wiimote can get up and going sooner????

	if (! m_VariablesContainer.empty())
	{
		int Timeout( GetXmlVariable( HashString::WiiMoteIdleTimeoutInSeconds) );
		if ( Timeout > 0 )
		{
			WPAD_SetIdleTimeout(Timeout); 
		}
	}
}

GXRModeObj* WiiManager::GetBestVideoMode()
{
	GXRModeObj* vmode = VIDEO_GetPreferredMode(NULL); // get default video mode

	bool pal = false;

	if (vmode == &TVPal528IntDf)
	{
		pal = true;
		vmode = &TVPal574IntDfScale;
	}

	if (CONF_GetAspectRatio() == CONF_ASPECT_16_9)
	{
	      vmode->viWidth = 678;  // probably top limit for stretching the display onto your TV
	}
	//else
	//{
	//		vmode->viWidth = 672;  // not tested...this may work ok
	//}


	if (pal)
	{
		vmode->viXOrigin = (VI_MAX_WIDTH_PAL - vmode->viWidth) / 2;
		vmode->viYOrigin = (VI_MAX_HEIGHT_PAL - vmode->viHeight) / 2;
	}
	else
	{
		vmode->viXOrigin = (VI_MAX_WIDTH_NTSC - vmode->viWidth) / 2;
		vmode->viYOrigin = (VI_MAX_HEIGHT_NTSC - vmode->viHeight) / 2;
	}

	s8 hoffset = 0;

	if (CONF_GetDisplayOffsetH(&hoffset) == 0)
		vmode->viXOrigin += hoffset;

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


//private
void WiiManager::InitialiseVideo()
{
	//	GXRModeObj* pScreenMode( VIDEO_GetPreferredMode(NULL) );

	GXRModeObj* pScreenMode( GetBestVideoMode() );

	if (pScreenMode != NULL)
	{
		SetGXRMode( pScreenMode );		// keep a working copy - bit nasty as it creates dependances

		// Initialise the video system
		VIDEO_Init();

		// incase InitialiseVideo is called multiple times - it may change to another display region later.
		if (m_pFrameBuffer[0] != NULL) // Has the 1'st frame buffer already been set
		{
			free(MEM_K1_TO_K0(m_pFrameBuffer[0]));
		}
		m_pFrameBuffer[0] = static_cast<u32*>(MEM_K0_TO_K1(SYS_AllocateFramebuffer(m_pGXRMode)));
		VIDEO_ClearFrameBuffer (GetGXRMode(), m_pFrameBuffer[0], COLOUR_FOR_WIPE);   

		if (DEBUGCONSOLESTATE == eDebugConsoleOn)
		{
			InitDebugConsole();
		}

		VIDEO_Configure( GetGXRMode() );	// Setup the video registers with the chosen mode

		VIDEO_WaitVSync();												// for VI retrace
		VIDEO_SetNextFramebuffer(m_pFrameBuffer[m_uScreenBufferId]);	// Give H/W a starting point.
		VIDEO_SetBlack(FALSE);											// signal output - show our frame buffer
		VIDEO_Flush();  												// Apply the changes

		// 2nd frame buffer - doing this here should reduce screen starting hickups
		// As we need to prepare the first buffer before the VI displays it
		if (m_pFrameBuffer[1] != NULL) 
		{
			free(MEM_K1_TO_K0(m_pFrameBuffer[1]));
		}
		m_pFrameBuffer[1] = static_cast<u32*>(MEM_K0_TO_K1(SYS_AllocateFramebuffer(m_pGXRMode)));
		VIDEO_ClearFrameBuffer (GetGXRMode(), m_pFrameBuffer[1], COLOUR_FOR_WIPE );

		if ((GetGXRMode()->viTVMode) & VI_NON_INTERLACE)  
			VIDEO_WaitVSync(); // wait for 2nd interlace to finnish.. is this really needed?

	}
}

void WiiManager::FinaliseInputDevices() const
{
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);  // use everthing
	if (m_pGXRMode==NULL)
		ExitPrintf("m_pGXRMode is null, did you forget to initialise screen first");
	else
		//		WPAD_SetVRes(WPAD_CHAN_ALL, GetScreenWidth(), GetScreenHeight() );  // resolution of IR
		//		WPAD_SetVRes(WPAD_CHAN_ALL, GetViWidth(), GetScreenHeight() );  // resolution of IR
		WPAD_SetVRes(WPAD_CHAN_ALL, 640+100, 480+100 );  // resolution of IR

	//WPAD_SetVRes(WPAD_CHAN_ALL, GetGXRMode()->fbWidth, GetGXRMode()->xfbHeight);  // resolution of IR
}



// Matrix tutorial:- http://www.gamedev.net/reference/articles/article877.asp
void	WiiManager::SetUp3DProjection()
{
	f32 w = GetScreenWidth();
	f32 h = GetScreenHeight();
	static float Fov = 45.0f; 
	static float aspect = w/h;
	Mtx44 perspective;
	guPerspective(perspective, Fov, aspect, 1.0f, 50000.0f);
	GX_LoadProjectionMtx(perspective, GX_PERSPECTIVE);


	//------------------------------------------------------------
	SetFrustumView(w,h); 	// setup culling logic 
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

	//GX_VTXFMT4 used for tex with normals
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

void WiiManager::SwapScreen(bool bState)
{

	//TODO - Time this waste???
	GX_DrawDone();  // flush & wait for the pipline (could use callback here)


	if (bState)
		GX_CopyDisp( GetCurrentFrame() , GX_TRUE); // mainly to clear the z buffer (don't think its possible to clear just the Z buffer ? )
	else
		GX_CopyDisp( GetCurrentFrame() , GX_FALSE);

	GX_Flush();

	++m_uScreenBufferId; 
	if (m_uScreenBufferId >= GetMaxScreenBuffers()) 
		m_uScreenBufferId=0;
	// calling 'VIDEO_WaitVSync' at the incorrect time can cause screen tearing when the hardware flips
	VIDEO_WaitVSync(); //must wait here for a full VBL before the next frame is set

	VIDEO_SetNextFramebuffer( GetCurrentFrame() );
	VIDEO_Flush();

	++m_uFrameCounter;
}


void WiiManager::Printf(int x, int y, const char* pFormat, ...)
{
	static const u32 BufferSize(128);

	va_list tArgs;
	va_start(tArgs, pFormat);
	char Buffer[BufferSize+1];
	vsnprintf(Buffer, BufferSize, pFormat, tArgs);	
	va_end(tArgs);

	Util3D::Trans(GetCamera()->GetCamX(),GetCamera()->GetCamY());
	GetFontManager()->DisplayText(Buffer,x,y,200,HashString::SmallFont);
}

void WiiManager::CreateSettingsFromXmlConfiguration(std::string FileName)
{
	TiXmlDocument doc( FileName.c_str() );
	if ( doc.LoadFile() )
	{
		//map<string,int> VariablesContainer;

		TiXmlHandle docHandle( &doc );
		TiXmlHandle Data( docHandle.FirstChild( "Data" ) );

		// do we have a valid 'data' root
		if (Data.Element() != NULL)
		{
			// Catch things like;
			// <Variables><AmountStars>3000</AmountStars> ... </Variables>
			//

			TiXmlElement* pChild =  Data.FirstChild( "Variables" ).FirstChildElement().ToElement();
			for( TiXmlElement* pElem(pChild); pElem!=NULL; pElem=pElem->NextSiblingElement() )
			{
				const char* const pKey(pElem->Value());
				const char* const pText(pElem->GetText());
				if (pKey && pText) 
				{
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
					if (Key=="AddImage") 
					{
						FrameInfo Info;
						string  Name =	pGraphicsElem->Attribute("Name");
						pGraphicsElem->Attribute("StartX",&Info.iStartX);
						pGraphicsElem->Attribute("StartY",&Info.iStartY);
						pGraphicsElem->Attribute("ItemWidth",&Info.iItemWidth);
						pGraphicsElem->Attribute("ItemHeight",&Info.iItemHeight);
						pGraphicsElem->Attribute("NumberItems",&Info.iNumberItems);

						const char* str = pGraphicsElem->Attribute("Direction");
						string Direction = "";
						if (str!=NULL)
						{
							Direction = str;
						}

						if (Direction=="Down")
						{
							Info.eDirection = ImageManager::eDown;
						}
						else
						{
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
				if (Key=="AddSound") 
				{
					FileInfo Info(pElement->Attribute("FileName"),pElement->Attribute("Name"));
					m_SoundinfoContainer.push_back( Info );
				}
			}

			// *** Fonts ***
			TiXmlElement* pFonts =  Data.FirstChild( "Fonts" ).FirstChildElement().ToElement();
			for( TiXmlElement* pElement(pFonts); pElement!=NULL; pElement=pElement->NextSiblingElement() )
			{
				string Key(pElement->Value());
				if (Key=="AddFont") 
				{
					FileInfo Info(pElement->Attribute("FileName"),pElement->Attribute("Name"));
					m_FontinfoContainer.push_back( Info );
				}
			}

			// *** LWO ***
			TiXmlElement* pLwo =  Data.FirstChild( "LightWaveObjects" ).FirstChildElement().ToElement();
			for( TiXmlElement* pElement(pLwo); pElement!=NULL; pElement=pElement->NextSiblingElement() )
			{
				string Key(pElement->Value());
				if (Key=="AddLWO") 
				{
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

			//////// *** Mod's ***
			//////TiXmlElement* pMod =  Data.FirstChild( "TrackerModules" ).FirstChildElement().ToElement();
			//////for( TiXmlElement* pElement(pMod); pElement!=NULL; pElement=pElement->NextSiblingElement() )
			//////{
			//////	string Key(pElement->Value());
			//////	if (Key=="AddMod") 
			//////	{
			//////		FileInfo Info(pElement->Attribute("FileName"),pElement->Attribute("Name"));
			//////		m_ModinfoContainer.push_back( Info );
			//////	}
			//////}



			// *** Downloads ***
			TiXmlElement* pOgg =  Data.FirstChild( "DownloadFiles" ).FirstChildElement().ToElement();
			for( TiXmlElement* pElement(pOgg); pElement!=NULL; pElement=pElement->NextSiblingElement() )
			{
				string Key(pElement->Value());
				if (Key=="AddURI") 
				{
					if ( (pElement->Attribute("URI")!=0) && (pElement->Attribute("FullDownloadPath")!=0) )
					{
						FileInfo Info( pElement->Attribute("URI"), Util::urlDecode( pElement->Attribute("URI") ) );
						Info.FullDownloadPath = pElement->Attribute("FullDownloadPath");
						m_DownloadinfoContainer.push_back( Info );
						//printf("\n%s\n%s\n",Info.FileName.c_str(),Info.LogicName.c_str());
					}
				}
			}

			// *** Raw tga's ***
			TiXmlElement* pRawTga =  Data.FirstChild( "RawTga" ).FirstChildElement().ToElement();
			for( TiXmlElement* pElement(pRawTga); pElement!=NULL; pElement=pElement->NextSiblingElement() )
			{
				string Key(pElement->Value());
				if (Key=="AddRawTga") 
				{
					FileInfo Info(pElement->Attribute("FileName"),pElement->Attribute("Name"));
					m_RawTgainfoContainer.push_back( Info );
				}
			}


			// *** Languages ***
			vector<string> WorkingTempLanguagesFoundContainer;
			TiXmlElement* pLanguages =  Data.FirstChild( "Languages" ).FirstChildElement().ToElement();
			for( TiXmlElement* pElement(pLanguages); pElement!=NULL; pElement=pElement->NextSiblingElement() )
			{
				string Key(pElement->Value());
				if (Key=="AddLanguage") 
				{
					std::string Text(pElement->Attribute("Name"));
					//printf("AddLanguage: %s ", Text.c_str());
					WorkingTempLanguagesFoundContainer.push_back( Text );
				}
			}

			// fill up the different language containers - but only with the those found above 
			for ( vector<string>::iterator LangIter(WorkingTempLanguagesFoundContainer.begin());LangIter!=WorkingTempLanguagesFoundContainer.end(); ++LangIter )
			{
				for ( TiXmlElement* pElement(pLanguages); pElement!=NULL; pElement=pElement->NextSiblingElement() )
				{
					if ( pElement->Value() == *LangIter )
					{
						for ( TiXmlElement* pAddText(pElement->FirstChildElement()); pAddText!=NULL; pAddText=pAddText->NextSiblingElement() )
						{
							string Value( pAddText->Value() );
							if (  Value == "AddText" )
							{
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
	
	//if (m_DownloadinfoContainer.empty()) ExitPrintf(ErrorString.c_str(),"Dowloads's");
	

}

void WiiManager::DrawRectangle(f32 xpos, f32 ypos, f32 w, f32 h, u8 Alpha, u8 r, u8 g, u8 b)
{
	DrawRectangle(xpos, ypos, w, h,Alpha, r,g, b,r,g,b );
}
void WiiManager::DrawRectangle(f32 xpos, f32 ypos, f32 w, f32 h, u8 Alpha, u8 r, u8 g, u8 b,u8 r2, u8 g2, u8 b2  )
{	
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

	float z=0; 

	GX_Begin(GX_QUADS, GX_VTXFMT3,4);		

	GX_Position3f32(xpos, ypos,z);		
	GX_Color4u8(r,g,b,Alpha);        

	GX_Position3f32(xpos+(w), ypos,z);
	GX_Color4u8(r,g,b,Alpha);     

	GX_Position3f32(xpos+(w), ypos+(h),z);         
	GX_Color4u8(r2,g2,b2,Alpha);     

	GX_Position3f32(xpos, ypos+(h),z);
	GX_Color4u8(r2,g2,b2,Alpha);  

	GX_End();
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
	DrawRectangle(0,0,w,h,98,0,0,0);

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
	// *** fonts ***
	for ( vector<FileInfo>::iterator Iter( GetFontInfoBegin());	Iter !=  GetFontInfoEnd() ; ++Iter )
	{
		GetFontManager()->LoadFont(WiiFile::GetGamePath() + Iter->FileName, Iter->LogicName);

		if (Iter->LogicName == "SmallFont")
		{
			// do message as soon as the font becomes available
			GetCamera()->InitialiseCamera();
			GetGameDisplay()->DisplaySmallSimpleMessage("Loading...");
		}
	}
	

	// *** Add 3D Objects -  Lightwave 3d Objects, LWO ***
	for ( vector<FileInfo>::iterator Iter( GetLwoInfoBegin());	Iter !=  GetLwoInfoEnd() ; ++Iter )
	{
	
		Render.Add3DObject( WiiFile::GetGamePath() + Iter->FileName, !Iter->m_bNorms ) 
			->SetName(Iter->LogicName);
			Render.CreateDisplayList(Iter->LogicName);  


		if (Iter->m_IndexLayerForBones != -1)
		{
			// We don't create a display list for this part, holds BONEs
			Render.Add3DObject( WiiFile::GetGamePath() + Iter->FileName, !Iter->m_bNorms, Iter->m_IndexLayerForBones) 
					->SetName(Iter->LogicName+"[BONE]");
		}
	
	}


	//***  Sounds, WAV or OGG ***
	for ( vector<FileInfo>::iterator SoundInfoIter( GetSoundinfoBegin()); SoundInfoIter !=  GetSoundinfoEnd() ; ++SoundInfoIter )
	{
		GetSoundManager()->LoadSound(WiiFile::GetGamePath()+SoundInfoIter->FileName,SoundInfoIter->LogicName);
	}

	// *** Raw tga ***
	for ( vector<FileInfo>::iterator SoundInfoIter( GetRawTgaInfoBegin()); SoundInfoIter !=  GetRawTgaInfoEnd() ; ++SoundInfoIter )
	{
		RawTgaInfo Info;
		Info.m_pTinyLogo = (Tga::PIXEL*) Tga::LoadTGA( WiiFile::GetGamePath() + SoundInfoIter->FileName, Info.m_pTinyLogoHeader ); 
		m_RawTgaInfoContainer[(HashLabel)SoundInfoIter->LogicName] = Info;
	}

	//---------------------------------------------------------------------------
	// Collect the files required for loading images
	std::set<std::string> ContainerOfUnqueFileNames;  // using set to store unique names
	for ( map<HashLabel,FrameInfo>::iterator FrameInfoIter( GetFrameinfoBegin() );FrameInfoIter !=  GetFrameinfoEnd() ; ++FrameInfoIter )
	{
		ContainerOfUnqueFileNames.insert( FrameInfoIter->second.FileName );
	}

			

	ImageManager* pImageManager( GetImageManager() );
	for (std::set<std::string>::iterator NameIter(ContainerOfUnqueFileNames.begin()); NameIter != ContainerOfUnqueFileNames.end(); ++NameIter )
	{
		// pick the one's that match the current file being looked at
		if (pImageManager->BeginGraphicsFile(WiiFile::GetGamePath() + *NameIter ))
		{
			// Cut-out sprite graphics into memory from graphic file
			for ( map<HashLabel,FrameInfo>::iterator FrameInfoIter( GetFrameinfoBegin() );FrameInfoIter !=  GetFrameinfoEnd() ; ++FrameInfoIter )
			{
				FrameInfo& Info( FrameInfoIter->second );

				if (Info.FileName != *NameIter)
					continue;

				int frameStart = pImageManager->AddImage(
					Info.iStartX,Info.iStartY,
					Info.iItemWidth,Info.iItemHeight,
					Info.iNumberItems,Info.eDirection );

				FrameStartEnd frameinfo = {frameStart, frameStart + Info.iNumberItems - 1 };

				m_FrameEndStartConstainer[(HashLabel)Info.Name] = frameinfo;

				//printf("%s x:%d y:%d (%dx%d) FrameStart:%d Frames:%d", 
				//	Info.Name.c_str(),Info.iStartX,Info.iStartY,
				//	Info.iItemWidth,Info.iItemHeight,frameStart,Info.iNumberItems );
			}
			pImageManager->EndGraphicsFile();
		}
	}
	
	BuildMenus();

	ScanMusicFolder();
	PlayMusic();

}

void WiiManager::ScanMusicFolder()
{
	m_MusicFilesContainer.clear();
	WiiFile::GetFolderFileNames( WiiFile::GetGameMusicPath(), &m_MusicFilesContainer );

	if ( m_MusicFilesContainer.empty() )
		return;

	m_MusicFilesContainer.begin()->b_ThisSlotIsBeingUsed = true; // tag one tune to start with

	LoadMusic();
}

void WiiManager::LoadMusic()
{
	FileInfo* pInfo( GetCurrentMusicInfo() );
	if (pInfo!=NULL)
	{
		m_pModuleTrackerData = WiiFile::ReadFile(pInfo->FileName);
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
	if (m_pModuleTrackerData==NULL)
		return;

	char* pTemp = new char[5];
	memset (pTemp,0,5);
	memcpy (pTemp,m_pModuleTrackerData,4);
	string Header2 = pTemp;
	if (Header2 == "OggS")
	{
		OggPlayer Ogg;  // TODO !!!!

		if ( Volume > 0)
		{
			Ogg.SetVolumeOgg(Volume * (255/5));
			Ogg.PauseOgg(false);
		}
		else
			Ogg.PauseOgg();
		
	}
	else
	{
		//if (m_ModuleTrackerPlayerInterface.playing)
		{
			if ( Volume > 0 )
			{
				MODPlay_Start(&m_ModuleTrackerPlayerInterface); 
				MODPlay_SetVolume( &m_ModuleTrackerPlayerInterface, Volume*20,Volume*20);     
			}
			else
				MODPlay_Stop(&m_ModuleTrackerPlayerInterface);
		}
	}
}


void WiiManager::PlayMusic()
{
	if (m_pModuleTrackerData==NULL)
			return;

	char* pTemp = new char[5];
	memset (pTemp,0,5);
	memcpy (pTemp,m_pModuleTrackerData,4);
	string Header2 = pTemp;
	if (Header2 == "OggS")
	{
		//-------------
		if (m_ModuleTrackerPlayerInterface.playing)
		{
			MODPlay_Stop( &m_ModuleTrackerPlayerInterface );
			MODPlay_Unload( &m_ModuleTrackerPlayerInterface );
		}
		//-------------
		
		FileInfo* Info = GetCurrentMusicInfo();

		if (Info != NULL)
		{
			OggPlayer Ogg;
			Ogg.PlayOgg(m_pModuleTrackerData, (s32)Info->Size, 0, OGG_ONE_TIME);
			Ogg.SetVolumeOgg(255);
		}
	}
	else
	{
		MODPlay_Init(&m_ModuleTrackerPlayerInterface);

		MODPlay_SetMOD(&m_ModuleTrackerPlayerInterface, m_pModuleTrackerData);
		MODPlay_Start(&m_ModuleTrackerPlayerInterface); 
		MODPlay_SetVolume( &m_ModuleTrackerPlayerInterface, 100,100);
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

			if (m_pModuleTrackerData!=NULL)
				free(m_pModuleTrackerData);

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

void WiiManager::BuildMenus(bool KeepSettings)
{
	int Music = 1; 
	int Difficulty = 1;
	int Language = 0;
	int MusicVolume = 3;
	string Group = GetMenuManager()->GetMenuGroup(); 

	if ( KeepSettings )
	{
		Music = GetMenuManager()->GetMenuItemIndex(HashString::IngameMusicState);
		Difficulty = GetMenuManager()->GetMenuItemIndex(HashString::DifficultySetting);
		Language = GetMenuManager()->GetMenuItemIndex(HashString::LanguageSetting);
		MusicVolume = GetMenuManager()->GetMenuItemIndex(HashString::IngameMusicVolumeState);
	}


	SetMusicEnabled( (bool) Music );
	SetIngameMusicVolume( MusicVolume );

	GetMenuManager()->ClearMenus();

	//========================================
	// Main Menu - one time setup
	int y=-36;
	int step=24+8;
	int height=24;
	float width=180;
	
	GetMenuManager()->SetMenuGroup("MainMenu");

	GetMenuManager()->AddMenu(-width*0.10, y, width,height,"Start_Game",false,true);
	y+=step;
	GetMenuManager()->AddMenu(-width*0.15, y, width,height,"Options",false,true);
	y+=step;
	GetMenuManager()->AddMenu(-width*0.20, y, width,height,"Intro",false,true);
	y+=step;
	//move this one into options


	if ( m_MusicFilesContainer.size() > 1)
		GetMenuManager()->AddMenu( -width*0.25, y, width,height,"Change_Tune",false,true);
	else
		GetMenuManager()->AddMenu( -width*0.25, y, width,height,"Change_Tune",true,false);
	
	if (m_MusicStillLeftToDownLoad)
		GetMenuManager()->AddMenu( 160, y, 220,height,"download_extra_music",false,false);

	y+=step;
	GetMenuManager()->AddMenu( -width*0.30, y, width,height ,"Controls",false,true );
	y+=step;
	GetMenuManager()->AddMenu( -width*0.35, y, width,height ,"Credits",false,true );

	//==========================================================


	// Options Menu - one time setup
	GetMenuManager()->SetMenuGroup("OptionsMenu");

	int x=0; // centre of screen
	y=-98;
	height=26;
	step=26+8;

	//GetMenuManager()->AddMenu( x - 108, y, 200, height ,"Credits" );
	//GetMenuManager()->AddMenu( x + 108, y, 200, height ,"Controls" );
	y+=step;

	GetMenuManager()->AddMenu(x, y, 600, height, "Ingame_Music",false,true);
	GetMenuManager()->AddMenu(x+222, y, 1, height, "IngameMusicState",true)->
		AddTextItem(GetText("off"))->AddTextItem(GetText("on"))->SetCurrentItemIndex(Music);

	y+=step;
	GetMenuManager()->AddMenu(x, y, 600, height, "Ingame_MusicVolume",false,true);
	GetMenuManager()->AddMenu(x+222, y, 1, height, "IngameMusicVolumeState",true)->
		AddTextItem(("0"))->AddTextItem(("1"))->
		AddTextItem(("2"))->AddTextItem(("3"))->
		AddTextItem(("4"))->AddTextItem(("5"))->SetCurrentItemIndex(MusicVolume);

	y+=step;
	GetMenuManager()->AddMenu(x, y , 600, height, "Difficulty_Level",false,true );
	GetMenuManager()->AddMenu(x+222, y, 1, height, "DifficultySetting",true)->
		AddTextItem(GetText("easy"))->AddTextItem(GetText("medium"))->AddTextItem(GetText("hard"))->SetCurrentItemIndex(Difficulty);

	y+=step;
	GetMenuManager()->AddMenu(x, y , 600, height, "Set_Language",false,true);
	// GetMenuManager()->AddMenu(x+300, y, 1, height, "LanguageSetting",true)->AddTextItem("English")->AddTextItem("Italian")->AddTextItem("Esperanto")->SetCurrentItemIndex(0);
	if ( !m_SupportedLanguages.empty() )
	{
		Menu* NextItem = NULL; 
		//TODO - typedef this !!!
		for (map<string, map<string,string> >::iterator IterSupportedLanguages(m_SupportedLanguages.begin()) ;
			IterSupportedLanguages!=m_SupportedLanguages.end(); ++IterSupportedLanguages)
		{
			// (first is the language, second english word we wish to find in that language
			if (NextItem==NULL)
				NextItem= GetMenuManager()->AddMenu(x+222, y, 1, height, "LanguageSetting",true)->AddTextItem(IterSupportedLanguages->first) ;
			else
				NextItem = NextItem->AddTextItem(IterSupportedLanguages->first) ;
		}
		NextItem->SetCurrentItemIndex(Language); // set the fisrt one as the default language
	}
	y+=step+30;
	GetMenuManager()->AddMenu(0, y , 600, height, "Back");

	GetMenuManager()->SetMenuGroup( Group );

	// From now on text loaded and available - call to CreateSettingsFromXmlConfiguration is needed first
	// default message - may get overwritten later
	GetUpdateManager()->SetMessageVersionReport( GetText("RunningLatestVersion")  + s_ReleaseVersion + " - " + s_DateOfRelease );
}

void WiiManager::SetFrustumView(int w, int h) 
{
	if(h == 0)
		h = 1;

	static const float nearP = 1.0f, farP = 50000.0f;
	static const float angle = 45.0f;
	
	float ratio = w * 1.0 / h;
	m_Frustum.setCamInternals(angle,ratio,nearP,farP);
}



// Profiler section
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
	u64 stop_time;
	u64 start_time;
	//u64 duration;
	
	stop_time = Util::timer_gettime();
	
	if(pjob->active)
	{
		start_time = pjob->start_time;
		pjob->duration = stop_time - start_time;
		pjob->total_time += pjob->duration;
		
		if (pjob->duration < pjob->min_time)
			pjob->min_time = pjob->duration;
		
		if (pjob->duration > pjob->max_time)
			pjob->max_time = pjob->duration;
		
		pjob->no_hits += 1;
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
	ss << pjob->name << " times, min:" << min_us << " max:" << max_us << " now:" << dur_us;

	return ss.str();

	//reutn "%s duration min:%llu max:%llu" + pjob->name.c_str() min_us,max_us );
}

int WiiManager::GetConfigValueWithDifficultyApplied(HashLabel Name) 
{
	GetMenuManager()->SetMenuGroup("OptionsMenu");
	float Value = GetXmlVariable(Name);
	return ApplyDifficultyFactor( Value );

	//GetMenuManager()->SetMenuGroup("OptionsMenu");
	//float value = GetXmlVariable(Name);

	////value *= GetXmlVariable( (HashLabel)GetMenuManager()->GetMenuItemText(HashString::DifficultySetting) );

	//int Index = GetMenuManager()->GetMenuItemIndex(HashString::DifficultySetting);
	//if (Index==0)
	//	value *= GetXmlVariable( (HashLabel)"easy" );
	//else if (Index==1)
	//	value *= GetXmlVariable( (HashLabel)"medium" );
	//else if (Index==2)
	//	value *= GetXmlVariable( (HashLabel)"hard" );

	//printf("%d %f",Index,value);

	//return value;
}
float WiiManager::ApplyDifficultyFactor(float Value) 
{
	///float Factor = GetXmlVariable( (HashLabel)GetMenuManager()->GetMenuItemText(HashString::DifficultySetting) );

	int Index = GetMenuManager()->GetMenuItemIndex(HashString::DifficultySetting);
	if (Index==0)
		Value *= GetXmlVariable( (HashLabel)"easy" );
	else if (Index==1)
		Value *= GetXmlVariable( (HashLabel)"medium" );
	else if (Index==2)
		Value *= GetXmlVariable( (HashLabel)"hard" );

	//return Value * Factor;
	return Value;
}

string WiiManager::GetText(string Name)
{
	if (m_SupportedLanguages.empty())
		return "-";

	//printf(Name.c_str());
	map< string, string >* ptr( &m_SupportedLanguages[m_Language] );
//	printf((*ptr)[Name].c_str());
	return (*ptr)[Name]; // todo  ... some checking needed here
}

//int WiiManager::GetSizeOfDownloadInfoContainer() { return m_DownloadinfoContainer.size(); }

//LWO VAR "index" is seen HERE!!!!