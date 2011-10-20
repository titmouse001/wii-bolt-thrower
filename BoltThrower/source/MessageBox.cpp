#include "MessageBox.h"
#include "WiiManager.h"
#include "FontManager.h"
#include "ImageManager.h"
#include "Image.h"
#include "Util.h"
#include "Util3D.h"
#include "debug.h"
#include "ogc\lwp_watchdog.h"


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

void MessageBox::DisplayMessageBox()
{
	//WiiManager& Wii( Singleton<WiiManager>::GetInstanceByRef() );
	m_pWii->GetCamera()->StoreCameraView();
	m_pWii->GetCamera()->SetCameraView(0,0) ;

	float BoxWidth = 400;
	float BoxHeight = 220;
	float moveoff(0);

	if (IsFadingOut())
	{
		moveoff = ((1.0f-GetFadeValue()) * (BoxHeight*0.5f));  // looks better with smaller movements while fading???
		FadeLogic();
	}

	float RoomAroundEdge = 1.00-0.08f;  // factor i.e 8% off the edge
	std::vector<std::string> MessageContainer = FitTextToBox(m_Message,(BoxWidth*RoomAroundEdge),(BoxHeight*RoomAroundEdge));

	Util3D::Trans(-BoxWidth/2, (-BoxHeight/2) + moveoff);
	m_pWii->DrawRectangle(0, 0,BoxWidth,BoxHeight,128*GetFadeValue() ,0,0,0);
	
	Util3D::Trans(0, 0 + moveoff);
	int h = m_pWii->GetFontManager()->GetFont(HashString::SmallFont)->GetHeight();
	m_pWii->GetFontManager()->DisplayTextCentre(m_MessageHeading, 0,h+((-BoxHeight/2)*RoomAroundEdge),255*GetFadeValue());

	Util3D::Trans((-BoxWidth/2) * RoomAroundEdge, -38 + moveoff);
	for (std::vector<std::string>::iterator iter(MessageContainer.begin()); iter!=MessageContainer.end(); ++iter)
	{
		m_pWii->GetFontManager()->DisplayText(*iter, 0,std::distance(MessageContainer.begin(),iter)*22,255*GetFadeValue(),HashString::SmallFont);
	}

	Image* pWiiMoteButtonA = m_pWii->GetImageManager()->GetImage(HashString::WiiMoteButtonA);
	pWiiMoteButtonA->DrawImageXYZ(BoxWidth/2 - (pWiiMoteButtonA->GetWidth()*0.5) ,BoxHeight/2 - (pWiiMoteButtonA->GetHeight()*0.5) + moveoff ,0,200*GetFadeValue());

	m_pWii->GetCamera()->RecallCameraView();
}

std::vector<std::string> MessageBox::MessageBox::FitTextToBox(std::string Text,int BoxWidth, int /*BoxHeight*/)
{
	HashLabel FontType(HashString::SmallFont);

	vector<string> tokens; 
	split_string(Text," -",tokens);

	vector<string> FormattedText; 
	string BuildString;
	for (vector<string>::iterator iter(tokens.begin()); iter!=tokens.end(); ++iter)
	{
		string Before(BuildString);
		BuildString += *iter;
		if ((m_pWii->GetFontManager()->GetTextWidth(BuildString,FontType) > BoxWidth) || (iter->find("\n")!=std::string::npos) )
		{
			FormattedText.push_back(Before);
			BuildString = *iter;
		}
		BuildString += " ";
	}

	if ( BuildString != "" )
		FormattedText.push_back(BuildString);

	return FormattedText;
}

void MessageBox::split_string(const string& Text,const string& delimitter,vector<string>& TextContainer)
{
	string::size_type delim(0);
	string::size_type prev_delim(0);

	// Found this nice use of the assignment operator, I normally stay clear of things like this but it works well here.
	while ( (delim = Text.find_first_of(delimitter,prev_delim) ) != string::npos) 
	{
		TextContainer.push_back(Text.substr(prev_delim,delim - prev_delim));
		prev_delim = delim + 1;
	}
	TextContainer.push_back(Text.substr(prev_delim)); // add tail
}
