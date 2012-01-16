#include <aesndlib.h>
#include <gccore.h>
//#include <unistd.h>
#include <string.h>
#include "../Util.h"
#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>
#include "oggplayer.h"
#include "../Singleton.h"
#include "../Wiimanager.h"
#include "../DEBUG.h"

//class OggPlayerInfo
//{
//public:
//	OggPlayerInfo*	SetMem(u8* Value)	{ m_Mem = Value; return this; }
//	u8*				GetMem()  const		{ return m_Mem; }
//	OggPlayerInfo*	SetSize(int Value)	{ m_Size = Value; return this; }
//	int				GetSize() const		{ return m_Size; } 
//	OggPlayerInfo*	SetPos(int Value)	{ m_Pos = Value; return this; }
//	int				GetPos()  const		{ return m_Pos; }
//	void			AddPos(u32 Value)	{ m_Pos += Value; }
//	bool			Empty() const		{ return (m_Size==0); }
//	void			Reset()				{ m_Mem=NULL, m_Size=0, m_Pos=0; } 
//private:
//	u8*				m_Mem;
//	int				m_Size; 
//	int				m_Pos;
//};
//OggPlayerInfo OggFile;

static int CallBackRead(void* Ptr, size_t Size, size_t Count, void *Stream)
{
	// This buffer holds the OggVorbis data that needs decoding
	OggPlayerInfo* pBuffer( (OggPlayerInfo*) Stream );
	
	if (pBuffer->Empty())
		return -1;

	int RequestedBytes( Size * Count );
	if (RequestedBytes <= 0) // can't drop through, still have cases negitive or div by zero
		return 0;

	int AmountLeft( RequestedBytes );
	while (AmountLeft > 0)  // not used very often, but this loop is needed
	{
		int AmountOfBytesToCopy( AmountLeft );
		if (AmountOfBytesToCopy > BUFFER_SIZE)
			AmountOfBytesToCopy = BUFFER_SIZE;

		if ((pBuffer->GetPos() + AmountOfBytesToCopy) > pBuffer->GetSize())
		{
			AmountOfBytesToCopy = pBuffer->GetSize() - pBuffer->GetPos();
		}

		if (AmountOfBytesToCopy > 0)
		{
			// fill the memory given to us by the callback
			memcpy(Ptr, pBuffer->GetMem() + pBuffer->GetPos(), AmountOfBytesToCopy);
			pBuffer->AddPos( AmountOfBytesToCopy );
		}
		else
		{
			break;  // empty or negative
		}
		AmountLeft -= AmountOfBytesToCopy;
	}

	// The total number of elements successfully read 
	return (RequestedBytes - AmountLeft) / Size;
}

// ov_read may use CallBackSeek to to it job from time to time, more so when the stream breaks into smaller parts (non 1024 sizes)
static int CallBackSeek(int *Ptr, ogg_int64_t offset, int mode)
{
	OggPlayerInfo* pBuffer( (OggPlayerInfo*)Ptr );
	if ( pBuffer->Empty() )
		return -1;

	switch ( mode )
	{
	case SEEK_SET:
		pBuffer->SetPos( offset );
		break;
	case SEEK_CUR:
		pBuffer->AddPos( offset );
		break;
	case SEEK_END:
		pBuffer->SetPos( pBuffer->GetSize() + offset );
		break;
	}

	if (pBuffer->GetPos() >= pBuffer->GetSize() )
	{
		pBuffer->SetPos( pBuffer->GetSize() );
		return -1;
	}

	if (pBuffer->GetPos() < 0)
	{
		pBuffer->SetPos( 0 );
		return -1;
	}

	return 0;
}

static int CallBackClose(long *Ptr)
{
	OggPlayerInfo* pBuffer( (OggPlayerInfo*) Ptr ); 
	pBuffer->Reset();
	return 0;
}

static long CallBackTell(int *f)
{
	OggPlayerInfo* pBuffer = (OggPlayerInfo*) f;
	return 	(long)( pBuffer->GetPos() );
}

