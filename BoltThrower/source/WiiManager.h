#ifndef WiiManager_H
#define WiiManager_H

#include "Singleton.h"
#include "font.h"
#include "Camera.h"
#include "MapManager.h"
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <gcmodplay.h>
#include "HashLabel.h"
#include "Render3D.h"
#include "tga.h"
#include "GameLogic.h"
#include "ImageManager.h"
#include "HashString.h"

#include "CullFrustum\Vec3.h"
#include "CullFrustum\FrustumR.h"

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
class URLManager;
class UpdateManager;
class SetUpGame;

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



class WiiManager : private SingletonClient
{
public:

	enum EAlign{eRight,eLeft,eCentre};

	
	#define ONE_KILOBYTE (1024)
	#define DEFAULT_GX_FIFO_SIZE ( (4*256) * ONE_KILOBYTE)
	enum EDebugConsole {eDebugConsoleOn, eDebugConsoleOff} ;

	WiiManager();
	~WiiManager();

	void InitWii();
	void UnInitWii();
	void FinaliseInputDevices() const;

	void InitialiseVideo();

	void InitDebugConsole(int ScreenOriginX = 64, int ScreenOriginY = 0);  // was 20,20

	u32* GetCurrentFrame() const;
	u32 GetScreenBufferId() const;
	void SwapScreen(bool bState = true); 

