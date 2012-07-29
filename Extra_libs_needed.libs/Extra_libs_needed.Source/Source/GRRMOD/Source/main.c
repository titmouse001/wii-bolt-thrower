/*===========================================
        GRRMOD
        - Test Code -
============================================*/
//#include <grrlib.h>
//
// removed anything graphics-wise from this test rig - Titmouse
//

#include "grrmod.h"

#include <stdlib.h>
#include <wiiuse/wpad.h>

// Image file
//#include "Impact_9_png.h"

// Mod file
#include "music_mp3.h"
#include "music_mod.h"
#include "music_xm.h"
#include "music_s3m.h"
#include "music_it.h"

#define MAX_WIDTH 6.0f
#define MIN_WIDTH 0.2f
#define DECAY 0.5f;

typedef struct
{
    u8 *Mem;
    u32 Size;
} PLAYLIST;

typedef struct
{
    int freq;
    int vol;
    int realvol;
    float width;
} CH;

//static CH channel1 = {0, 0, 0, MIN_WIDTH};
//static CH channel2 = {0, 0, 0, MIN_WIDTH};
//static CH channel3 = {0, 0, 0, MIN_WIDTH};
//static CH channel4 = {0, 0, 0, MIN_WIDTH};
//static float calc_size(u8 voice, CH* channel);


int main(int argc, char **argv) {
  //  float a = 0.0f;
    s16 Volume = 255;
    s8 SongNum = 0;
    PLAYLIST PlayList[] = { {(u8 *)music_mp3, music_mp3_size}, // lib compiled to ignore mp3, this line is not used - will do nothing is used!
                            {(u8 *)music_mod, music_mod_size},
                            {(u8 *)music_s3m, music_s3m_size},
                            {(u8 *)music_it, music_it_size},
                            {(u8 *)music_xm, music_xm_size} };

 //   GRRLIB_Init();
    //GRRLIB_texImg *tex_Font = GRRLIB_LoadTexture(Impact_9_png);
    //GRRLIB_InitTileSet(tex_Font, 10, 16, 32);

	VIDEO_Init();
	WPAD_Init();

//    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC);

  //  GRRLIB_Settings.antialias = true;
  //  GRRLIB_SetBackgroundColour(0x00, 0x00, 0x00, 0xFF);

  
  	 SongNum++;
	 
    GRRMOD_Init(true);
    GRRMOD_SetMOD(PlayList[SongNum].Mem, PlayList[SongNum].Size);
    GRRMOD_Start(255); // start tune
			
			
  
    // Loop forever
    while(1) {
       // GRRLIB_2dMode();
        WPAD_ScanPads();  // Scan the Wiimotes

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) {
            break;
        }
        if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_PLUS) {
            Volume++;
            GRRMOD_SetVolume(Volume, Volume);
        }
        if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_MINUS) {
            Volume--;
            GRRMOD_SetVolume(Volume, Volume);
        }
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A) {
            GRRMOD_Pause();
        }
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_1) {
            GRRMOD_Start(255);
        }
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_2) {
            GRRMOD_Stop();
        }
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_LEFT) {
            SongNum--;
            if(SongNum < 0) SongNum = 0;
            GRRMOD_Unload();
            GRRMOD_SetMOD(PlayList[SongNum].Mem, PlayList[SongNum].Size);
            GRRMOD_Start(255);
        }
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_RIGHT) {
            SongNum++;
            if(SongNum > 4) SongNum = 4;
            GRRMOD_Unload();
            GRRMOD_SetMOD(PlayList[SongNum].Mem, PlayList[SongNum].Size);
            GRRMOD_Start(255);
        }
		VIDEO_WaitVSync();
    }

    GRRMOD_End();
  //  GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB

    exit(0);  // Use exit() to exit a program, do not use 'return' from main()
}
//
//static float calc_size(u8 voice, CH* channel) 
//{
//    int freq = GRRMOD_GetVoiceFrequency(voice);
//    int vol = GRRMOD_GetVoiceVolume(voice);
//    int realvol = GRRMOD_GetRealVoiceVolume(voice);
//
//    if (freq != channel->freq || vol != channel->vol || realvol > channel->realvol)
//    {
//        channel->width = MAX_WIDTH;
//    }
//    else
//    {
//        channel->width -= DECAY;
//        if (channel->width < MIN_WIDTH)
//            channel->width = MIN_WIDTH;
//    }
//
//    channel->vol = vol;
//    channel->freq = freq;
//    channel->realvol = realvol;
//
//    return channel->width;
//}