static ov_callbacks callbacks = 
{
	/* The function prototypes for the callbacks are basically the same as for
	* the stdio functions fread, fseek, fclose, ftell. 
	* The one difference is that the FILE * arguments have been replaced with
	* a void * - this is to be used as a pointer to whatever internal data these
	* functions might need. In the stdio case, it's just a FILE * cast to a void *
	* 
	* If you use other functions, check the docs for these functions and return
	* the right values. For seek_func(), you *MUST* return -1 if the stream is
	* unseekable
	*/
	(size_t (* /*read_func*/ )(void *ptr, size_t /*size*/, size_t /*nmemb*/, void * /*datasource*/)) CallBackRead, // size_t fread ( void * ptr, size_t size, size_t count, FILE * stream );
	(int    (* /*seek_func*/ )(void * /*datasource*/, ogg_int64_t offset, int /*whence*/)) CallBackSeek, // int fseek ( FILE * stream, long int offset, int origin );
	(int    (* /*close_func*/)(void * /*datasource*/)) CallBackClose, // int fclose ( FILE * stream );
	(long   (* /*tell_func*/ )(void * /*datasource*/)) CallBackTell // long int ftell ( FILE * stream );
};

static OggPlayer::OggDataInfo OggData;

static lwpq_t oggplayer_queue = LWP_TQUEUE_NULL;
static lwp_t h_oggplayer = LWP_THREAD_NULL;

OggPlayer::OggPlayer() : m_DedicatedOggVorbisVoice (NULL) 
{
}

void OggPlayer::VoiceCallBackFunction(AESNDPB *pb, u32 state) 
{
	if (state == VOICE_STATE_STREAM)
	{
		if (OggData.pcm_indx > 0)
		{
			// tremer gives signed 16-bit little-endian samples. 
			AESND_SetVoiceBuffer(pb, (void*)OggData.pcmout[OggData.DoubleBufferToggle], BUFFER_SIZE );
			OggData.pcm_indx = 0;
			OggData.DoubleBufferToggle ^= 1;
		}
		if (OggData.ogg_thread_running)
		{
			// Re-fill the buffer - time to kick the sleeping thread into action
			LWP_ThreadSignal(oggplayer_queue); 
		}
	}
}

void* OggPlayer::ogg_player_thread(OggPlayer* ptr)
{
	return ptr->ogg_player_thread2(&OggData);
}

void* OggPlayer::ogg_player_thread2(OggDataInfo* ptr)
{
	ptr->pcm_indx = 0;
	ptr->DoubleBufferToggle = 0;
	LWP_InitQueue(&oggplayer_queue);

	OggData.ogg_thread_running = true;
	while (OggData.ogg_thread_running)
	{
		LWP_ThreadSleep(oggplayer_queue); // wait for next sample to end and trigger this again
		while (ptr->pcm_indx < BUFFER_SIZE)
		{	
			int IgnoreBitStream(0);
			// Tremor outputs signed 16 bit only (i.e x2 paramaters are removed from the tremor version of ov_read)
			//Return Values
			//"OV_HOLE" indicates there was an interruption in the data. (one of: garbage between pages, loss of sync followed by recapture, or a corrupt page) 
			//"OV_EBADLINK" indicates that an invalid stream section was supplied to libvorbisidec, or the requested link is corrupt. 
			//"0" indicates EOF 
			//"n" indicates actual number of bytes read. ov_read() will decode at most one vorbis packet per invocation, so the value returned will generally be less than length. 
			long result	= ov_read(	&ptr->vf, (void *)&ptr->pcmout[ptr->DoubleBufferToggle][ptr->pcm_indx], BUFFER_SIZE - ptr->pcm_indx, &IgnoreBitStream);

			if (result > 0) // successful read
			{
				ptr->pcm_indx += result;  // /sizeof(u16) .. changed to u8 to get rid of the converting between byte word diferences.
			}
			else if (result == 0) // end 
			{
				ov_time_seek(&ptr->vf, 0); // loop again
			}
			else 
			{
				OggData.ogg_thread_running = false;  // error in decoding so just stop everyting
				break;
			}
		}
	}

	ov_clear(&ptr->vf);
//	priv->pcm_indx = 0;
//	LWP_CloseQueue(oggplayer_queue);
//	oggplayer_queue = LWP_TQUEUE_NULL;

	return 0;
}

