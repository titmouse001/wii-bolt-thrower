#ifndef FILE_H_
#define FILE_H_

#include "stdio.h"
#include "GCTypes.h"
#include <string>
using namespace std;

#include <vector>


struct FileMemInfo
{
	u8* pData;
	u32	Size;
};


class FileInfo  // general (used for just about anything!!!)
{
public:
	FileInfo(string InFileName,string InLogicName) : 
			b_ThisSlotIsBeingUsed(false) ,
			FileName(InFileName) , LogicName(InLogicName), m_bNorms(true), m_IndexLayerForBones(-1),
			OverwriteExistingFile(true) {;}

	FileInfo() : b_ThisSlotIsBeingUsed(false) {;}

	bool b_ThisSlotIsBeingUsed;
	string FileName;
	string LogicName;
	string FullDownloadPath;
	bool m_bNorms;
	int m_IndexLayerForBones;
	int Size;
	bool OverwriteExistingFile;
};


namespace WiiFile 
{

	void	InitFileSystem();
	
	string GetGamePath();
	string GetGameMusicPath();

	string GetFileExtension(const string& FileName);

	FILE*	FileOpenForRead(string FileName);
	FILE*	FileOpenForRead(const char* const pFileName);
	bool	CheckFileExist(std::string FileName);
	bool	CheckFileExist(const char* FileName);


	int		GetFileSize(FILE* pFile);

	u8*		mallocfread(FILE* pFile);
	u8*		mallocfread(FILE* pFile, FileMemInfo* Info);

	u8*		ReadFile(string FileName);
	u8*		ReadFile(string FileName,FileMemInfo* Info);

	u32		ReadInt32(FILE* pFile);
	u16		ReadInt16(FILE* pFile);
	u8		ReadInt8(FILE* pFile);
	string	ReadString(FILE* pFile);

	string	GetFileNameWithoutPath(string FullFileName);
	string	GetPathFromFullFileName(string FullFileName);

	
	void	WriteInt32( s32 val,FILE* pFile);
	void	WriteInt16( s16 val,FILE* pFile);
	
	void	GetFolderFileNames(string Path, vector<FileInfo>* rMusicFilesContainer);



};

#endif

