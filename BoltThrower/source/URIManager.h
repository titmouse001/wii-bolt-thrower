#ifndef URIManager_H_
#define URIManager_H_

#include "GCTypes.h"
#include <string>

using namespace std;

class MemoryInfo; // todo...need to move/work this into utils

class URLManager
{
public:

	URLManager();
	~URLManager();

	MemoryInfo* GetFromURI(string URI);
	void SaveURI(string URI, string DestinationPath = "c:\\" );

private:
	string	CreateHttpRequest(const string& CommandWithSpace, const string& Url);
	string	GetHostNameFromUrl(const string& Url, const string& Match = "http://" );

	int		GetValueFromHeaderLabel(string WorkingString, string Label);
	string  GetStringFromHeaderLabel(string WorkingString, string Label);
};

class MemoryInfo
{
public:

	MemoryInfo() : m_pData(NULL), m_uSize(0) {;}

	void SetData(u8* pData) { m_pData = pData; }
	void SetSize(u32 uSize) { m_uSize = uSize; }

	u8* GetData() const { return m_pData; }
	u32 GetSize() const { return m_uSize; }


	void Save(string FullPathFileName);
	void SavePath(string PathName);
	void SetFileNameWithExtension(string Name) { m_FileNameWithExtension = Name; }
	string urlDecode(string Text);

	~MemoryInfo() { free(m_pData); m_pData=NULL; m_uSize=0; m_FileNameWithExtension.clear(); }

private:
	u8*	m_pData;
	u32	m_uSize;
	string	m_FileNameWithExtension;
};




#endif