void OggPlayer::Init()
{ 
	//AESNDPB* Chan = Singleton<WiiManager>::GetInstanceByPtr()->GetSoundManager()->m_FixMusicVoice;
	//AESND_RegisterVoiceCallback(Chan,VoiceCallBackFunction);

	if (m_DedicatedOggVorbisVoice==NULL)
	{
		m_DedicatedOggVorbisVoice = AESND_AllocateFixedVoice(VoiceCallBackFunction);
		//m_DedicatedOggVorbisVoice = AESND_AllocateVoice(VoiceCallBackFunction);
	}
}

void OggPlayer::Play(const void* buffer, s32 len, u8 Volume)
{
	OggFile.SetMem((u8*) buffer)->SetSize( len )->SetPos( 0 );
	if ( OggFile.Empty() )
	{
		ExitPrintf("OggFile.Empty()");
		return;
	}

	// open and initialize an OggVorbis_File structure when using a data source other than a file
	if (ov_open_callbacks( (void*)&OggFile, &OggData.vf, NULL, 0, callbacks) < 0)
	{
		ExitPrintf("ov_open_callbacks failed");
		OggFile.Reset();
		OggData.ogg_thread_running = false;
		return;
	}

	//Init();  // grabs a unused voice
	
	// fill out OggVorbis_File struct with the ogg streams details information
	OggData.vi = ov_info(&OggData.vf, -1); 

//	if ( LWP_CreateThread( &h_oggplayer, (void* (*)(void*))OggPlayer::ogg_player_thread, 
//		&OggData, m_OggPlayerStack, STACKSIZE, 70) != -1 )
	if ( LWP_CreateThread( &h_oggplayer,(void* (*)(void*))OggPlayer::ogg_player_thread,&OggData,NULL,0,70) != -1 )
	{
		if (OggData.vi->channels == 2)
			AESND_SetVoiceFormat(m_DedicatedOggVorbisVoice, VOICE_STEREO16);
		else
			AESND_SetVoiceFormat(m_DedicatedOggVorbisVoice, VOICE_MONO16);

		AESND_SetVoiceFrequency(m_DedicatedOggVorbisVoice, OggData.vi->rate);
		SetVolume(Volume);
		AESND_SetVoiceStream(m_DedicatedOggVorbisVoice, true);
        AESND_SetVoiceStop(m_DedicatedOggVorbisVoice, false); 
    }
}

void OggPlayer::Pause(bool Status)
{
	if (OggData.ogg_thread_running)
	{
		//OggData.m_bPaused = Status;
		if (Status)
		{
			// stop voice from triggering hcallbacks
			AESND_SetVoiceStop(m_DedicatedOggVorbisVoice, true);  
		}
		else
		{
			AESND_SetVoiceStop(m_DedicatedOggVorbisVoice, false);
			//LWP_ThreadSignal(oggplayer_queue);
		}
	}
}

void OggPlayer::Stop()
{
	OggData.ogg_thread_running = false;

	if (m_DedicatedOggVorbisVoice != NULL)
	{
		AESND_SetVoiceStream(m_DedicatedOggVorbisVoice, false);
		AESND_SetVoiceStop(m_DedicatedOggVorbisVoice, true);
	}
	else
	{
		ExitPrintf("Call to InitOggVorbisPlayer needed");
	}

	// stop the thread playback
	if (h_oggplayer != LWP_THREAD_NULL)
	{
		if(oggplayer_queue != LWP_TQUEUE_NULL)
		{
			LWP_ThreadSignal(oggplayer_queue); // give the thread the opportunity to close the queue
		}
		LWP_JoinThread(h_oggplayer, NULL);
		h_oggplayer = LWP_THREAD_NULL;
	}

	if (oggplayer_queue != LWP_TQUEUE_NULL)
	{
		LWP_CloseQueue(oggplayer_queue);
		oggplayer_queue = LWP_TQUEUE_NULL;
	}
}

void OggPlayer::SetVolume(int volume)
{
	OggData.volume = volume;
	AESND_SetVoiceVolume(m_DedicatedOggVorbisVoice, volume, volume);
}

s32 OggPlayer::Tell()
{
	if (OggData.ogg_thread_running)
		return (s32)ov_time_tell(&OggData.vf);
	else
		return -1;
}

void OggPlayer::Seek(s32 time_pos)
{
	if (time_pos >= 0)
		ov_time_seek(&OggData.vf, time_pos);
}
