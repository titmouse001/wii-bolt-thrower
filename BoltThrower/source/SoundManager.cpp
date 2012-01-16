// Manager: Holds Oggs and Wavs from file in memory

#include "SoundManager.h"
#include "malloc.h"
#include "debug.h"
#include "string.h"
#include "config.h"
#include "WiiManager.h"
#include "WiiFile.h"
#include "Util.h"
#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>
#include <grrmod.h>  // for sound init - since it likes to take over the whole shop

//static void VoiceCallBack(AESNDPB *pb, u32 state) 
//{
//   switch (state) 
//   {
//        case VOICE_STATE_STOPPED:
//	//		printf("VoiceCallBack %p %d",pb,state);  // OK!?!  this AESNDPB is pointing to the main sound call back... not for each voice as I was expecting
//            break;
//        case VOICE_STATE_RUNNING:
//            break;
//        case VOICE_STATE_STREAM:
//            break;
//    }
//}

AESNDPB* RawSample::Play(u8 VolumeLeft, u8 VolumeRight, bool bLoop)
{
	// change library ... AESNDPB* AESND_AllocateVoice(AESNDVoiceCallback cb) to use VOICE_STOPPED not VOICE_USED to pick free slot

	AESNDPB* Chan = AESND_AllocateNextFreeVoice(NULL); 
	//AESNDPB* Chan = AESND_AllocateVoice(NULL); 

	if (Chan!=NULL)
	{		

		//DCFlushRange(m_RawData, m_RawDataLength);

		AESND_SetVoiceFormat(Chan, m_VoiceFormat);
		AESND_SetVoiceFrequency(Chan, m_SampleRate);
		AESND_SetVoiceVolume(Chan,VolumeLeft,VolumeRight);
    AESND_SetVoiceStream(Chan, false);
		//AESND_SetVoiceBuffer(Chan, (void*)m_RawData, m_RawDataLength);


		AESND_PlayVoice(Chan,
			m_VoiceFormat,
			(void*)m_RawData,
			m_RawDataLength,m_SampleRate,
			0,bLoop);

	}
	return Chan;
}	

void RawSample::PlayFromVoice(u8 VolumeLeft, u8 VolumeRight, bool bLoop, AESNDPB* VoiceData )
{
	if (VoiceData!=NULL)
	{
		//DCFlushRange(m_RawData, m_RawDataLength);


		AESND_SetVoiceVolume(VoiceData,VolumeLeft,VolumeRight);
		AESND_PlayVoice(VoiceData,
			m_VoiceFormat,
			(void*)m_RawData,
			m_RawDataLength,m_SampleRate,
			0,bLoop);
	}
}	

//--------------------

void SoundManager::PlayRandomExplodeSound()
{
	int VolumeFactor = 200 - (rand()%60);
	switch (rand()%3)
	{
		case 0:
			PlaySound( HashString::Explode1, VolumeFactor, VolumeFactor );
		break;
		case 1:
			PlaySound( HashString::Explode2, VolumeFactor, VolumeFactor );
		break;
		case 2:
			PlaySound( HashString::Explode3, VolumeFactor, VolumeFactor );
		break;
	}
}

void SoundManager::PlayRandomBigExplodeSound()
{
	int VolumeFactor = 255 - (rand()%30);
	switch (rand()%2)
	{
		case 0:
			PlaySound( HashString::ExplodeBig1, VolumeFactor, VolumeFactor );
		break;
		case 1:
			PlaySound( HashString::ExplodeBig2, VolumeFactor, VolumeFactor );
		break;
	}
}

void SoundManager::PlayRandomHullBang()
{
	int VolumeFactor = 255 - (((rand()%128)/2)*2);
	switch (rand()%4)
	{
		case 0:
			PlaySound( HashString::Hull_bang1, VolumeFactor, VolumeFactor );
		break;
		case 1:
			PlaySound( HashString::Hull_bang2, VolumeFactor, VolumeFactor );
		break;
		case 2:
			PlaySound( HashString::Hull_bang3, VolumeFactor, VolumeFactor );
		break;
		case 3:
			PlaySound( HashString::Hull_bang4, VolumeFactor, VolumeFactor );
		break;
	}
}
//---------------------


