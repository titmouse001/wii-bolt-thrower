#ifndef SoundManager_H_
#define SoundManager_H_

#include <gccore.h>
#include <vector>
#include <string>
#include <map>
#include "HashLabel.h"

//#include <vorbisfile.h>
#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>

#define USE_AESNDLIB 


#include <aesndlib.h>

typedef void (*AESNDVoiceCallback)(AESNDPB *pb,u32 state);


#include "Oggplayer/OggPlayer.h"


class RawSample
{
public:

	AESNDPB* Play(u8 VolumeLeft=0xff, u8 VolumeRight=0xff, bool bLoop = false);
	void PlayFromVoice(u8 VolumeLeft, u8 VolumeRight, bool bLoop, AESNDPB* VoiceData );


	void SetRawData(u8* pData) {m_RawData = pData;}
	void SetRawDataLength(u32 Data) {m_RawDataLength = Data;}
	void SetVoiceFormat(u8 Data) {m_VoiceFormat = Data;}
	void SetSampleRate(u32 Data) {m_SampleRate = Data;}
	void SetBitsPerSample(u32 Data) { m_BitsPerSample = Data; }
	u8* m_RawData;
	u32	m_RawDataLength;
private:
//	u8* m_RawData; put back
//	u32	m_RawDataLength;
	u8  m_VoiceFormat; // uses SND_SetVoice format as defined in 'asndlib.h'
	u32 m_SampleRate;		// pitch frequency (in Hz)
	u32 m_BitsPerSample ;	// 8bits 16bits
};


class SoundManager
{
public:

	SoundManager();
	~SoundManager();
		
	void LoadSound( std::string FullFileNameWithPath, std::string LookUpName );  // wav, ogg

	std::map<HashLabel,RawSample*> m_SoundContainer;
	RawSample*	GetSound(HashLabel SoundName) { return m_SoundContainer[SoundName]; }

	AESNDPB*	PlaySound(HashLabel SoundName,u8 VolumeLeft = 255, u8 VolumeRight = 255, bool bLoop = false);
	void		PlaySoundFromVoice(HashLabel SoundName, u8 VolumeLeft, u8 VolumeRight, bool bLoop, AESNDPB* VoiceData);
	void		StopSound(AESNDPB* Chan);

	void PlayRandomExplodeSound();
	void PlayRandomBigExplodeSound();
	void PlayRandomHullBang();
	void PlayRandomGunFire(u8 VolumeFactor);
	void PlayRandomHull();
	void PlayRandomHullCreak();

	OggPlayer  m_OggPlayer;


	AESNDPB*  m_FixSoundVoice;
	//AESNDPB*  m_FixMusicVoice;

		void Init();
	void UnInit();


private:

	void StoreSoundFromOgg(std::string FullFileNameWithPath,std::string LookUpName);
	void StoreSoundFromWav( std::string FullFileNameWithPath, std::string LookUpName );

	u32 GetOggTotal(OggVorbis_File* vf);

	//A WAV header consits of several chunks:
	struct RIFFChunk
	{
		char RIFF[4];		// "RIFF"  - big-endian form
		u32 NextChunkSize;	// 36 + SubChunk2Size, or more precisely:
                            //  4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
                            // This is the size of the rest of the chunk 
                            // following this number.  This is the size of the 
                            // entire file in bytes minus 8 bytes for the
                            // two fields not included in this count:
                            // ChunkID and ChunkSize.
		char RIFFType[4];	// Contains the letters "WAVE"  - big-endian form
	};

	struct fmtChunk
	{
		char  fmt[4];		// "fmt "  - big-endian form
		u32  fmtLength;		// 16 for PCM.  This is the size of the rest of the Subchunk which follows this number.
		u16 WaveType;		//PCM = 1 (i.e. Linear quantization) Values other than 1 indicate some form of compression.
		
		//format 
		//#define  VOICE_MONO_16BIT   1 
		//#define  VOICE_MONO_8BIT   0 
		//#define  VOICE_STEREO_16BIT   3 
		//#define  VOICE_STEREO_8BIT   2 
		//
		u16 Channels;		//Mono 1, Stereo 2

		u32 SampleRate;		//8000, 44100, etc.
		u32 BytesPerSecond; // == SampleRate * NumChannels * BitsPerSample/8
		u16 BlockAlignment; // == NumChannels * BitsPerSample/8   The number of bytes for one sample including
                            // all channels. I wonder what happens when this number isn't an integer?
		u16 BitResolution;  // bits Per Sample 8 bits, 16 bits
	};

	struct dataChunk
	{
		char data[4];    // "data"  -  big-endian form
		u32 dataLength;  // sound data length
	};

};


#endif