	enum EScreenBuffer { eFrontScreenBuffer,eBackScreenBuffer, eMaxScreenBuffers };
	u32 GetMaxScreenBuffers() const  {return eMaxScreenBuffers;}
	GXRModeObj* GetGXRMode() const	
	{ 
		if  (m_pGXRMode==NULL) exit(1); 
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
	MapManager*			GetMapManager()			{ return m_MapManager; }
	SoundManager*		GetSoundManager()		{ return m_SoundManager; }
	Camera*				GetCamera()	const		{ return m_Camera; }
	GameLogic*			GetGameLogic()	const	{ return m_pGameLogic; }
	MenuScreens*		GetMenuScreens() const	{ return m_pMenuScreens; }
	MessageBox*			GetMessageBox() const	{ return m_MessageBox; }
	GameDisplay*		GetGameDisplay() const	{ return m_pGameDisplay; }
	URLManager*			GetURLManager() const	{ return m_URLManager; }
	UpdateManager*		GetUpdateManager() const	{ return m_UpdateManager; }

	SetUpGame*			GetSetUpGame() const	{ return m_SetUpGame; }
	
	s16 GetViewportX() const { return m_ViewportX; }
	s16 GetViewportY() const { return m_ViewportY; }

	void InitGX(u32 GXFifoBufferSize = DEFAULT_GX_FIFO_SIZE);
	void SetUp2DProjection();
	void SetUp3DProjection();

	void Printf(int x, int y, const char* pFormat, ...);

	void CreateSettingsFromXmlConfiguration(std::string FileName);

	float GetXmlVariable(HashLabel Name) { return m_VariablesContainer[Name]; }
	FrameInfo& GetXmlFrameinfo(HashLabel Name)  { return m_FrameinfoContainer[Name]; }
	map<HashLabel,FrameInfo>::iterator GetFrameinfoBegin() { return m_FrameinfoContainer.begin(); }
	map<HashLabel,FrameInfo>::iterator GetFrameinfoEnd() { return m_FrameinfoContainer.end(); }
	
	//sounds
	vector<FileInfo>::iterator GetSoundinfoBegin() { return m_SoundinfoContainer.begin(); }
	vector<FileInfo>::iterator GetSoundinfoEnd() { return m_SoundinfoContainer.end(); }
	//fonts
	vector<FileInfo>::iterator GetFontInfoBegin() { return m_FontinfoContainer.begin(); }
	vector<FileInfo>::iterator GetFontInfoEnd() { return m_FontinfoContainer.end(); }
	//LWO's
	vector<FileInfo>::iterator GetLwoInfoBegin() { return m_LwoinfoContainer.begin(); }
	vector<FileInfo>::iterator GetLwoInfoEnd() { return m_LwoinfoContainer.end(); }
	//////Mod's
	////vector<FileInfo>::iterator GetModInfoBegin() { return m_ModinfoContainer.begin(); }
	////vector<FileInfo>::iterator GetModInfoEnd() { return m_ModinfoContainer.end(); }
	//RawTga's
	vector<FileInfo>::iterator GetRawTgaInfoBegin() { return m_RawTgainfoContainer.begin(); }
	vector<FileInfo>::iterator GetRawTgaInfoEnd()   { return m_RawTgainfoContainer.end(); }
	//Ogg's
	vector<FileInfo>::iterator GetDownloadInfoBegin() { return m_DownloadinfoContainer.begin(); }
	vector<FileInfo>::iterator GetDownloadInfoEnd() { return m_DownloadinfoContainer.end(); }
//	int GetSizeOfDownloadInfoContainer();

	//Image*					m_pSpaceBackground;
	struct RawTgaInfo
	{
		Tga::PIXEL*						m_pTinyLogo;
		Tga::TGA_HEADER			m_pTinyLogoHeader;
	};

	map<HashLabel,RawTgaInfo> m_RawTgaInfoContainer;

	void DrawRectangle(f32 xpos, f32 ypos, f32 w, f32 h, u8 Alpha, u8 r, u8 g, u8 b );
	void DrawRectangle(f32 xpos, f32 ypos, f32 w, f32 h, u8 Alpha, u8 r, u8 g, u8 b,u8 r2, u8 g2, u8 b2  );

	void TextBox(const std::string& rText, float x, float y, EAlign eAlign);
	void TextBox(const std::string& rText, float x, float y, int w, int h, EAlign eAlign);
	void TextBox(float x, float y, int w, int h, EAlign eAlign, const char*  formatstring, ...);
	void TextBoxWithIcon( float x, float y, int w, int h, EAlign eAlign, HashLabel IconName, const char*  formatstring, ...);

	GXRModeObj* GetBestVideoMode();
	MenuManager* GetMenuManager()			{ return m_pMenuManager; }
	MissionManager* GetMissionManager()		{return  m_MissionManager; }

	Render3D	Render;  // MOVE THIS!!!


	enum EGameState{
		eIntro,
		eMenu,
		eCredits,
		eGame,
		eDemoMode,
		eHighScore,
		eControls,
		eOptions,
		eExit,
	};
	void		SetGameState(EGameState State)	{ m_GameState = State; }
	EGameState	GetGameState(void)				{ return m_GameState; }
	bool	IsGameStateIntro()					{ return m_GameState==eIntro; }
	bool	IsGameStateMenu()					{ return m_GameState==eMenu; }
	bool	IsGameStateCredits()				{ return m_GameState==eCredits; }
	bool	IsGameStateGame()					{ return m_GameState==eGame; }
	bool	IsGameStateControls()				{ return m_GameState==eControls; }
	bool	IsGameStateOptions()				{ return m_GameState==eOptions; }
	bool	IsGameStateExit()					{ return m_GameState==eExit; }
	void	InitGameResources();

	std::map<HashLabel,FrameStartEnd> m_FrameEndStartConstainer;  
	FileMemInfo* m_pMusicData;  // holds things like mods & oggs

	Image* GetSpaceBackground() { return GetImageManager()->GetImage(HashString::SpaceBackground01); }
	Image* GetTinyLogo() { return GetImageManager()->GetImage(HashString::TinyLogo); }

	//-----------------------------------------------------------------
	// Profiler Section
	//static u64 __ticks_to_us(u64 ticks);  // Microseconds (us)
	//static u64 __ticks_to_cycles(u64 ticks); //Clock cycles taken
	void profiler_create(profiler_t* pjob, std::string name);
	string profiler_output(profiler_t* pjob);
	void profiler_start(profiler_t* pjob);
	void profiler_stop(profiler_t* pjob);
	void profiler_reset(profiler_t* pjob);
	//------------------------------------------------------------------

	void SetFrustumView(int w, int h);
	FrustumR m_Frustum;

	void BuildMenus(bool KeepSettings = false);

//	void DisplaySimpleMessage(std::string Text);
	
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
	void ScanMusicFolder();
	void NextMusic();
	void PlayMusic();
	void SetMusicVolume(int Volume);
	FileInfo* GetCurrentMusicInfo();

	string GetNameOfCurrentMusic();

	void GetFolderFileNames(string Path, vector<FileInfo>* rMusicFilesContainer);

	vector<FileInfo> m_MusicFilesContainer;

	string	m_ExePath;

private:
	
	void LoadMusic();

	void SetViewport(s16 x, s16 y) { m_ViewportX = x;m_ViewportY=y; }

	u32* 					m_pFrameBuffer[2];
	GXRModeObj*				m_pGXRMode;
	u32*					m_gp_fifo;
	u32						m_uScreenBufferId;
	u32						m_uFrameCounter;
	ImageManager*			m_ImageManager;
	FontManager*			m_FontManager;
	InputDeviceManager*		m_InputDeviceManager;
	MapManager*				m_MapManager;
	SoundManager*			m_SoundManager;
	Camera*					m_Camera;
	URLManager*				m_URLManager;
	UpdateManager*			m_UpdateManager;
	SetUpGame*				m_SetUpGame;
	MenuManager*			m_pMenuManager;
	MissionManager*			m_MissionManager;
	GameLogic*				m_pGameLogic;
	GameDisplay*			m_pGameDisplay;
	MenuScreens*			m_pMenuScreens;
	MessageBox*				m_MessageBox;
	s16						m_ViewportX;
	s16						m_ViewportY;
	EGameState				m_GameState;
	string					m_Language;
	bool					m_bMusicEnabled;
	string					m_Difficulty;
public:
	u8						m_IngameMusicVolume;
	bool					m_MusicStillLeftToDownLoad;
private:

	map<HashLabel,FrameInfo> m_FrameinfoContainer;
	vector<FileInfo> m_SoundinfoContainer;
	// maybe use a <map> if this stuff gets out of hand
	vector<FileInfo> m_FontinfoContainer;
	vector<FileInfo> m_LwoinfoContainer;
	//vector<FileInfo> m_ModinfoContainer;
	vector<FileInfo> m_RawTgainfoContainer;
	vector<FileInfo> m_DownloadinfoContainer;

	map< string, map< string, string > > m_SupportedLanguages;

	map<HashLabel,float> m_VariablesContainer;


	
};	

#endif
