//
// WiiManager - Singleton class
//
#include <stdio.h>
#include <stdlib.h>
#include <wiiuse/wpad.h>
#include <vector>
#include <malloc.h>
#include <stdarg.h> 
#include <vector>
#include <set>

#include <math.h>

#include <limits.h>

#include "WiiManager.h"
#include "font.h"
#include "Image.h"
#include "ImageManager.h"
#include "fontManager.h"
#include "SpriteManager.h"
#include "InputDeviceManager.h"
#include "SoundManager.h"
#include "MapManager.h"
#include "debug.h"
#include "Util3D.h"
#include "Util.h"
#include "TinyXML/TinyXML.h"
#include "FontManager.h"
#include "HashString.h"
#include "Menu.h"
#include "GameLogic.h"
#include "config.h"
#include "MenuScreens.h"
#include "Mission.h"
#include "MessageBox.h"
#include "GameDisplay.h"

#include "Vessel.h"
//#include "profiler/timer.h"

#define DEBUGCONSOLESTATE	( eDebugConsoleOn )  // its ignored (off) in final release
#define COLOUR_FOR_WIPE		( COLOR_BLACK )  // crash at startup colour


WiiManager::WiiManager() :	m_pGXRMode(NULL), m_gp_fifo(NULL), 
							m_uScreenBufferId(eFrontScreenBuffer), 
							m_uFrameCounter(0),
							m_ImageManager(NULL), 
							m_MapManager(NULL), 
							m_SoundManager(NULL),
							m_Camera(NULL),						
							m_ViewportX(0),
							m_ViewportY(0),
							m_GameState(eIntro),
							m_Language("English")
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


}

WiiManager::~WiiManager()
{
	//	UnInitScreen();

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
}

