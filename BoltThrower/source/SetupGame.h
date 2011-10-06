#ifndef StartGame_H_
#define StartGame_H_

class WiiManager;

class SetUpGame
{

public:

	SetUpGame() { ; }
	void Init();
	void MainLoop();

private:

	void Play();
	void Menus();
	void Intro();

	WiiManager* m_pWii;

};
	

#endif
