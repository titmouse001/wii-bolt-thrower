#ifndef GameDisplay_H
#define GameDisplay_H

#include <string>

#include "HashLabel.h"

class ImageManager;
class WiiManager;
class GameLogic;
class FontManager;
struct ThreadData;

class GameDisplay
{

public:

	GameDisplay();
	void Init();

	void DebugInformation();

	void DisplayAllForIngame();

//	void DisplaySimpleMessage(std::string Text, float fAngle = (-3.14f/12.0f));
	
	void DisplayLoadingTuneMessageForThread(ThreadData* pData);
	void DisplaySmallSimpleMessageForThread(ThreadData* pData);

	void DisplayMoon(HashLabel ModelName);
	void DisplayMoonShieldAndRocks();
	void Display3DInfoBar(float x , float y, std::string Message, float Tilt = 0.0f);
	void DisplayShotForGunTurret();
	void DisplayProbMines();
	void DisplayMissile();
	void DisplayExhaust();
	void DisplayExplosions();
	void DisplayBadShips();
	void DisplayProjectile();
	void DisplayGunTurrets();


	WiiManager*			m_pWii;
	GameLogic*			m_pGameLogic;
	ImageManager*		m_pImageManager;
	FontManager*		m_pFontManager;

private:
	void DisplayPlayer();
	void DisplayPickUps();
	void DisplayHealthPickUps();
	void DisplayShieldGenerators();
	void DisplaySkull();
	void DisplayRadar();
	void DisplaySporeThings(bool bDrawInBackground);
	void DisplayEnemySatellite();
	void DisplayAsteroids();
	void DisplayGunShips();
	void DisplayScorePing();

	
	void Printf(int x, int y, const char* pFormat, ...);

};


#endif
