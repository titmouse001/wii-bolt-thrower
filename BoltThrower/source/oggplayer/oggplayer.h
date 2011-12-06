#ifndef OggPlayer_H
#define OggPlayer_H

#include <aesndlib.h>


static const int BUFFER_SIZE (DSP_STREAMBUFFER_SIZE * 4); // the playback buffer must be dividable by this DSP value



class OggPlayerInfo
{
public:
	OggPlayerInfo*	SetMem(u8* Value)	{ m_Mem = Value; return this; }
	u8*				GetMem()  const		{ return m_Mem; }
	OggPlayerInfo*	SetSize(int Value)	{ m_Size = Value; return this; }
	int				GetSize() const		{ return m_Size; } 
	OggPlayerInfo*	SetPos(int Value)	{ m_Pos = Value; return this; }
	int				GetPos()  const		{ return m_Pos; }
	void			AddPos(u32 Value)	{ m_Pos += Value; }
	bool			Empty() const		{ return (m_Size==0); }
	void			Reset()				{ m_Mem=NULL, m_Size=0, m_Pos=0; } 
private:
	u8*				m_Mem;
	int				m_Size; 
	int				m_Pos;
};

class OggPlayer  
{
public:

	struct OggDataInfo
	{
		OggVorbis_File vf;
		vorbis_info *vi;
		u8 pcmout[2][BUFFER_SIZE] ATTRIBUTE_ALIGN(32);
		int DoubleBufferToggle;
		int pcm_indx;
		int volume;
		//int ContinuousPlay; 
		//bool m_bPaused;
		bool ogg_thread_running;
	} ;
	//OggPlayer()  { ogg_thread_running = false;}

	void	Init();
	void	Play(const void* buffer, s32 len, u8 Volume);
	void	Pause(bool Status = true);
	void	Stop();
	void	SetVolume(int volume);
	s32		Tell();
	void	Seek(s32 time_pos);
private:

	static void* ogg_player_thread(OggPlayer* ptr);
	void* ogg_player_thread2(OggDataInfo* ptr);
	static void  VoiceCallBackFunction(AESNDPB *pb, u32 state);

	AESNDPB* m_DedicatedOggVorbisVoice;

	//---------------------------------------
//	#define STACKSIZE (8192)
//	u8 m_OggPlayerStack[STACKSIZE];
	//---------------------------------------


	OggPlayerInfo OggFile;

	//bool ogg_thread_running;

};

#endif
