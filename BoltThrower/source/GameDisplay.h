#ifndef GameDisplay_H
#define GameDisplay_H

#include <string>
class ImageManager;
class WiiManager;
class GameLogic;

class GameDisplay
{

public:

	GameDisplay();
	void Init();

	void DisplayAllForIngame();
	void DisplayAllForIntro();

	void DisplaySimpleMessage(std::string Text, float fAngle = (-3.14f/12.0f));
	void DisplaySmallSimpleMessage(std::string Text);

	void DisplayMoon();

private:

	void DebugInformation();

	void DisplayPickUps();
	void DisplayShieldGenerators();
	void DisplayGunTurrets();
	void DisplayShotForGunTurret();
	//void DisplayMoon();
	void DisplayInformationPanels();
	void DisplaySkull();
	void DisplayRadar();
	void DisplaySporeThings();
	void DisplayAsteroids();
	void DisplayProbMines();
	void DisplayProjectile();
	void DisplayMissile();
	void DisplayExhaust();
	void DisplayExplosions();
	void DisplayGunShips();
	void DisplayBadShips();

	WiiManager*			m_pWii;
	GameLogic*			m_pGameLogic;
	ImageManager*		m_pImageManager;
};


#endif