AESNDPB* SoundManager::PlaySound(HashLabel SoundName, u8 VolumeLeft, u8 VolumeRight, bool bLoop)
{
	if ( !Singleton<WiiManager>::GetInstanceByRef().IsGameStateGame() )
		return NULL;

	RawSample* pRaw = GetSound(SoundName);
	if (pRaw!=NULL)
	{
		AESNDPB* pSound = m_SoundContainer[SoundName]->Play( VolumeLeft, VolumeRight, bLoop ); 
		return pSound;
	}

	ExitPrintf("missing sound");
	return NULL;
}


void SoundManager::PlaySoundFromVoice(HashLabel SoundName, u8 VolumeLeft, u8 VolumeRight, bool bLoop, AESNDPB* VoiceData)
{
	if ( !Singleton<WiiManager>::GetInstanceByRef().IsGameStateGame() )
		return;

	RawSample* pRaw = GetSound(SoundName);
	if (pRaw!=NULL)
	{
		m_SoundContainer[SoundName]->PlayFromVoice( VolumeLeft, VolumeRight, bLoop, VoiceData); 
	}
	else
		ExitPrintf("missing sound");
}

void SoundManager::StopSound(AESNDPB* Chan)
{
	if ( !Singleton<WiiManager>::GetInstanceByRef().IsGameStateGame() )
		return;

	AESND_SetVoiceStop(Chan,true);
}

SoundManager::SoundManager()// : m_FixMusicVoice(NULL)
{ 
	//Init();
}

SoundManager::~SoundManager()
{ 
	//UnInit();
}

void SoundManager::Init( )
{ 
	GRRMOD_Init(true);  // this will to things like AESND_Init();  for you

//AESND_Init();

	m_FixSoundVoice = AESND_AllocateFixedVoice(NULL); // used for players looping thrusters
//	m_FixSoundVoice = AESND_AllocateVoice(NULL); // used for players looping thrusters


	m_OggPlayer.Init();

}

void SoundManager::UnInit( )
{ 
    GRRMOD_End();
}

void SoundManager::LoadSound( std::string FullFileNameWithPath,std::string LookUpName )
{
	Util::StringToLower(FullFileNameWithPath);

	if (WiiFile::GetFileExtension(FullFileNameWithPath) == "wav")
	{
		StoreSoundFromWav(FullFileNameWithPath,LookUpName);
	}
	else if (WiiFile::GetFileExtension(FullFileNameWithPath) == "ogg")
	{
		StoreSoundFromOgg(FullFileNameWithPath,LookUpName);
	}
}

