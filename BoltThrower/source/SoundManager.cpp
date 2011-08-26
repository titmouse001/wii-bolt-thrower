#include <asndlib.h>

#include "SoundManager.h"
#include "malloc.h"
#include "debug.h"
#include "string.h"
#include "config.h"
#include "WiiManager.h"
#include "WiiFile.h"
#include "Util.h"
#include "tremor/ivorbiscodec.h"
#include "tremor/ivorbisfile.h"

int RawSample::Play(u8 VolumeLeft, u8 VolumeRight, bool bLoop)
{
	if ( !Singleton<WiiManager>::GetInstanceByRef().IsGameStateShowingGame() )
		return 0;

	int Chan = ASND_GetFirstUnusedVoice();

	if (bLoop)
		ASND_SetInfiniteVoice( Chan, m_NumberOfChannels,m_SampleRate,0, m_RawData, m_RawDataLength , VolumeLeft, VolumeRight);
	else
		ASND_SetVoice( Chan, m_NumberOfChannels,m_SampleRate,0, m_RawData, m_RawDataLength , VolumeLeft, VolumeRight, NULL);

	return Chan;
}	

int SoundManager::PlaySound(HashLabel SoundName, u8 VolumeLeft, u8 VolumeRight, bool bLoop)
{
	if ( !Singleton<WiiManager>::GetInstanceByRef().IsGameStateShowingGame() )
		return 0;


	//todo - replace [] will something that will fail rather then create a new item we are looking for !
	RawSample* pRaw = GetSound(SoundName);
	if (pRaw!=NULL)
	{
		return m_SoundContainer[SoundName]->Play( VolumeLeft, VolumeRight, bLoop ); 
	}


	return -1;
}

void SoundManager::StopSound(u8 Chan)
{
	if ( !Singleton<WiiManager>::GetInstanceByRef().IsGameStateShowingGame() )
		return;

	ASND_StopVoice(Chan);
}


SoundManager::SoundManager( )
{ 
	Init();
}

SoundManager::~SoundManager( )
{ 
	UnInit();
}

void SoundManager::Init( )
{ 
#ifdef ENABLE_SOUND
	SND_Init(INIT_RATE_48000);  // note: SND_xxx is the same as ASND_xxx
	SND_Pause(0);  
#endif
}

void SoundManager::UnInit( )
{ 
	ASND_Pause(1);  //pause
	ASND_End();
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
			pRawSample->SetNumberOfChannels(VOICE_MONO_16BIT);
		else
			pRawSample->SetNumberOfChannels(VOICE_STEREO_16BIT);
	}
	else
	{
		if (fmtChunkData.Channels == 1)
			pRawSample->SetNumberOfChannels(VOICE_MONO_8BIT);
		else
			pRawSample->SetNumberOfChannels(VOICE_STEREO_8BIT);
	}

	pRawSample->SetSampleRate(fmtChunkData.SampleRate);
	pRawSample->SetBitsPerSample(fmtChunkData.BitResolution);
	//printf("dataLength %d",dataChunkData.dataLength);
	//printf("Channels %d",fmtChunkData.Channels);
	//printf("SampleRate %d",fmtChunkData.SampleRate);
	//printf("BitResolution %d",fmtChunkData.BitResolution);
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

// StoreSoundFromOgg is a supporting function for LoadSound
void SoundManager::StoreSoundFromOgg(std::string FullFileNameWithPath,std::string LookUpName)
{ 
	//printf("start ogg");

	char PCM_Out[4096]; // quick working fudge - todo make it dynamic
	OggVorbis_File vf;

	int current_section;

	if(ov_open(  WiiFile::FileOpenForRead( FullFileNameWithPath.c_str() ) , &vf, NULL, 0) < 0) 
	{
		ExitPrintf("Not a Ogg file\n");
	}

	//	We're going to pull the channel and bitrate info from the file using ov_info() 
	//and show them to the user. We also want to pull out and show the user a comment attached to the file using ov_comment(). 
	
		char **ptr=ov_comment(&vf,-1)->user_comments;
		vorbis_info *vi=ov_info(&vf,-1);
		while(*ptr){
			fprintf(stderr,"%s\n",*ptr);
			++ptr;
		}
		//printf("\nBitstream is %d channel, %ldHz\n",vi->channels,vi->rate);
		//printf("\nDecoded length: %ld samples\n",(long)ov_pcm_total(&vf,-1));
		//printf("Encoded by: %s\n\n",ov_comment(&vf,-1)->vendor);
		//printf("--------");
		//printf("%ld", ((long)ov_pcm_total(&vf,-1)) * vi->channels * (int)sizeof(u16) );
		//printf("--------");
	
	// Raw sound data
	RawSample* pRawSample( new RawSample );
	int RawDataLength = ((long)ov_pcm_total(&vf,-1)) * vi->channels * (int)sizeof(u16);

	int NumberOfChannels = VOICE_STEREO_16BIT;
	if (vi->channels==1)
		NumberOfChannels = VOICE_MONO_16BIT;

	int SampleRate = vi->rate;
	int BitsPerSample = 16; //sizeof(u16);

	RawDataLength/=2;

	u8* pRawData = (u8*)memalign(32, RawDataLength );
	if (pRawData==NULL)
	{
		ExitPrintf("memalign NULL");
	}

	u8* pTemp = pRawData;

	int eof=0;
	int CheckTotal=0;
	while(!eof)
	{
		long ret=ov_read(&vf,PCM_Out,sizeof(PCM_Out),&current_section);
		if (ret == 0) 
		{
			eof=1;
		} 
		else if (ret < 0) // error in the stream
		{
		} 
		else // we don't bother dealing with sample rate changes, etc, but you'll have to
		{
			if (CheckTotal>=RawDataLength)
			{
				//printf("half");
				break;
			}
			memcpy(pTemp, PCM_Out, ret) ;//fwrite(PCM_Out,1,ret,pOutFile);
			pTemp+=ret;
			CheckTotal += ret;
		}
	}
	ov_clear(&vf);

	//printf("SampleRate  %d",SampleRate);
	//printf("SampleRate  %d",BitsPerSample);
	//printf("SampleRate  %d",NumberOfChannels);
	//printf("SampleRate  %d",RawDataLength);

	pRawSample->SetRawData(pRawData);
	pRawSample->SetRawDataLength(RawDataLength);
	pRawSample->SetNumberOfChannels(NumberOfChannels);
	pRawSample->SetSampleRate(SampleRate);
	pRawSample->SetBitsPerSample(BitsPerSample);
	//-------------------------------------------------------------

	m_SoundContainer[ (HashLabel)LookUpName ] = pRawSample ;
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