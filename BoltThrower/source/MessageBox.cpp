#include "MessageBox.h"
#include "WiiManager.h"
#include "FontManager.h"
#include "ImageManager.h"
#include "Image.h"
#include "Util.h"
#include "Util3D.h"
#include "debug.h"
#include "ogc\lwp_watchdog.h"
#include "font.h"
#include "camera.h"
#include "Draw_Util.h"

MessageBox::MessageBox(): m_Message("-"), m_MessageHeading("-"),m_Timer(), m_Enabled(false), m_FadeValue(1.0f), m_FadingOut(false)
{
}

void MessageBox::Init()
{
	m_pWii = Singleton<WiiManager>::GetInstanceByPtr();
}

void MessageBox::FadeOut()
{
	if (!m_pWii->GetMessageBox()->IsFadingOut())
	{
		m_pWii->GetMessageBox()->EnableFadeOut(true);
	}
}

void MessageBox::SetUpMessageBox(std::string Heading, std::string Text)
{
	m_MessageHeading = Heading;
	m_Message = Text; 
	m_Timer = Util::timer_gettime() + secs_to_ticks(30);
	SetEnabled(true);
	EnableFadeOut(false); 
}

bool MessageBox::IsMessageComplete()
{
	return (Util::timer_gettime() >= m_Timer);
}

bool MessageBox::IsReadyForFading()
{
	return (Util::timer_gettime() >= (m_Timer - secs_to_ticks(1)) );
}

void MessageBox::DoMessageBox()
{
	if (IsEnabled())
	{
		if (IsMessageComplete())
		{
			SetEnabled(false);
		}
		if (IsReadyForFading())
		{
			EnableFadeOut(true); // or by a users button press
		}
		DisplayMessageBox();
	}
}

void MessageBox::FadeLogic()
{ 
	m_FadeValue-=0.02f; 
	if (m_FadeValue<0)
	{
		SetEnabled(false);
		m_FadeValue=0; 
	}
}

void MessageBox::DisplayMessageBox(float BoxWidth , float BoxHeight )
{
	m_pWii->GetCamera()->StoreCameraView();
	m_pWii->GetCamera()->SetCameraView(0,0) ;

	float moveoff(0);
	if (IsFadingOut())
	{
		moveoff = ((1.0f-GetFadeValue()) * (BoxHeight*0.5f));  // looks better with smaller movements while fading???
		FadeLogic();
	}

	float RoomAroundEdge = 1.00-0.04f;  // factor i.e 8% off the edge
	std::vector<std::string> MessageContainer = FitTextToBox(m_Message,(BoxWidth*RoomAroundEdge),(BoxHeight*RoomAroundEdge));

	Util3D::Trans(-BoxWidth/2, (-BoxHeight/2) + moveoff);
	Draw_Util::DrawRectangle(0, 0,BoxWidth,BoxHeight,128*GetFadeValue() ,0,0,0);
	
	Util3D::Trans(0, 0 + moveoff);
	float h = m_pWii->GetFontManager()->GetFont(HashString::LargeFont)->GetHeight();
	m_pWii->GetFontManager()->DisplayTextCentre(m_MessageHeading, 0,(h*0.5f)+((-BoxHeight/2)*RoomAroundEdge),255*GetFadeValue());

	static const float gap = 8.0f;
	Util3D::Trans((-BoxWidth/2) * RoomAroundEdge, gap + h+((-BoxHeight/2)*RoomAroundEdge) + moveoff);
	for (std::vector<std::string>::iterator iter(MessageContainer.begin()); iter!=MessageContainer.end(); ++iter)
	{
		m_pWii->GetFontManager()->DisplayText(*iter, 0,std::distance(MessageContainer.begin(),iter)*22,255*GetFadeValue(),HashString::SmallFont);
	}

	Image* pWiiMoteButtonA = m_pWii->GetImageManager()->GetImage(HashString::WiiMoteButtonA);
	pWiiMoteButtonA->DrawImageXYZ(BoxWidth/2 - (pWiiMoteButtonA->GetWidth()*0.5) ,BoxHeight/2 - (pWiiMoteButtonA->GetHeight()*0.5) + moveoff ,0,190*GetFadeValue());

	m_pWii->GetCamera()->RecallCameraView();
}

std::vector<std::string> MessageBox::MessageBox::FitTextToBox(std::string Text,int BoxWidth, int /*BoxHeight*/)
{

	HashLabel FontType(HashString::SmallFont);

	vector<string> tokens; 
	split_string(Text," ",tokens);


	vector<string> FormattedText; 
	string BuildString;
	for (vector<string>::iterator iter(tokens.begin()); iter!=tokens.end(); ++iter)
	{
	//	printf(iter->c_str());

		string Before(BuildString);
		string a = *iter;

		int n;
		while ( ( n = a.find("\\n") ) != std::string::npos )
		{
			string b = a.substr(0,n);
			FormattedText.push_back(BuildString + b);
			n+=2;
			a = a.substr(n,a.length()-n);
			//printf(BuildString.c_str());
			BuildString = "";
		}
	
		BuildString += a;

		if ((m_pWii->GetFontManager()->GetTextWidth(BuildString,FontType) > BoxWidth) )
		{
		//	printf(Before.c_str());
			FormattedText.push_back(Before);
			BuildString = a;
		}
		
		if (!BuildString.empty())
			BuildString += " ";
	}

	if ( BuildString != " " )
		FormattedText.push_back(BuildString);

	return FormattedText;

}

// uses find_first_of so more than one delimitter may be used e.g. " -[]"
void MessageBox::split_string(const string& Text,const string& delimitters,vector<string>& TextContainer)
{
	string::size_type delim(0);
	string::size_type prev_delim(0);

	// Found this nice use of the assignment operator, normally stay clear of things like this but it works well here.
	while ( (delim = Text.find_first_of(delimitters,prev_delim) ) != string::npos) 
	{
		TextContainer.push_back(Text.substr(prev_delim,delim - prev_delim));
		prev_delim = delim + 1;
	}
	TextContainer.push_back(Text.substr(prev_delim)); // add tail
}
