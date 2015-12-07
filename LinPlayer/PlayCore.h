#pragma once

//using owner main, not SDL_main
#define SDL_MAIN_HANDLED


#include "packet.h"


class CPlayCore
{
public:
	CPlayCore(HWND w);
	~CPlayCore();

	static int InitEnv(void);
	static void sigterm_handler(int sig);
	static int decode_interrupt_cb(void *ctx);

	static int beginRead(void *ptr);
	static int beginDisplay(void *ptr);
	static int beginEvent(void *ptr);

	int thread_Read(void);
	int thread_Display(void);
	int thread_Event(void);

	void video_refresh(double *remaining_time);

	int stream_video_open();
	int stream_audio_open();
	int stream_subtitle_open();

	void RecordError(const char *MediaName, const char *action, int error /* = 0 */);
	bool isPlaying(void)	{return bPlaying; }
	bool isPause(void)	{return bPause; }

	int play(const char *url);
	void pause(void);
	void stop(void);

private:
	HWND wnd;
	bool bPause,bStop,bPlaying;
	bool video_disable, audio_disable, subtitle_disable;

	SDL_Thread *Read_tid, *Display_tid, *Event_tid;
	SDL_cond *continue_read_thread;

	AVFormatContext *ic;
	AVCodecContext  *pCodecCtx;
	SwsContext *img_convert_ctx;
	AVPacket *packet;
	
	SDL_Window *screen;
	SDL_Renderer *renderer;
	
	int video_stream, audio_stream, subtitle_stream;
	AVStream *video_st,*audio_st;
	PacketQueue videoq, audioq, subtitleq;
};

