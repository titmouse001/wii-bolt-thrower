#include "WiiFile.h"
#include <stdlib.h>
#include "fat.h"
#include "debug.h"
#include "ogcsys.h"
#include "Util.h"
#include "Config.h"

#include "Singleton.h"
#include "WiiManager.h"

#include <sys/dir.h>
#include <dirent.h>
#include <sys/stat.h>

#if (BYTE_ORDER == BIG_ENDIAN)
#define ENDIAN16(Value) (Value = ((Value&0x00ff)<<8) |  ((Value&0xff00)>>8))
#define ENDIAN32(Value) (Value = ((Value&0xff000000)>>24) |  ((Value&0x00ff0000)>>8) | ((Value&0x0000ff00)<<8) |  ((Value&0x000000ff)<<24))
#else
#define ENDIAN16(Value) (Value)
#define ENDIAN32(Value) (Value)
#endif


std::string WiiFile::GetFileExtension(const std::string& FileName) 
{     
	std::string Temp("");

	if(FileName.find_last_of(".") != std::string::npos)        
	{
		Temp = FileName.substr(FileName.find_last_of(".")+1);     
	}

	return Temp; 
} 


std::string WiiFile::GetGameMusicPath()
{
	return GetGamePath()+"Music/";
}

std::string WiiFile::GetGamePath()
{
#ifdef BUILD_FINAL_RELEASE 
	std::string TempGamePath = Singleton<WiiManager>::GetInstanceByRef().m_ExePath; //  Final build - use path relative to the executable
#else 
	std::string TempGamePath = "sd:/apps/BoltThrower/"; // debug only
#endif
//	
//#ifdef BUILD_FINAL_RELEASE 
//	std::string TempGamePath = ""; //  Final build - use path relative to the executable
//#else 
//	std::string TempGamePath = "sd:/apps/BoltThrower/"; // debug only
//#endif

	return TempGamePath;

}

void WiiFile::InitFileSystem()
{
	if (!fatInitDefault()) 
	{
		ExitPrintf("error: calling fatInitDefault (using emulator? UnMount 'sd.raw')\n");
	}
}

int WiiFile::GetFileSize(FILE* pFile) 
{
	int index( fseek(pFile, 0, SEEK_CUR) ); 
    fseek(pFile, 0, SEEK_END);   
    int FileLength( ftell(pFile) );   
    fseek(pFile, index, SEEK_SET);

	return FileLength;
}

FILE* WiiFile::FileOpenForRead(string FileName)
{
	return FileOpenForRead( FileName.c_str() );
}

FILE* WiiFile::FileOpenForRead(const char* const pFileName)
{
//	const char* name; 
//	char* label;
//	fatGetVolumeLabel(name, label);

	FILE* pFile(fopen(pFileName,"rb"));
	if (pFile == NULL)
	{
		printf("file not found '%s'\n",pFileName);
		Util::SleepForMilisec(1000*3);   
		exit(1);
	}
//	else
//	{
//		printf("loading... '%s'\n",pFileName);
//	}

	return pFile;
}

u8* WiiFile::mallocfread(FILE* pFile)
{
	uint FileSize = GetFileSize(pFile);
	u8* pData = (u8*)malloc(FileSize );
	size_t result = fread (pData,1,FileSize,pFile);
	if (result != FileSize) 
		ExitPrintf ("Reading error"); 
//	else
//		fclose(pFile);

	return pData;
}

u8* WiiFile::ReadFile(string FileName)
{
	u8* pData(NULL);
	FILE* pFile = FileOpenForRead(FileName);
	pData = mallocfread(pFile);
	fclose (pFile);
	return pData;
}

bool WiiFile::CheckFileExist(std::string FileName)
{
	return CheckFileExist(FileName.c_str());
}

// check for missing folders or files
bool WiiFile::CheckFileExist(const char* FileName)
{
	struct stat buffer ;
	if ( stat( FileName, &buffer ) == 0 ) 
		return true;
	else
		return false;

	//////if (FILE * file = fopen(FileName, "r"))
	//////{
	//////	fclose(file);
	//////	return true;
	//////}
	//////return false;
}

u32 WiiFile::ReadInt32(FILE* pFile)
{
	u32 uData;

	if ( fread ( &uData, 1, sizeof(uData), pFile ) != sizeof(uData) )
		exit(1);

	ENDIAN32(uData);
	return uData;
}

u16 WiiFile::ReadInt16(FILE* pFile)
{
	u16 uData;
	if ( fread ( &uData, 1, sizeof(uData), pFile ) != sizeof(uData) )
		exit(1);

	ENDIAN16(uData);
	return uData;
}

u8 WiiFile::ReadInt8(FILE* pFile)
{
	u8 uData;
	if ( fread ( &uData, 1, sizeof(uData), pFile ) != sizeof(uData) )
		exit(1);

	return uData;
}

string WiiFile::ReadString(FILE* pFile)
{
	// 'first byte read' gives the length of string (length given includes null)
	u8 size( WiiFile::ReadInt8(pFile) ); 
	char* str( new char[size] );

	if ( fread ( str, 1, size, pFile ) != size )
		exit(1);

	string Result = str;
	delete [] str;

	return Result;
}

void WiiFile::WriteInt32(s32 Value ,FILE* pFile )
{
	ENDIAN32(Value);

	if ( fwrite ( &Value, sizeof(Value), 1, pFile ) != 1 )
		exit(1);
}

void WiiFile::WriteInt16(s16 Value, FILE* pFile)
{
	ENDIAN16(Value);

	if ( fwrite ( &Value, sizeof(Value), 1, pFile ) != 1 )
		exit(1);
}


string	WiiFile::GetPathFromFullFileName(string FullFileName)
{
	size_t Pos( FullFileName.rfind("/") );
	if (Pos != string::npos)
		return FullFileName.substr(0, Pos + 1);
	else
		return FullFileName;
}

string	WiiFile::GetFileNameWithoutPath(string FullFileName)
{
	size_t Pos( FullFileName.rfind("/") );
	if (Pos != string::npos)
	{
		Pos+=1;
		size_t Length( FullFileName.length() - Pos );
		return FullFileName.substr(Pos, Length)	;
	}
	else
		return FullFileName;
}


void WiiFile::GetFolderFileNames(string Path, vector<FileInfo>* rMusicFilesContainer)
{
	if ( !WiiFile::CheckFileExist(Path) )
		return;

	DIR* pdir( opendir(Path.c_str()) );
	if (pdir != NULL)
	{
		dirent *pent;
		while ( (pent = readdir(pdir)) != NULL ) 
		{
			string FileName( Path + pent->d_name );
			struct stat statbuf;
			if (stat(FileName.c_str(), &statbuf) != 0)
				continue;
			int Size = statbuf.st_size;
			if ( (statbuf.st_mode & S_IFDIR) == 0 )
			{
				FileInfo Data(FileName,GetFileNameWithoutPath(FileName));
				Data.Size = Size;
				rMusicFilesContainer->push_back(Data);
				//printf("%s",FileName.c_str());
			}
		}
	}
	closedir(pdir);
}
