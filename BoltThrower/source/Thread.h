#ifndef Thread_H
#define Thread_H

#include <gccore.h>
#include <string>

struct ThreadData {
	enum ThreadState { LOADING,LOADING_TUNE, ONLINEDOWNLOAD_UPDATE,CHECKING_FOR_UPDATE, QUESTION, ONLINEDOWNLOAD_EXTRAFILES,UPDATE_COMPLETE_RESET, QUIT} ;
	bool Runnning;
	std::string Message;
	std::string PreviousMessage;

	int WorkingBytesDownloaded;

	ThreadState State;
		
	bool HasStopped;

	// hacks for tune loading message
	float FinalYposForLoadingTune;
	float YposForLoadingTune;
};


class Thread
{

public:

	ThreadData m_Data;

	static void* ThreadCallingFunc(Thread* ptr);

	void* thread2(ThreadData* ptr);
	void Start(ThreadData::ThreadState eState = ThreadData::LOADING, string Message="" );
	void Stop();

};

#endif