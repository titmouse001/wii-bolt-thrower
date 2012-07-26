#ifndef UpdateManager_H_
#define UpdateManager_H_

#include "GCTypes.h"
#include <string>
#include <vector>
using namespace std;

class WiiManager;

class UpdateManager
{
public:
	UpdateManager();

	void Init();

	void DoUpdate(string MasterUpdateFile);
	bool UpdatePackageYesOrNo();
	bool CheckForUpdate(string MasterUpdateFile);
	void  UpdateApplicationFiles();

	void SetMessageVersionReport(string Value) { m_MessageVersionReport = Value; }
	string GetMessageVersionReport() const { return m_MessageVersionReport; }

	bool DownloadFilesListedInConfiguration(bool MisssingCheckOnly);

	string m_ReleaseNotes;
	string m_LatestReleaseAvailable;

	vector<FileInfo> m_ApplicationSpecificUpdatesForDownloadFileInfoContainer;

private:

	WiiManager* m_pWiiManager;

	string m_MessageVersionReport;
};

#endif
