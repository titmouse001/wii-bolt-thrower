#ifndef MessageBox_H
#define MessageBox_H

#include "GCTypes.h"
#include <string>
#include <vector>

class WiiManager;

using namespace std;

class MessageBox
{
public:
	MessageBox();
	void Init();
	void SetUpMessageBox(std::string Heading, std::string Text);
	void DisplayMessageBox();
	void DoMessageBox();
	void SetEnabled(bool State) { m_Enabled = State; }
	bool IsEnabled() { return m_Enabled; }
	bool IsReadyForFading();
	void EnableFadeOut(bool Status) { m_FadingOut = Status; m_FadeValue = 1.0f; }
	bool IsFadingOut() { return m_FadingOut; }
	void FadeOut();
private:
	vector<string> FitTextToBox(std::string Text,int BoxWidth, int BoxHeight);
	void split_string(const string& Text,const string& delimitter,vector<string>& TextContainer);
	bool IsMessageComplete();
	void FadeLogic();
	float GetFadeValue() { return m_FadeValue; }
	string m_Message;
	string m_MessageHeading;
	u64 m_Timer;
	bool m_Enabled;
	float m_FadeValue;
	bool m_FadingOut;

	WiiManager* m_pWii;
};


#endif