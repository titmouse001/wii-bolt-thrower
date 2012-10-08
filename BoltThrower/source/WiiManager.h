#ifndef WiiManager_H
#define WiiManager_H

#include "Singleton.h"
#include "config.h"
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include "ogc/gx.h"

#include "HashLabel.h"
#include "GameLogic.h"
#include "ImageManager.h"
#include "Panels3D.h"
#include "WiiFile.h"  // for 'FileInfo'


using namespace std;

class ImageManager;
class FontManager;
class InputDeviceManager;
class SoundManager;
class Camera;
class MenuManager;
class GameLogic;
class MenuScreens;
class FrustumR;
class MissionManager;
class MessageBox;
class GameDisplay;
class IntroDisplay;
class URLManager;
class UpdateManager;
class SetUpGame;
class Render3D;
class PanelManager;

struct StartAndEndFrameInfo;

struct FrameInfo
{
	string FileName;
	string Name;
	int iStartX;
	int iStartY;
	int iItemWidth;
	int iItemHeight;
	int iNumberItems;
	ImageManager::EDirection eDirection;
};

#if defined BUILD_FINAL_RELEASE
struct profiler_t { };
#else
struct profiler_t
{
	std::string 	name;
	u32		active;
	u32		no_hits;
	u64		total_time;
	u64		min_time;
	u64		max_time;
	u64		start_time;
	u64		duration;
} ;
#endif



class WiiManager : private SingletonClient
{
public:

	enum EAlign{eRight,eLeft,eCentre};


#define ONE_KILOBYTE (1024)
#define DEFAULT_GX_FIFO_SIZE ( (4*256) * ONE_KILOBYTE)

	WiiManager();
	~WiiManager();

	void InitWii();
	void UnInitWii();

	void PreInitManagers();
	void FinalInitManagers();

	GXRModeObj* InitialiseVideo();

	void InitDebugConsole(int ScreenOriginX = 64, int ScreenOriginY = 0);  // was 20,20

	u32* GetCurrentFrame() const;
	u32 GetScreenBufferId() const;
	void SwapScreen(int bState = GX_TRUE, bool WaitVBL = true); 
	static  void RetraceCallback( u32 retraceCnt );

	enum EScreenBuffer { eFrontScreenBuffer,eBackScreenBuffer, eMaxScreenBuffers };
	u32 GetMaxScreenBuffers() const  {return eMaxScreenBuffers;}
	GXRModeObj* GetGXRMode() const	
	{ 
		#ifndef BUILD_FINAL_RELEASE
		if  (m_pGXRMode==NULL) {
			printf("m_pGXRMode is NULL");  // catch this one in the emulator
			exit(1); 
		}
		#endif

		return m_pGXRMode; 
	}
	void SetGXRMode(GXRModeObj* pMode) 	{ m_pGXRMode = pMode; }

	int GetViWidth() const { return GetGXRMode()->viWidth; }   // width of a scan line - useful for wiimote when sertting resolution of IR
	int GetScreenWidth() const { return GetGXRMode()->fbWidth; }
	int GetScreenHeight() const { return GetGXRMode()->efbHeight; }


	Mtx m_GXmodelView2D;

	// Managers
	ImageManager*		GetImageManager()		{ return m_ImageManager; }
	FontManager*		GetFontManager()		{ return m_FontManager; }
	InputDeviceManager*	GetInputDeviceManager()	{ return m_InputDeviceManager; }
	SoundManager*		GetSoundManager()		{ return m_SoundManager; }
	Render3D*			GetRender3D()	const	{ return m_Render3D; }
	FrustumR*			GetFrustum()	const	{ return m_Frustum; }
	Camera*				GetCamera()	const		{ return m_Camera; }
	GameLogic*			GetGameLogic()	const	{ return m_pGameLogic; }
	MenuScreens*		GetMenuScreens() const	{ return m_pMenuScreens; }
	MessageBox*			GetMessageBox() const	{ return m_MessageBox; }

	GameDisplay*		GetGameDisplay() const	{ return m_pGameDisplay; }

	IntroDisplay*		GetIntroDisplay() const	{ return m_pIntroDisplay; }

