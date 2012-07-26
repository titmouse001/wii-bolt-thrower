#ifndef IntroDisplay_H
#define IntroDisplay_H

#include "GameDisplay.h"

class IntroDisplay : public GameDisplay
{
public:
	void Init();
	void DisplayAllForIntro();
	void DisplayViper();

private:

	float m_fIncrementor;
	float m_fMessagewobble;
};


#endif