// StoreSoundFromWav is a supporting function for LoadSound
void SoundManager::StoreSoundFromWav( std::string FullFileNameWithPath,std::string LookUpName )
{
	FILE* WAVFile = WiiFile::FileOpenForRead((FullFileNameWithPath).c_str());
	//-------------------------------------------------------------
	// "RIFF" chunk discriptor
	RIFFChunk RIFFChunkData;
	fread(&RIFFChunkData, sizeof(RIFFChunk), 1, WAVFile);
	if ( strncmp ( RIFFChunkData.RIFF, "RIFF", 4 ) != 0 )
		ExitPrintf("'RIFF' check failed %c%c%c%c" ,RIFFChunkData.RIFF[0],RIFFChunkData.RIFF[1],RIFFChunkData.RIFF[2],RIFFChunkData.RIFF[3] );
	if ( strncmp ( RIFFChunkData.RIFFType, "WAVE", 4 ) != 0 )
		ExitPrintf("'WAVE' check failed %c%c%c%c",RIFFChunkData.RIFFType[0],RIFFChunkData.RIFFType[1],RIFFChunkData.RIFFType[2],RIFFChunkData.RIFFType[3]);
	//-------------------------------------------------------------
	// "fmt" sub-chunk
	fmtChunk fmtChunkData;
	fread(&fmtChunkData, sizeof(fmtChunk), 1, WAVFile);
	if ( strncmp ( fmtChunkData.fmt, "fmt ", 4 ) != 0 )
		ExitPrintf("fmt' check failed %c%c%c%c" , fmtChunkData.fmt[0],fmtChunkData.fmt[1],fmtChunkData.fmt[2],fmtChunkData.fmt[3]);

	fmtChunkData.Channels = Util::ENDIAN16(fmtChunkData.Channels);
	fmtChunkData.SampleRate = Util::ENDIAN32(fmtChunkData.SampleRate);
	//-------------------------------------------------------------
	// "data" sub-chunk
	dataChunk dataChunkData;
	fread(&dataChunkData, sizeof(dataChunk), 1, WAVFile);
	if ( strncmp ( dataChunkData.data, "data", 4 ) != 0 )
		ExitPrintf("'data' check failed");

	dataChunkData.dataLength = Util::ENDIAN32(dataChunkData.dataLength);
	//-------------------------------------------------------------
	// Raw sound data
	RawSample* pRawSample( new RawSample );
	u8* pData = (u8*)memalign(32, dataChunkData.dataLength );
	fread(pData, 1, dataChunkData.dataLength, WAVFile);
	fmtChunkData.BitResolution = Util::ENDIAN16(fmtChunkData.BitResolution);
	pRawSample->SetRawData(pData);
	pRawSample->SetRawDataLength(dataChunkData.dataLength);


	if (fmtChunkData.BitResolution == 16 )
	{
	
		if (fmtChunkData.Channels == 1)
		{
			pRawSample->SetVoiceFormat(VOICE_MONO16);
				printf("VOICE_MONO16");
		}
		else
		{
			pRawSample->SetVoiceFormat(VOICE_STEREO16);
			printf("VOICE_STEREO16");
		}
	}
	else
	{
		if (fmtChunkData.Channels == 1)
			pRawSample->SetVoiceFormat(VOICE_MONO8);
		else
			pRawSample->SetVoiceFormat(VOICE_STEREO8);
	}

	pRawSample->SetSampleRate(fmtChunkData.SampleRate);
	pRawSample->SetBitsPerSample(fmtChunkData.BitResolution);
	//printf("dataLength %d",dataChunkData.dataLength);
	printf("Channels %d",fmtChunkData.Channels);
	printf("SampleRate %d",fmtChunkData.SampleRate);
	printf("BitResolution %d",fmtChunkData.BitResolution);
	//-------------------------------------------------------------
	u16* pData16 = (u16*)pData;
	if (fmtChunkData.BitResolution == 16) // 8 or 16 bit samples - anything other than 16 is just seen as 8 bit
	{
		for (u32 i(0); i<dataChunkData.dataLength / (fmtChunkData.BitResolution/8); i++)
		{
			pData16[i] = Util::ENDIAN16(pData16[i]);
		}
	}

	fclose ( WAVFile );
	m_SoundContainer[ (HashLabel)LookUpName ] = pRawSample ;
}


u32 SoundManager::GetOggTotal(OggVorbis_File* vf)
{
	char PCM_Out[2048];
	u32 Total=0;
	int current_section;
	long ret(0);
	do
	{
//
//#ifdef
//		static const int BIGENDIAN (1);
//		ret = ov_read(vf,PCM_Out,sizeof(PCM_Out),BIGENDIAN,2,1,&current_section);
//		Total += ret;
//#else
		ret = ov_read(vf,PCM_Out,sizeof(PCM_Out),&current_section);
//		printf("===%d",ret);
		Total += ret;
//#endif
	}while (ret!=0);

	return Total;
}



