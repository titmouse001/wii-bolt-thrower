
#ifndef OggPlayer_H
#define OggPlayer_H

// this class stll TODO !!!

class OggPlayer
{
public:

	#define OGG_ONE_TIME         0
	#define OGG_INFINITE_TIME    1
	#define OGG_STATUS_RUNNING   1
	#define OGG_STATUS_ERR      -1
	#define OGG_STATUS_PAUSED    2
	#define OGG_STATUS_EOF     255

	int PlayOgg(const void *buffer, s32 len, int time_pos, int mode);
	void StopOgg();
	void PauseOgg(bool Status = true);
	int StatusOgg();
	void SetVolumeOgg(int volume);
	s32 GetTimeOgg();
	void SetTimeOgg(s32 time_pos);

private:



};

#endif