	URLManager*			GetURLManager() const	{ return m_URLManager; }
	UpdateManager*		GetUpdateManager() const	{ return m_UpdateManager; }
	PanelManager*		GetPanelManager() const	{ return m_PanelManager; }

	void InitGX(u32 GXFifoBufferSize = DEFAULT_GX_FIFO_SIZE);
	void SetUp2DProjection();
	void SetUp3DProjection();

	void CreateSettingsFromXmlConfiguration(std::string FileName);

	float GetXmlVariable(HashLabel Name) { return m_VariablesContainer[Name]; }

	//sounds
	vector<FileInfo>::iterator GetSoundinfoBegin() { return m_SoundinfoContainer.begin(); }
	vector<FileInfo>::iterator GetSoundinfoEnd() { return m_SoundinfoContainer.end(); }
	//fonts
	vector<FileInfo>::iterator GetFontInfoBegin() { return m_FontinfoContainer.begin(); }
	vector<FileInfo>::iterator GetFontInfoEnd() { return m_FontinfoContainer.end(); }
	//LWO's
	vector<FileInfo>::iterator GetLwoInfoBegin() { return m_LwoinfoContainer.begin(); }
	vector<FileInfo>::iterator GetLwoInfoEnd() { return m_LwoinfoContainer.end(); }
	//RawTga's
	vector<FileInfo>::iterator GetRawTgaInfoBegin() { return m_RawTgainfoContainer.begin(); }
	vector<FileInfo>::iterator GetRawTgaInfoEnd()   { return m_RawTgainfoContainer.end(); }
	//Ogg's
	vector<FileInfo>::iterator GetDownloadInfoBegin() { return m_DownloadinfoContainer.begin(); }
	vector<FileInfo>::iterator GetDownloadInfoEnd() { return m_DownloadinfoContainer.end(); }

	//Image*					m_pSpaceBackground;
	struct RawTgaInfo
	{
		Tga::PIXEL*				m_pPixelData;
		Tga::TGA_HEADER			m_pTgaHeader;
	};

	map<HashLabel,RawTgaInfo> m_RawTgaInfoContainer;

	void TextBox(const std::string& rText, float x, float y, EAlign eAlign);
	void TextBox(const std::string& rText, float x, float y, int w, int h, EAlign eAlign);
	void TextBox(float x, float y, int w, int h, EAlign eAlign, const char*  formatstring, ...);
	void TextBoxWithIcon( float x, float y, int w, int h, EAlign eAlign, HashLabel IconName, const char*  formatstring, ...);

	GXRModeObj* GetBestVideoMode();
	MenuManager* GetMenuManager()			{ return m_pMenuManager; }
	MissionManager* GetMissionManager()		{return  m_MissionManager; }

	enum EGameState{
		eIntro,
		eMenu,
		eCredits,
		ePlaying,
		eDemoMode,
		eHighScore,
		eControls,
		eOptions,
		eExit,
	};
	void		SetGameState(EGameState State)	{ 
		if (m_GameState != WiiManager::eExit) {
			m_GameState = State; 
		}
	}
	EGameState	GetGameState(void)				{ return m_GameState; }
	bool	IsGameStateIntro()					{ return m_GameState==eIntro; }
	bool	IsGameStateMenu()					{ return m_GameState==eMenu; }
	bool	IsGameStateCredits()				{ return m_GameState==eCredits; }
	bool	IsGameStatePlaying()				{ return m_GameState==ePlaying; }
	bool	IsGameStateControls()				{ return m_GameState==eControls; }
	bool	IsGameStateOptions()				{ return m_GameState==eOptions; }
	bool	IsGameStateExit()					{ return m_GameState==eExit; }

	void	InitGameResources();
	void	FinalInitGameResources_NOT_DISPLAY_THREAD_SAFE() ;

	StartAndEndFrameInfo* GetFrameContainer( HashLabel Id) { return &m_FrameEndStartConstainer[Id]; }

	std::map<HashLabel,StartAndEndFrameInfo> m_FrameEndStartConstainer;  

	Image* GetSpaceBackground();

	//-----------------------------------------------------------------
	// Profiler Section
	void profiler_create(profiler_t* pjob, std::string name);
	string profiler_output(profiler_t* pjob);
	void profiler_start(profiler_t* pjob);
	void profiler_stop(profiler_t* pjob);
	void profiler_reset(profiler_t* pjob);
	//------------------------------------------------------------------

