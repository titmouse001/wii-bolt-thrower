#include <asndlib.h>
#include <gccore.h>
#include <unistd.h>
#include <string.h>

#include "../tremor/ivorbiscodec.h"
#include "../tremor/ivorbisfile.h"

#include "oggplayer.h"

// This shows the buffer use amounts - not always a single hit, so loop fill out is needed 
//
//15:23:265 Src\HLE\HLE_OS.cpp:52 N[OSREPORT]: 8006d944->8006d92c| ----
//15:23:265 Src\HLE\HLE_OS.cpp:52 N[OSREPORT]: 8006d92c->8006d910| 1024  <- x1
//15:23:265 Src\HLE\HLE_OS.cpp:52 N[OSREPORT]: 8006d944->8006d92c| ----
//15:23:265 Src\HLE\HLE_OS.cpp:52 N[OSREPORT]: 8006d92c->8006d910| 1024
//15:23:265 Src\HLE\HLE_OS.cpp:52 N[OSREPORT]: 8006d944->8006d92c| ----
//15:23:281 Src\HLE\HLE_OS.cpp:52 N[OSREPORT]: 8006d92c->8006d910| 1024
//15:23:281 Src\HLE\HLE_OS.cpp:52 N[OSREPORT]: 8006d944->8006d92c| ----
//15:23:281 Src\HLE\HLE_OS.cpp:52 N[OSREPORT]: 8006d92c->8006d910| 755   <- x4
//15:23:281 Src\HLE\HLE_OS.cpp:52 N[OSREPORT]: 8006d92c->8006d910| 755
//15:23:281 Src\HLE\HLE_OS.cpp:52 N[OSREPORT]: 8006d92c->8006d910| 754
//15:23:281 Src\HLE\HLE_OS.cpp:52 N[OSREPORT]: 8006d92c->8006d910| 1024
//15:23:281 Src\HLE\HLE_OS.cpp:52 N[OSREPORT]: 8006d944->8006d92c| ----


class OggPlayerInfo
{
public:
	u8*		GetMem()  const { return m_Mem; }
	int		GetSize() const { return m_Size; } 
	int		GetPos()  const { return m_Pos; }
//	void	SetMem(u8* Value)  { m_Mem = Value; }
//	void	SetSize(u32 Value) { m_Size = Value; } 
//	void	SetPos(u32 Value)  { m_Pos = Value; }
	OggPlayerInfo*	SetMem(u8* Value) { m_Mem = Value; return this; }
	OggPlayerInfo*	SetSize(int Value) { m_Size = Value; return this; }
	OggPlayerInfo*	SetPos(int Value)  { m_Pos = Value; return this; }
	void	AddPos(u32 Value)  { m_Pos += Value; }

	bool	Empty() const { return (m_Size==0); }
	void	Reset() { m_Mem=NULL, m_Size=0, m_Pos=0; } 

//private:
	u8*	m_Mem;
	int	m_Size; 
	int	m_Pos;
};

OggPlayerInfo file;

#define READ_SAMPLES 4096	// try to consume 
#define BUFFER_SIZE 4096	// read limit


static int CallBackRead(void* Ptr, int Size, int Count, int *Stream)
{
	OggPlayerInfo* pBuffer( (OggPlayerInfo*) Stream );
	
	if (pBuffer->m_Size == 0)
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

		if ((pBuffer->m_Pos + AmountOfBytesToCopy) > pBuffer->m_Size)
			AmountOfBytesToCopy = pBuffer->m_Size - pBuffer->m_Pos;

		if (AmountOfBytesToCopy > 0)
		{
			memcpy(Ptr, pBuffer->m_Mem + pBuffer->m_Pos, AmountOfBytesToCopy);
			pBuffer->m_Pos += AmountOfBytesToCopy;
		}
		else
		{
			break;  // empty or negative value copy amount left
		}
		AmountLeft -= AmountOfBytesToCopy;
	}

	// The total number of elements successfully read 
	return (RequestedBytes - AmountLeft) / Size;
}

//
//static int CallBackRead(void* Ptr, int Size, int Count, int *Stream)
//{
//	OggPlayerInfo* pBuffer( (OggPlayerInfo*)Stream);
//
//	if ( (Size * Count) <= 0 || pBuffer->Empty() )
//	{
//		return -1;
//	}
//	else
//	{
//		int BytesToCopy( Size * Count );
//		if (BytesToCopy > BUFFER_SIZE)
//			BytesToCopy = BUFFER_SIZE;
//
//		memcpy(Ptr, pBuffer->GetMem() + pBuffer->GetPos(), BytesToCopy);
//		pBuffer->AddPos(BytesToCopy);
//
//		return Count;
//	}
//}

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