void WiiManager::InitWii()
{
//	Vessel::m_pWii = Singleton<WiiManager>::GetInstanceByPtr();

//	Vessel v;
//	v.Init();

	m_pGameLogic->Init();
	m_pGameDisplay->Init();
	m_pMenuScreens->Init();
	m_MessageBox->Init();

	WiiFile::InitFileSystem();

	////InitScreen();  
	// try to do everything after this - that way we get to see the debug console
	//maybe my ExitPrintf() should check is the display is up and running - creating one when needed for user error messages???
	// do this after the display is setup - its a debug dependancy thing
	//Screen specific
	SetViewport(0.0f,0.0f);
	InitialiseVideo();
	InitGX();
	SetUp3DProjection();

	// XML configuration - this places sections of data into specificly named containers found in the code
	CreateSettingsFromXmlConfiguration(Util::GetGamePath() + "GameConfiguration.xml");

	FinaliseInputDevices();  // maybe do this first? but has dependancy on screen size
	//maybe do kind of part1 and part2 of this, that way the wiimote can get up and going sooner????

	int Timeout( GetXmlVariable( HashString::WiiMoteIdleTimeoutInSeconds ) );
	if (Timeout>0)
	{
		WPAD_SetIdleTimeout(Timeout); 
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
		vmode->viWidth = 678;
		//vmode->viWidth = 720;
	}
	//	else
	//	{
	//		vmode->viWidth = 678;
	//	}


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

	//
	//
	//
	//	GXRModeObj* mode = VIDEO_GetPreferredMode(NULL); // get default video mode
	//
	//	
	//	// choose the desired video mode
	//	switch(0)
	//	{
	//		case 1: // NTSC (480i)
	//			mode = &TVNtsc480IntDf;
	//			break;
	//		case 2: // Progressive (480p)
	//			mode = &TVNtsc480Prog;
	//			break;
	//		case 3: // PAL (50Hz)
	//			mode = &TVPal528IntDf;
	//			break;
	//		case 4: // PAL (60Hz)
	//			mode = &TVEurgb60Hz480IntDf;
	//			break;
	//		default:
	//			mode = VIDEO_GetPreferredMode(NULL);
	//			// force TVNtsc480Prog for anyone using ComponentCable
	//			//if(VIDEO_HaveComponentCable())
	//			//	mode = &TVNtsc480Prog;
	//			break;
	//	}
	//
	//	////// check for progressive scan
	//	////if (mode->viTVMode == VI_TVMODE_NTSC_PROG)
	//	////	progressive = true;
	//	////else
	//	////	progressive = false;
	//
	//
	//	bool bPalMode( mode == &TVPal528IntDf );
	//
	////////if( CONF_GetAspectRatio()== CONF_ASPECT_16_9 )
	////////{
	////////	mode->viWidth = 678;
	////////	mode->viXOrigin = (VI_MAX_WIDTH_PAL - 678)/2;
	////////}
	//
	//
	//////????????
	//
	//	if (CONF_GetAspectRatio() == CONF_ASPECT_16_9)
	//	{
	//		static const float ratio( 16.0f / 9.0f ); 
	//		float width (680.0f);
	//
	//		float height = width / ratio;
	//
	//		height+=60;
	//
	//		mode->fbWidth = 640;
	//		mode->efbHeight = height;
	//		mode->viWidth = width;
	//
	//		//mode->fbWidth = 640;
	//		//mode->efbHeight = 456;
	//		//mode->viWidth = 686;
	//
	//		mode->xfbHeight = height;
	//		mode->viHeight = height;
	//	}
	//	else
	//	{
	//		static const float ratio( 4.0f / 3.0f );
	//		if (bPalMode)
	//			mode = &TVPal574IntDfScale;
	//
	//		mode->viWidth = 672;
	//	}
	//
	//
	//		// centre picture
	//		if (bPalMode)
	//		{
	//			mode->viXOrigin = (VI_MAX_WIDTH_PAL - mode->viWidth) / 2;
	//			mode->viYOrigin = (VI_MAX_HEIGHT_PAL - mode->viHeight) / 2;
	//		}
	//		else
	//		{
	//			mode->viXOrigin = (VI_MAX_WIDTH_NTSC - mode->viWidth) / 2;
	//			mode->viYOrigin = (VI_MAX_HEIGHT_NTSC - mode->viHeight) / 2;
	//		}
	//
	//
	//	return mode;
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

		printf ("using %d for GX Fifo",GXFifoBufferSize);
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

void WiiManager::SwapScreen()
{

	GX_DrawDone();  // flush & wait for the pipline (could use callback here)


	GX_CopyDisp( GetCurrentFrame() , GX_TRUE); // mainly to clear the z buffer (don't think its possible to clear just the Z buffer ? )
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
	GetFontManager()->DisplaySmallText(Buffer,x,y);
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
					printf("%s:%s",pKey,pText ); ///debug
					m_VariablesContainer[(HashLabel)pKey] = atof( pText ) ;
				}
			}

			// Catch things like;
			// <Graphics FileName="new.tga">
			//	 <AddImage Name="ShipFrames"  StartX="0" StartY="0"	ItemWidth="32" ItemHeight="32" NumberItems="4"/>
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

					printf("UseModelsNormalData %d",Info.m_bNorms);
					m_LwoinfoContainer.push_back( Info );
				}
			}

			// *** Mod's ***
			TiXmlElement* pMod =  Data.FirstChild( "TrackerModules" ).FirstChildElement().ToElement();
			for( TiXmlElement* pElement(pMod); pElement!=NULL; pElement=pElement->NextSiblingElement() )
			{
				string Key(pElement->Value());
				if (Key=="AddMod") 
				{
					FileInfo Info(pElement->Attribute("FileName"),pElement->Attribute("Name"));
					m_ModinfoContainer.push_back( Info );
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
					printf("AddLanguage: %s ", Text.c_str());
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

			printf("XML Setttings complete");
		}
		else
		{
			if (docHandle.FirstChildElement().Element() == NULL)
				printf("The root label is missing");
			else
				printf("The root is not labeled <Data>");
		}
	}

	// Check the XML for any obvious mistakes
	static const std::string ErrorString = "Misssing %s in the XML";
	if (m_VariablesContainer.empty()) ExitPrintf(ErrorString.c_str(),"variables");
	if (m_SoundinfoContainer.empty()) ExitPrintf(ErrorString.c_str(),"sound");
	if (m_FontinfoContainer.empty()) ExitPrintf(ErrorString.c_str(),"fonts");
	if (m_LwoinfoContainer.empty()) ExitPrintf(ErrorString.c_str(),"lwo's");
	if (m_ModinfoContainer.empty()) ExitPrintf(ErrorString.c_str(),"mod's");

	

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
		GetFontManager()->DisplaySmallTextCentre(rText, w/2,h/2, 222);
	}
	else if (eAlign==eRight)
	{
		int Diffx = w - TextWidth;
		int Diffy = (h - TextHeight)/2;
		GetFontManager()->DisplaySmallText(rText, Diffx - 4, Diffy,222);
	}
	else if (eAlign==eLeft)
	{
		int Diffx = 0;
		int Diffy = (h - TextHeight)/2;
		GetFontManager()->DisplaySmallText(rText, Diffx + 4, Diffy,222);
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
#warning *** DONT FORGET TO REMOVE THIS IN THE FINAL BUILDS ***
	//noticed 'InitDebugConsole' clears the given buffer with black

	// Initialise the console, required for printf (hopefully this can be called more than once, can't find any uninit?)
	console_init(	m_pFrameBuffer[0],
		ScreenOriginX,
		ScreenOriginY,
		640,480,
		//m_pGXRMode->fbWidth,
		//m_pGXRMode->xfbHeight,
		m_pGXRMode->fbWidth * VI_DISPLAY_PIX_SZ);

	VIDEO_ClearFrameBuffer (m_pGXRMode, m_pFrameBuffer[0], COLOUR_FOR_WIPE );  // put back the orginal wipe

	//CON_InitEx(GetGXRMode(), ScreenOriginX,ScreenOriginY,m_pGXRMode->fbWidth,m_pGXRMode->xfbHeight);

	printf("InitDebugConsole\n\n");
#endif
}