	int GetConfigValueWithDifficultyApplied(HashLabel Name);
	float ApplyDifficultyFactor(float Value);

	u32	GetFrameCounter() { return m_uFrameCounter; } 

	void SetDifficulty(string Value) { m_Difficulty = Value; }
	string GetDifficulty() { return m_Difficulty; }
	void SetMusicEnabled(bool bState) { m_bMusicEnabled = bState; }
	bool GetMusicEnabled() { return m_bMusicEnabled; }

	void SetIngameMusicVolume(u8 Value) { m_IngameMusicVolume = Value; }
	u8 GetIngameMusicVolume() { return m_IngameMusicVolume; }

	void SetLanguage(string Language) { m_Language = Language; }
	string GetLanguage() { return m_Language; }
	string GetText(string Name);
	void ScanMusicFolder( bool ResetPlayQue = false );
	void NextMusic();
	void PlayMusic();
	void SetMusicVolume(int Volume);
	FileInfo* GetCurrentMusicInfo();

	string GetNameOfCurrentMusic();

	void GetFolderFileNames(string Path, vector<FileInfo>* rMusicFilesContainer);


	string			m_ExePath;
	bool			m_MusicStillLeftToDownLoad;

	FileMemInfo*	m_pMusicData;  // holds things like mods & oggs

		void MainLoop();

	int GetMusicFilesContainerSize() const { return m_MusicFilesContainer.size(); }
	int GetSupportedLanguagesEmpty() const { return m_SupportedLanguages.empty(); }
	map<string, map<string,string> >::iterator  GetSupportedLanguagesBegin() { return m_SupportedLanguages.begin(); }
	map<string, map<string,string> >::iterator  GetSupportedLanguagesEnd() { return m_SupportedLanguages.end(); }


private:

	std::string LoadMusic();


	u32* 					m_pFrameBuffer[2];
	GXRModeObj*				m_pGXRMode;
	u32*					m_gp_fifo;
	u32						m_uScreenBufferId;
	u32						m_uFrameCounter;
	ImageManager*			m_ImageManager;
	FontManager*			m_FontManager;
	InputDeviceManager*		m_InputDeviceManager;
	SoundManager*			m_SoundManager;
	Render3D*				m_Render3D;  
	FrustumR*				m_Frustum;
	Camera*					m_Camera;
	URLManager*				m_URLManager;
	UpdateManager*			m_UpdateManager;
	u8						m_IngameMusicVolume;
//	SetUpGame*				m_SetUpGame;
	PanelManager*			m_PanelManager;
	MenuManager*			m_pMenuManager;
	MissionManager*			m_MissionManager;
	GameLogic*				m_pGameLogic;
	GameDisplay*			m_pGameDisplay;

	IntroDisplay*			m_pIntroDisplay;


	MenuScreens*			m_pMenuScreens;
	MessageBox*				m_MessageBox;
	EGameState				m_GameState;
	string					m_Language;
	bool					m_bMusicEnabled;
	string					m_Difficulty;

	
	void MenusLoop();

	void PlayLoop();
	void IntroLoop();

	//
	// Frame Info
	//
	//FrameInfo& GetXmlFrameinfo(HashLabel Name)  { return m_FrameinfoContainer[Name]; }
	map<HashLabel,FrameInfo>::iterator GetFrameinfoBegin() { return m_FrameinfoContainer.begin(); }
	map<HashLabel,FrameInfo>::iterator GetFrameinfoEnd() { return m_FrameinfoContainer.end(); }
	map<HashLabel,FrameInfo> m_FrameinfoContainer;

	//
	// load file containers
	//
	vector<FileInfo> m_SoundinfoContainer;  // maybe use a <map> if this stuff gets out of hand
	vector<FileInfo> m_FontinfoContainer;
	vector<FileInfo> m_LwoinfoContainer;
	vector<FileInfo> m_RawTgainfoContainer;
	vector<FileInfo> m_DownloadinfoContainer;
	vector<FileInfo> m_MusicFilesContainer;

	//
	// Language support
	//
	map< string, map< string, string > > m_SupportedLanguages;

	map<HashLabel,float> m_VariablesContainer;



};	

#endif
