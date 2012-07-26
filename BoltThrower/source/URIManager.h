#ifndef URIManager_H_
#define URIManager_H_

#include "GCTypes.h"
#include <string>
#include <stdlib.h>



using namespace std;

class MemoryInfo; // todo...need to move/work this into utils

class URLManager
{
public:

	URLManager();
	~URLManager();

	void Init();
	void UnInit();

	MemoryInfo* GetFromURI(string URI);
	bool	SaveURI(string URI, string DestinationPath = "sd://" );  //""c:\\" );
	bool	m_Initialised;
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
	u8* GetData() const { return m_pData; }

	void SetSize(u32 uSize) { m_uSize = uSize; }
	u32 GetSize() const { return m_uSize; }

	void SetFileNameWithExtension(string Name) { m_FileNameWithExtension = Name; }

	~MemoryInfo();
	void SavePath(string PathName);

	
private:
	

	void Save(string FullPathFileName);

	u8*	m_pData;
	u32	m_uSize;
	string	m_FileNameWithExtension;
};




#endif