// StoreSoundFromOgg is a supporting function for LoadSound
void SoundManager::StoreSoundFromOgg(std::string FullFileNameWithPath,std::string LookUpName)
{ 
	//printf("start ogg");

	char PCM_Out[4096]; // quick working fudge - todo make it dynamic
	OggVorbis_File vf;

	int current_section;

	FILE* pFile = WiiFile::FileOpenForRead( FullFileNameWithPath.c_str() );

	if(ov_open(  pFile , &vf, NULL, 0) < 0) 
	{
		fclose(pFile);
		ExitPrintf("Not a Ogg file\n");
	}

	char **ptr=ov_comment(&vf,-1)->user_comments;
	vorbis_info *vi=ov_info(&vf,-1);
	while (*ptr)
	{
		//printf("%s\n",*ptr);
		++ptr;
	}

	//   !!!!! SHIT - some oggs are unseekable   !!!!
	// need to fallback to something else, it's going to be slow
	s32 pcm_total = ov_pcm_total(&vf,-1);  // should be 64bits , but I'm not using anything that big!
	if (pcm_total == OV_EINVAL)	
	{
	//	printf( "%ld", ov_time_total(&vf,-1) );

		pcm_total = GetOggTotal(&vf);

	//	printf("slow VERSION: pcm_total %d",pcm_total);

		// Fudge - not possible to use ov_pcm_total, need to close file to zero seek (not nice, but can't find any other way)
		ov_clear(&vf); // this will close the open file
		ov_open(  WiiFile::FileOpenForRead( FullFileNameWithPath.c_str() ) , &vf, NULL, 0);
	}
	else
	{
		pcm_total *= vi->channels;
		pcm_total *= sizeof(u16);

	//	printf("FAST VERSION: pcm_total %d",pcm_total);
	}

//	printf("\nBitstream is %d channel, %ldHz\n",vi->channels,vi->rate);	
//	printf("\nDecoded length: %d samples\n",pcm_total);
//	printf("Encoded by: %s\n\n",ov_comment(&vf,-1)->vendor);
//	printf("channels: %d\n\n",vi->channels);
//	printf("pcm_total: %d\n\n", pcm_total );

	// Raw sound data
	RawSample* pRawSample( new RawSample );

	int NumberOfChannels = VOICE_STEREO16;
	if (vi->channels==1)
	{
//		printf("VOICE_MONO16");
		NumberOfChannels = VOICE_MONO16;
	}
//	else
//	{
//		printf("VOICE_STEREO16");
//	}

	int SampleRate = vi->rate;
	int BitsPerSample = 16;
	
	//printf("rate %d", vi->rate);


	u8* pRawData = (u8*)memalign(32, pcm_total );
	if (pRawData==NULL)
	{
		ov_clear(&vf);
		ExitPrintf("fail memalign(32,%d)", pcm_total);
	}

	u8* pTemp = pRawData;

	int eof=0;
	//u32 CheckTotal=0;
	while(!eof)
	{

//#ifdef 
//		static const int BIGENDIAN (1);
//		long ret= ov_read(vf,PCM_Out,sizeof(PCM_Out),BIGENDIAN,2,1,&current_section);
//#else
		long ret= ov_read(&vf,PCM_Out,sizeof(PCM_Out),&current_section);
//#endif

		if (ret == 0) 
		{
			eof=1;
		} 
		//else if (ret < 0) // error in the stream
		//{
		//} 
		else // we don't bother dealing with sample rate changes, etc, but you'll have to
		{

			memcpy(pTemp, PCM_Out, ret) ;//fwrite(PCM_Out,1,ret,pOutFile);
			pTemp+=ret;

		}
	}
//	printf("CheckTotal  %d",pcm_total);

//	if (!ov_seekable(&vf))
//		ExitPrintf("not seekable");
//	double length=ov_time_total(&vf,-1);

	// **********************************************************************************************
	// Once a FILE * handle is passed to ov_open() successfully, the application 
	// MUST NOT fclose() or in any other way manipulate that file handle. 
	// Vorbisfile will close the file in ov_clear(). If the application must be able to 
	// close the FILE * handle itself, see ov_open_callbacks() with the use of OV_CALLBACKS_NOCLOSE. 
    // **********************************************************************************************
	
	ov_clear(&vf); // this will close the open file



//	printf("SampleRate  %d",SampleRate);
//	printf("SampleRate  %d",BitsPerSample);
//	printf("SampleRate  %d",NumberOfChannels);
//	printf("SampleRate  %d",pcm_total);

	//-------------------------------------------------------------
	pRawSample->SetRawData(pRawData);
	pRawSample->SetRawDataLength(pcm_total);
	pRawSample->SetVoiceFormat(NumberOfChannels);
	pRawSample->SetSampleRate(SampleRate);
	pRawSample->SetBitsPerSample(BitsPerSample);
	//-------------------------------------------------------------


	m_SoundContainer[ (HashLabel)LookUpName ] = pRawSample ;


	//Util::SleepForMilisec(6000);
}



//-----------------------------------------------------------------------------------------
// MP3 stuff bellow works fine but is no longer needed - leaving commented out for future reference
//-----------------------------------------------------------------------------------------