static int CallBackClose(int *Ptr)
{
	OggPlayerInfo* pBuffer( (OggPlayerInfo*) Ptr ); 

//	pBuffer->SetSize( 0 );
//	pBuffer->SetPos( 0 );
//	pBuffer->SetMem( 0 );

	pBuffer->Reset();

	return 0;
}

static long CallBackTell(int *f)
{
	OggPlayerInfo* pBuffer = (OggPlayerInfo*) f;
	return 	(long)( pBuffer->GetPos() );
}

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

static ov_callbacks callbacks = 
{
	// size_t fread ( void * ptr, size_t size, size_t count, FILE * stream );
	(size_t (* /*read_func*/)  (void *ptr, size_t /*size*/, size_t /*nmemb*/, void * /*datasource*/)) CallBackRead,

	// int fseek ( FILE * stream, long int offset, int origin );
	(int    (* /*seek_func*/)  (void * /*datasource*/, ogg_int64_t offset, int /*whence*/)) CallBackSeek,

	// int fclose ( FILE * stream );
	(int    (* /*close_func*/) (void * /*datasource*/)) CallBackClose,

	// long int ftell ( FILE * stream );
	(long   (* /*tell_func*/)  (void * /*datasource*/)) CallBackTell
};



typedef struct
{
	OggVorbis_File vf;
	vorbis_info *vi;

	int ContinuousPlay; 
	int eof;
	int flag;
	int volume;
	int seek_time;
	int VoiceFormat; 

	short pcmout[2][READ_SAMPLES + BUFFER_SIZE * 2]; /* take 4k out of the data segment, not the stack */

	int DoubleBufferToggle;
	int pcm_indx;

} private_data_ogg;

static private_data_ogg private_ogg;

#define STACKSIZE		8192

static u8 oggplayer_stack[STACKSIZE];
static lwpq_t oggplayer_queue = LWP_TQUEUE_NULL;
static lwp_t h_oggplayer = LWP_THREAD_NULL;
static int ogg_thread_running = 0;

static void ogg_add_callback_for_SetVoice(int /*voice*/)
{
	if (!ogg_thread_running)
	{
		ASND_StopVoice(0);
		return;
	}

	if (private_ogg.flag & 128)   // why not call StatusOgg????
		return; // Ogg is paused

	if (private_ogg.pcm_indx >= READ_SAMPLES)
	{
		if (ASND_AddVoice(0,
			(void *) private_ogg.pcmout[private_ogg.DoubleBufferToggle],
			private_ogg.pcm_indx << 1) == 0)
		{
			private_ogg.DoubleBufferToggle ^= 1;
			private_ogg.pcm_indx = 0;
			private_ogg.flag = 0;
			LWP_ThreadSignal(oggplayer_queue);
		}
	}
	else
	{
		if (private_ogg.flag & 64)
		{
			private_ogg.flag &= ~64;
			LWP_ThreadSignal(oggplayer_queue);
		}
	}
}

static void* ogg_player_thread(private_data_ogg* priv)
{
	int first_time = 1;
	long ret;

	// thread synchronization queue
	LWP_InitQueue(&oggplayer_queue);

	// fill out OggVorbis_File struct with the ogg streams details information
	priv->vi = ov_info(&priv->vf, -1); 

	ASND_Pause(0);

	// Can oggs support more complex playback then this can support?
	// Anyway for this player we don't care, avoid streams that change voice format on the fly (if that's even possible?).
	if (priv->vi->channels == 2)
		priv->VoiceFormat =  VOICE_STEREO_16BIT;
	else
		priv->VoiceFormat =  VOICE_MONO_16BIT;

	priv->pcm_indx = 0;
	priv->DoubleBufferToggle = 0;
	priv->eof = 0;
	priv->flag = 0;
	ogg_thread_running = 1;
	int bitstream(0);

	while (!priv->eof && ogg_thread_running)
	{
		if (priv->flag)
			LWP_ThreadSleep(oggplayer_queue); // wait only when i have samples to send

		if (priv->flag == 0) // wait to all samples are sent
		{
			if (ASND_TestPointer(0, priv->pcmout[priv->DoubleBufferToggle])
				&& ASND_StatusVoice(0) != SND_UNUSED)
			{
				priv->flag |= 64;
				continue;
			}
			if (priv->pcm_indx < READ_SAMPLES)
			{
				priv->flag = 3; // 3 00000011  ?? this just gets masked out later on????

				if (priv->seek_time >= 0)
				{
					ov_time_seek(&priv->vf, priv->seek_time);
					priv->seek_time = -1;
				}

				ret	= ov_read( &priv->vf,
					(void *) &priv->pcmout[priv->DoubleBufferToggle][priv->pcm_indx],
					BUFFER_SIZE, &bitstream);

				priv->flag &= 192;  //128+64 11000000
				// above might as well read...  priv->flag = 192

				if (ret == 0)
				{
					// end of file
					if (priv->ContinuousPlay & 1)
						ov_time_seek(&priv->vf, 0); // repeat
					else
						priv->eof = 1; // stops
				}
				else if (ret < 0)
				{
					if (ret != OV_HOLE) // error in the stream.
					{
						if (priv->ContinuousPlay & 1)
							ov_time_seek(&priv->vf, 0); // repeat
						else
							priv->eof = 1; // stops
					}
				}
				else
				{
					// with sample rate changes, etc ... may not be very future proof just here!
					priv->pcm_indx += ret >> 1; //get 16 bits samples
				}
			}
			else
				priv->flag = 1;
		}

		if (priv->flag == 1)
		{
			if (ASND_StatusVoice(0) == SND_UNUSED || first_time)
			{
				first_time = 0;

				ASND_SetVoice(0, priv->VoiceFormat, priv->vi->rate, 0,
					(void *) priv->pcmout[priv->DoubleBufferToggle],
					priv->pcm_indx << 1, priv->volume,
					priv->volume, ogg_add_callback_for_SetVoice);

				priv->DoubleBufferToggle ^= 1;
				priv->pcm_indx = 0;
				priv->flag = 0;
			}
		}

		// need to look into this sleep value of 100 , looks like this could be increased
		// using double buffer so thinking this should be calculated for the playback time of a buffer with add little added for delays
		usleep(100);  
	}
	ov_clear(&priv->vf);
	priv->pcm_indx = 0;

	return 0;
}

