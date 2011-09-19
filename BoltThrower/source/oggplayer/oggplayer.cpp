#include <asndlib.h>
#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>
#include <gccore.h>
#include <unistd.h>
#include <string.h>

#include "oggplayer.h"

struct OggPlayerInfo
{
	char	*mem;
	int		size;
	int		pos;
};

OggPlayerInfo file;


static int CallBackRead(void * punt, int bytes, int blocks, int *f)
{
	if (bytes * blocks <= 0)
		return 0;

	struct OggPlayerInfo* pBuffer = (OggPlayerInfo*) f;

	if (pBuffer->size == 0)
		return -1;

	blocks = bytes * blocks;
	int CountBlocks = 0;

	while (blocks > 0)
	{
		int b = blocks;
		if (b > 4096)	//clamp
			b = 4096;

		if ((pBuffer->pos + b) > pBuffer->size)
			b = pBuffer->size - pBuffer->pos;

		if (b > 0)
		{
			memcpy(punt, pBuffer->mem + pBuffer->pos, b);
			pBuffer->pos += b;
		}

		if (b <= 0)
		{
			return CountBlocks / bytes;
		}
		CountBlocks += b;
		blocks -= b;
	}
	return CountBlocks / bytes;
}

static int CallBackSeek(int *f, ogg_int64_t offset, int mode)
{
	OggPlayerInfo* pBuffer = (OggPlayerInfo*)f;
	if (pBuffer->size == 0)
		return -1;

	switch (mode&3)
	{
	case SEEK_SET:
		pBuffer->pos = offset;
		break;
	case SEEK_CUR:
		pBuffer->pos += offset;
		break;
	case SEEK_END:
		pBuffer->pos = pBuffer->size + offset;
		break;
	}
	
	if (pBuffer->pos >= pBuffer->size)
	{
		pBuffer->pos = pBuffer->size;
		return -1;
	}

	if (pBuffer->pos < 0)
	{
		pBuffer->pos = 0;
		return -1;
	}

	return 0;
}

static int CallBackClose(int *f)
{
	OggPlayerInfo* pBuffer = (OggPlayerInfo*) f;

	pBuffer->size = 0;
	pBuffer->pos = 0;
	pBuffer->mem = 0;

	return 0;
}

static long CallBackTell(int *f)
{
	OggPlayerInfo* pBuffer = (OggPlayerInfo*) f;
	return 	(long)(pBuffer->pos);
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


#define READ_SAMPLES 4096 // samples that it must read before to send
#define MAX_PCMOUT 4096 // minimum size to read ogg samples
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

	short pcmout[2][READ_SAMPLES + MAX_PCMOUT * 2]; /* take 4k out of the data segment, not the stack */

	int DoubleBufferToggle;
	int pcm_indx;

} private_data_ogg;

static private_data_ogg private_ogg;

#define STACKSIZE		8192

static u8 oggplayer_stack[STACKSIZE];
static lwpq_t oggplayer_queue = LWP_TQUEUE_NULL;
static lwp_t h_oggplayer = LWP_THREAD_NULL;
static int ogg_thread_running = 0;

static void ogg_add_callback_for_SetVoice(int voice)
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

	int bitstream = 0;

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
					MAX_PCMOUT, &bitstream);

				priv->flag &= 192;  //128+64 11000000
				// above might as well read...  priv->flag = 192

				if (ret == 0)
				{
					/* EOF */
					if (priv->ContinuousPlay & 1)
						ov_time_seek(&priv->vf, 0); // repeat
					else
						priv->eof = 1; // stops
				}
				else if (ret < 0)
				{
					/* error in the stream.  Not a problem, just reporting it in
					case we (the app) cares.  In this case, we don't. */
					if (ret != OV_HOLE)
					{
						if (priv->ContinuousPlay & 1)
							ov_time_seek(&priv->vf, 0); // repeat
						else
							priv->eof = 1; // stops
					}
				}
				else
				{
					/* we don't bother dealing with sample rate changes, etc, but
					you'll have to*/
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

	file.mem = (char*) buffer;
	file.size = len;
	file.pos = 0;

	private_ogg.ContinuousPlay = mode;
	private_ogg.eof = 0;
	private_ogg.volume = 127;
	private_ogg.flag = 0;
	private_ogg.seek_time = -1;

	if (time_pos > 0)
		private_ogg.seek_time = time_pos;

	if (ov_open_callbacks((void *) &file, &private_ogg.vf, NULL, 0, callbacks) < 0)
	{
		file.size = 0;
		ogg_thread_running = 0;
		return -1;
	}

	// notes: LWP_PRIO_IDLE 0, LWP_PRIO_HIGHEST 127
	// This code was unsing 80, I found this a little hungry, using 64 instead.
	// Maybe possible to dynamicaly calculate this value

	if (LWP_CreateThread(	&h_oggplayer, 
		(void* (*)(void*))ogg_player_thread,  
		&private_ogg, 
		oggplayer_stack, 
		STACKSIZE,
		4 ) == -1)
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
	int ret;
	if (ogg_thread_running == 0 )
		return -1;
	ret = ((s32) ov_time_tell(&private_ogg.vf));

	return ret;
}

void OggPlayer::SetTimeOgg(s32 time_pos)
{
	if (time_pos >= 0)
		private_ogg.seek_time = time_pos;
}
