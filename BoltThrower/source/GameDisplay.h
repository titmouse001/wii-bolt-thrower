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

	void DisplaySimpleMessage(std::string Text);

private:

	void DebugInformation();

	void DisplayPickUps();
	void DisplayShieldGenerators();
	void DisplayGunTurrets();
	void DisplayShotForGunTurret();
	void DisplayMoon(float Dist = 0);
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
