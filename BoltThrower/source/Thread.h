#ifndef Thread_H
#define Thread_H

#include <gccore.h>
#include <string>

struct ThreadData {
	enum ThreadState { LOADING, ONLINEDOWNLOAD_UPDATE, QUESTION, ONLINEDOWNLOAD_EXTRAFILES,UPDATE_COMPLETE_RESET, QUIT} ;
	bool Runnning;
	std::string Message;
	std::string PreviousMessage;

	int WorkingBytesDownloaded;

	ThreadState State;
		
	bool HasStopped;
};


class Thread
{

public:

	ThreadData m_Data;

	static void* ThreadCallingFunc(Thread* ptr);

	void* thread2(ThreadData* ptr);
	void Start(ThreadData::ThreadState eState = ThreadData::LOADING, string CornerMessage="" );
	void Stop();

};

#endif