void WiiManager::ProgramStartUp()
{
	// *** fonts ***
	for ( vector<FileInfo>::iterator Iter( GetFontInfoBegin());	Iter !=  GetFontInfoEnd() ; ++Iter )
	{
		GetFontManager()->LoadFont(Util::GetGamePath() + Iter->FileName, Iter->LogicName);
	}
	
//	GetCamera()->InitialiseCamera();
//	GetGameDisplay()->DisplaySimpleMessage("Loading...");

	// *** Add 3D Objects -  Lightwave 3d Objects, LWO ***
	for ( vector<FileInfo>::iterator Iter( GetLwoInfoBegin());	Iter !=  GetLwoInfoEnd() ; ++Iter )
	{
	
		Render.Add3DObject( Util::GetGamePath() + Iter->FileName, !Iter->m_bNorms ) 
			->SetName(Iter->LogicName);
			Render.CreateDisplayList(Iter->LogicName);  


		if (Iter->m_IndexLayerForBones != -1)
		{
			// We don't create a display list for this part, holds BONEs
			Render.Add3DObject( Util::GetGamePath() + Iter->FileName, !Iter->m_bNorms, Iter->m_IndexLayerForBones) 
					->SetName(Iter->LogicName+"[BONE]");
		}
	
	}


	//***  Sounds, WAV or OGG ***
	for ( vector<FileInfo>::iterator SoundInfoIter( GetSoundinfoBegin()); SoundInfoIter !=  GetSoundinfoEnd() ; ++SoundInfoIter )
	{
		GetSoundManager()->LoadSound(Util::GetGamePath()+SoundInfoIter->FileName,SoundInfoIter->LogicName);
	}

	// *** Raw tga ***
	for ( vector<FileInfo>::iterator SoundInfoIter( GetRawTgaInfoBegin()); SoundInfoIter !=  GetRawTgaInfoEnd() ; ++SoundInfoIter )
	{
		RawTgaInfo Info;
		Info.m_pTinyLogo = (Tga::PIXEL*) Tga::LoadTGA( Util::GetGamePath() + SoundInfoIter->FileName, Info.m_pTinyLogoHeader ); 
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
		if (pImageManager->BeginGraphicsFile(Util::GetGamePath() + *NameIter ))
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

				printf("%s x:%d y:%d (%dx%d) FrameStart:%d Frames:%d", 
					Info.Name.c_str(),Info.iStartX,Info.iStartY,
					Info.iItemWidth,Info.iItemHeight,frameStart,Info.iNumberItems );
			}
			pImageManager->EndGraphicsFile();
		}
	}

	//========================================
	// Main Menu - one time setup
	int y=-92;
	static const int step=68;
	float width=400;
	static const int height=60;
	width*=0.65f;
	GetMenuManager()->SetMenuGroup("MainMenu");
	GetMenuManager()->AddMenu((-width*0.5)-4, y, width,height,"Options");
	GetMenuManager()->AddMenu((+width*0.5)+4, y, width,height,"Controls");
	y+=step;
	GetMenuManager()->AddMenu((-width*0.5)-4, y, width,height,"Credits");
	GetMenuManager()->AddMenu((+width*0.5)+4, y, width,height,"Intro");
	y+=step*1.5f;
	width*=1.50f;
	GetMenuManager()->AddMenu(0, y, width,height,"Start Game");
	y+=step*1.5;
	width*=0.65f;
	GetMenuManager()->AddMenu(0, y, width,height,"Quit");
	//==========================================================
	// Options Menu - one time setup
	int x=-96;
	y=-70;
	GetMenuManager()->SetMenuGroup("OptionsMenu");
	GetMenuManager()->AddMenu(x, y, 390, height, "In-game Music");
	GetMenuManager()->AddMenu(x+300, y, 1, height, "IngameMusicState",true)->AddTextItem("off")->AddTextItem("on")->SetCurrentItemIndex(1);

	y+=step;
	GetMenuManager()->AddMenu(x, y , 390, height, "Difficulty Level");
	GetMenuManager()->AddMenu(x+300, y, 1, height, "DifficultySetting",true)->AddTextItem("easy")->AddTextItem("medium")->AddTextItem("hard")->SetCurrentItemIndex(1);

	y+=step;
	GetMenuManager()->AddMenu(x, y , 390, height, "Set Language");
	GetMenuManager()->AddMenu(x+300, y, 1, height, "LanguageSetting",true)->AddTextItem("English")->AddTextItem("Italian")->AddTextItem("Esperanto")->SetCurrentItemIndex(0);

	y+=step+30;
	GetMenuManager()->AddMenu(0, y , 600, height, "Back");


	//Get the tracker module into memory
	for ( vector<FileInfo>::iterator Iter( GetModInfoBegin());	Iter !=  GetModInfoEnd() ; ++Iter )
	{
		string name( Util::GetGamePath() + Iter->FileName );
		FILE* pFile( WiiFile::FileOpenForRead(name.c_str()) );
		fseek (pFile , 0, SEEK_END);
		uint FileSize( ftell (pFile) );
		rewind(pFile); 
		m_pModuleTrackerData = (u8*) malloc (sizeof(char) * FileSize);
		size_t result = fread (m_pModuleTrackerData,1,FileSize,pFile);
		if (result != FileSize) 
			ExitPrintf ("mod/Music Reading error"); 
		else
			fclose(pFile); 

		break; // TODO - store more then one - just for now take the first one we find
	}
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
	u64 duration;
	
	stop_time = Util::timer_gettime();
	
	if(pjob->active)
	{
		start_time = pjob->start_time;
		duration = stop_time - start_time;
		pjob->total_time += duration;
		
		if (duration < pjob->min_time)
			pjob->min_time = duration;
		
		if (duration > pjob->max_time)
			pjob->max_time = duration;
		
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

void WiiManager::profiler_output(profiler_t* pjob,int x,int y)
{
	u64 min_us = Util::TicksToMicrosecs(pjob->min_time);
	u64 max_us = Util::TicksToMicrosecs(pjob->max_time);
	
	const static int gap = 18;
	Printf(x,y+=gap,"%s duration min:%llu max:%llu", pjob->name.c_str(),min_us,max_us );
}

int WiiManager::GetConfigValueWithDifficultyApplied(HashLabel Name) 
{
	GetMenuManager()->SetMenuGroup("OptionsMenu");
	float value = GetXmlVariable(Name);
	value *= GetXmlVariable( (HashLabel)GetMenuManager()->GetMenuItemText(HashString::DifficultySetting) );
	return value;
}
float WiiManager::ApplyDifficultyFactor(float Value) 
{
	float Factor = GetXmlVariable( (HashLabel)GetMenuManager()->GetMenuItemText(HashString::DifficultySetting) );
	return Value * Factor;
}