//#include "mad.h"
//////////
//////////u32 PCM_Length =0;
//////////
//////////u16* pTemporarySpaceReserveWhileLoading = NULL;
//////////
//////////
///////////* 
//////////* This is a private message structure. A generic pointer to this structure 
//////////* is passed to each of the callback functions. Put here any data you need 
//////////* to access from within the callbacks. 
//////////*/  
//////////  
//////////struct Tbuffer 
//////////{  
//////////    unsigned char const *start;  
//////////    unsigned long length;  
//////////};  
///////////* 
//////////* This is the error callback function. It is called whenever a decoding 
//////////* error occurs. The error is indicated by stream->error; the list of 
//////////* possible MAD_ERROR_* errors can be found in the mad.h (or stream.h) 
//////////* header file. 
//////////*/  
//////////  
//////////mad_flow error(void *data, struct mad_stream *stream,  struct mad_frame *frame)  
//////////{  
//////////    Tbuffer *buffer =(Tbuffer*) data;  
//////////      
//////////    fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n",  
//////////        stream->error, mad_stream_errorstr(stream),  
//////////        stream->this_frame - buffer->start);  
//////////      
//////////    /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */  
//////////      
//////////    return MAD_FLOW_CONTINUE;  
//////////}  
//////////
///////////* 
//////////* This is the input callback. The purpose of this callback is to (re)fill 
//////////* the stream buffer which is to be decoded. In this example, an entire file 
//////////* has been mapped into memory, so we just call mad_stream_buffer() with the 
//////////* address and length of the mapping. When this callback is called a second 
//////////* time, we are finished decoding. 
//////////*/  
//////////  
//////////mad_flow input(void *data, mad_stream *stream)  
//////////{  
//////////    Tbuffer* buffer = (Tbuffer*) data;  
//////////      
//////////    if (!buffer->length)  
//////////        return MAD_FLOW_STOP;  
//////////      
//////////    mad_stream_buffer(stream, buffer->start, buffer->length);  
//////////      
//////////    buffer->length = 0;  
//////////      
//////////    return MAD_FLOW_CONTINUE;  
//////////}  
//////////  
///////////* 
//////////* The following utility routine performs simple rounding, clipping, and 
//////////* scaling of MAD's high-resolution samples down to 16 bits. It does not 
//////////* perform any dithering or noise shaping, which would be recommended to 
//////////* obtain any exceptional audio quality. It is therefore not recommended to 
//////////* use this routine if high-quality output is desired. 
//////////*/  
//////////  
//////////s16 scale(mad_fixed_t sample)  
//////////{  
//////////    sample += (1L << (MAD_F_FRACBITS - 16));   /* round */  
//////////    if (sample >= MAD_F_ONE)  
//////////	{
//////////        sample = MAD_F_ONE - 1;  
//////////	}
//////////    else if (sample < -MAD_F_ONE)  
//////////	{
//////////		sample = -MAD_F_ONE; 
//////////	}
//////////	return sample >> (MAD_F_FRACBITS + 1 - 16);   /* quantize */  
//////////}  
//////////  
///////////* 
//////////* This is the output callback function. It is called after each frame of 
//////////* MPEG audio data has been completely decoded. The purpose of this callback 
//////////* is to output (or play) the decoded PCM audio. 
//////////*/  
//////////  
//////////mad_flow output(void *data,mad_header const *header, mad_pcm *pcm)  
//////////{  
//////////     /* output sample(s) in 16-bit signed little-endian PCM */  
//////////    /* pcm->samplerate contains the sampling frequency */  
//////////   	// note: any of these two methods can be use below
//////////
//////////    //s16 sample = scale(*left_ch++);   
//////////	//sam[0] = (sample >> 0) & 0xff;
//////////	//sam[1] = (sample >> 8) & 0xff;
//////////	// fwrite(&sam, 1, 2, fppcm);   
//////////	// or...
//////////	// fputc ((sample >> 0) & 0xff,fppcm);   
//////////	// fputc ((sample >> 8) & 0xff,fppcm);    
//////////
//////////	u32 nsamples  = pcm->length;  
//////////    mad_fixed_t const* left_ch  = pcm->samples[MAD_PCM_CHANNEL_STEREO_LEFT];  
//////////    mad_fixed_t const* right_ch = pcm->samples[MAD_PCM_CHANNEL_STEREO_RIGHT];  
//////////
//////////	u16* ptr = &pTemporarySpaceReserveWhileLoading[PCM_Length / sizeof(u16)]; // current posistion to start filling data
//////////
//////////	if (pcm->channels == 2) 
//////////	{  
//////////	//	PCM_Length += (sizeof(u16) * nsamples) * 2;
//////////
//////////		while (nsamples--) 
//////////		{  
//////////			s16 sample = scale(*left_ch++);   
//////////			*ptr = sample;
//////////			ptr++;
//////////
//////////			sample = scale(*right_ch++);  
//////////			*ptr = sample;
//////////			ptr++;
//////////
//////////			PCM_Length += sizeof(u16) * 2;
//////////		} 
//////////
//////////		printf("%d",PCM_Length);
//////////	}
//////////	else if (pcm->channels == 1) 
//////////	{
//////////		ExitPrintf("Error: decoding something with less than 2 channels");
//////////		PCM_Length += nsamples;  // 16bit
//////////
//////////		while (nsamples--) 
//////////		{  
////////////			sam[0] = (sample >> 0) & 0xff;
////////////			sam[1] = (sample >> 8) & 0xff;
//////////			*ptr = scale(*left_ch++);  
//////////			ptr++;
//////////		}
//////////	}
//////////	else
//////////	{
//////////		ExitPrintf("Error: decoding something with more than 2 channels");
//////////		return MAD_FLOW_BREAK;
//////////	}
//////////      
//////////    return MAD_FLOW_CONTINUE;  
//////////}  
//////////  
///////////* 
//////////* This is the function called by main() above to perform all the decoding. 
//////////* It instantiates a decoder object and configures it with the input, 
//////////* output, and error callback functions above. A single call to 
//////////* mad_decoder_run() continues until a callback function returns 
//////////* MAD_FLOW_STOP (to stop decoding) or MAD_FLOW_BREAK (to stop decoding and 
//////////* signal an error). 
//////////*/  
//////////  
//////////int decode(unsigned char const *start, unsigned long length)  
//////////{  
//////////	Tbuffer buffer; // = { start ,length } ;  
//////////    buffer.start  = start;  
//////////    buffer.length = length;  
//////////      
//////////    mad_decoder decoder;  
//////////    mad_decoder_init(&decoder, &buffer,  
//////////        input, 0 /* header */, 0 /* filter */, output,  
//////////        error, 0 /* message */);  
//////////      
//////////	printf("mad_decoder_run start");
//////////    int result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);  
//////////	printf("mad_decoder_run end");  
//////////    mad_decoder_finish(&decoder);      
//////////    return result;  
//////////}  
//////////
//////////
//////////u16* DecodeMp3FileAsPCM(string FileName)  
//////////{ 
//////////	FILE *pMp3File = WiiFile::FileOpenForRead(FileName.c_str()); //"sd://apps/BoltThrower/L.mp3");   
//////////	int FileLength = WiiFile::GetFileSize(pMp3File);
//////////	u8* pMp3Memory  = (u8*)malloc(FileLength);   
//////////	fread(pMp3Memory, 1, FileLength, pMp3File);  
//////////  
//////////	printf("decode start");
//////////	PCM_Length=0;
//////////	pTemporarySpaceReserveWhileLoading = (u16*)memalign(32, 1024*1024*32);
//////////	if (pTemporarySpaceReserveWhileLoading==NULL)
//////////	{
//////////		ExitPrintf("      mem");
//////////	}
//////////
//////////    decode( (u8*)pMp3Memory, FileLength);        
//////////	printf("decode end");
//////////    free(pMp3Memory);   
//////////    fclose(pMp3File);   
//////////    return pTemporarySpaceReserveWhileLoading;  
//////////}  
//////////  



//	u16* pSound = DecodeMp3FileAsPCM(Util::GetGamePath() +  "music.mp3" );



////-----------------------------------------------------
// test for MP3 
//string name( Util::GetGamePath() + "Music.MP3");
//FILE* pFile( WiiFile::FileOpenForRead(name.c_str() ) );
//fseek (pFile , 0 , SEEK_END);
//int FileSize( ftell( pFile ) );
//rewind( pFile );
//u8* sample_mp3( (u8*) malloc (sizeof(u8)*FileSize));
////u8* sample_mp3 = new u8[FileSize];
////fread ( &sample_mp3, 1,FileSize, pFile );
////size_t result = fread (sample_mp3,1,FileSize,pFile);
//fread (sample_mp3,1,FileSize,pFile);
//fclose (pFile);
//MP3Player_Init();
//MP3Player_PlayBuffer(sample_mp3, FileSize, NULL);
////----------------------------------------------------