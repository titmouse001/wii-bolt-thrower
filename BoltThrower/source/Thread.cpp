#include <gccore.h>
#include <string.h>

#include "Util.h"
#include "Singleton.h"
#include "Wiimanager.h"
#include "DEBUG.h"

#include "thread.h"
#include "GameDisplay.h"

static lwpq_t Thread_queue = LWP_TQUEUE_NULL;
static lwp_t h_ThreadControl = LWP_THREAD_NULL;

void* Thread::ThreadCallingFunc(Thread* ptr)
{
	return ptr->thread2(&(ptr->m_Data));
}

void* Thread::thread2(ThreadData* ptr)
{
	WiiManager& rWiiManager( Singleton<WiiManager>::GetInstanceByRef() );

	LWP_InitQueue(&Thread_queue);

	ptr->HasStopped = false;
	ptr->Runnning = true;
	ptr->Message = "";
	ptr->PreviousMessage = "";

	if (ptr->State == ThreadData::LOADING_TUNE) {

		ptr->YposForLoadingTune = 240+128;   // display origin is from the screen centre for this this part
		ptr->FinalYposForLoadingTune = 0;
		ptr->Message = "Loading Tune";
		for (int i=0; i<60; i++) {
			rWiiManager.GetGameDisplay()->DisplayLoadingTuneMessageForThread(ptr);
			Util::SleepForMilisec(5);   
		}

		while (ptr->Runnning) {
			rWiiManager.GetGameDisplay()->DisplayLoadingTuneMessageForThread(ptr);
			Util::SleepForMilisec(5);   
		}

		ptr->Message = "Now Playing";
		ptr->FinalYposForLoadingTune = 240;
		for (int i=0; i<60; i++) {
			rWiiManager.GetGameDisplay()->DisplayLoadingTuneMessageForThread(ptr);
			Util::SleepForMilisec(5);   
		}
	} else {
		while (ptr->Runnning) {
			rWiiManager.GetGameDisplay()->DisplaySmallSimpleMessageForThread(ptr);
			Util::SleepForMilisec(5);   
		}
	}

	ptr->HasStopped = true;

	return 0;
}
void Thread::Start(ThreadData::ThreadState eState, string Message )
{
	m_Data.State = eState;
	m_Data.Message = Message;
	m_Data.WorkingBytesDownloaded = 0;

	if ( LWP_CreateThread( &h_ThreadControl,
		(void* (*)(void*))Thread::ThreadCallingFunc,
		&m_Data,
		NULL,0,70) != -1 ) {
			// created 
	}
	else {
		// fail
		//todo
	}
}

void Thread::Stop()
{
	if (h_ThreadControl != LWP_THREAD_NULL)
	{

		m_Data.Runnning = false;

		while (!m_Data.HasStopped) {
			Util::SleepForMilisec(1);   
		};

		LWP_JoinThread(h_ThreadControl, NULL);
		h_ThreadControl = LWP_THREAD_NULL;
	 
		if (Thread_queue != LWP_TQUEUE_NULL) {
			LWP_CloseQueue(Thread_queue);
			Thread_queue = LWP_TQUEUE_NULL;
		}
	}
}
