#ifndef Font_H
#define Font_H

#include "GCTypes.h"
#include "CharInfo.h"

#include <vector>
#include <string>
using namespace std;

class Font
{
public:

	Font(const string& FileName);

	const CharInfo*	GetChar (u32 uChar) const;

	void			SetFirstChar(u32 uFirstChar) { m_uFirstChar = uFirstChar; }
	u32				GetFirstChar() const { return m_uFirstChar; }
	void			SetLastChar(u32 uLastChar) { m_uLastChar = uLastChar; }
	u32				GetLastChar() const { return m_uLastChar; }
	void			SetHeight(u16 uHeight) { m_uHeight = uHeight; }
	u16				GetHeight() const { return m_uHeight; }
	void			SetBaseLine(u16 uBaseLine) { m_uBaseLine = uBaseLine; }
	u16				GetBaseLine() const { return m_uBaseLine; }
	void			SetLeading(u16 uLeading) { m_uLeading = uLeading; }
	u16				GetLeading() const { return m_uLeading; }
	void			SetLineSpacing(u16 uLineSpacing) { m_uLineSpacing = uLineSpacing; }
	u16				GetLineSpacing() const { return m_uLineSpacing; }
	void			SetUnderline(s16 iUnderline) { m_iUnderline = iUnderline; }
	s16				GetUnderline() const { return m_iUnderline; }
	void			SetStrikeout(s16 iStrikeout) { m_iStrikeout = iStrikeout; }
	s16				GetStrikeout() const { return m_iStrikeout; }

	void StoreCharInfo(CharInfo& rChar) { m_vCharInfo.push_back(rChar); }

private:

	u32				m_uFirstChar;
	u32				m_uLastChar;
	u16				m_uHeight;
	u16				m_uBaseLine;
	u16				m_uLeading;
	u16				m_uLineSpacing;
	s16				m_iUnderline;
	s16				m_iStrikeout;

	vector<CharInfo>	m_vCharInfo;

	void CreateFontFromFile(const string& FileName);
};


#endif