void OggPlayer::StopOgg()
{
	ASND_StopVoice(0);
	ogg_thread_running = 0;

	if(h_oggplayer != LWP_THREAD_NULL)
	{
		if(oggplayer_queue != LWP_TQUEUE_NULL)
			LWP_ThreadSignal(oggplayer_queue);
		LWP_JoinThread(h_oggplayer, NULL);
		h_oggplayer = LWP_THREAD_NULL;
	}
	if(oggplayer_queue != LWP_TQUEUE_NULL)
	{
		LWP_CloseQueue(oggplayer_queue);
		oggplayer_queue = LWP_TQUEUE_NULL;
	}
}

int OggPlayer::PlayOgg(const void *buffer, s32 len, int time_pos, int mode)
{
	StopOgg();

	file.SetMem((u8*) buffer)->SetSize( len )->SetPos( 0 );
//	file.SetSize( len );
//	file.SetPos( 0 );

	if ( file.Empty() )
		return -1;

	private_ogg.ContinuousPlay = mode;
	private_ogg.eof = 0;
	private_ogg.volume = 127;
	private_ogg.flag = 0;
	private_ogg.seek_time = -1;

	if (time_pos > 0)
		private_ogg.seek_time = time_pos;

	// The ov_callbacks structure contains file manipulation function prototypes necessary for opening, closing, seeking, and location. 
	// The header vorbis/vorbisfile.h provides several predefined static ov_callbacks structures that may be passed to ov_open_callbacks(): 
	if (ov_open_callbacks((void *) &file, &private_ogg.vf, NULL, 0, callbacks) < 0)
	{
		//file.SetSize( 0 );
		file.Reset();
		ogg_thread_running = 0;
		return -1;
	}

	// notes: LWP_PRIO_IDLE 0, LWP_PRIO_HIGHEST 127
	// This code was unsing 80, I found this a little hungry, using 64 instead.
	// Maybe possible to dynamicaly calculate this value

	if (LWP_CreateThread( 
		&h_oggplayer,(void*(*)(void*))ogg_player_thread, 
		&private_ogg, oggplayer_stack, STACKSIZE, 64 ) == -1)
	{
		ogg_thread_running = 0;
		ov_clear(&private_ogg.vf);
		return -1;
	}
	return 0;
}

void OggPlayer::PauseOgg(int pause)
{
	if (pause)
	{
		private_ogg.flag |= 128;
	}
	else
	{
		if (private_ogg.flag & 128)
		{
			private_ogg.flag |= 64;
			private_ogg.flag &= ~128;
			if (ogg_thread_running > 0)
			{
				LWP_ThreadSignal(oggplayer_queue);
			}
		}
	}
}

int OggPlayer::StatusOgg()
{
	if (ogg_thread_running == 0)
		return -1; // Error
	else if (private_ogg.eof)
		return 255; // EOF
	else if (private_ogg.flag & 128)
		return 2; // paused
	else
		return 1; // running
}

void OggPlayer::SetVolumeOgg(int volume)
{
	private_ogg.volume = volume;
	ASND_ChangeVolumeVoice(0, volume, volume);
}

s32 OggPlayer::GetTimeOgg()
{
	if (ogg_thread_running == 0 )
		return -1;
	else
		return (s32)ov_time_tell(&private_ogg.vf);
}

void OggPlayer::SetTimeOgg(s32 time_pos)
{
	if (time_pos >= 0)
		private_ogg.seek_time = time_pos;
}
