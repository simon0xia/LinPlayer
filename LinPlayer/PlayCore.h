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

	int thread_Read(void);
	int thread_Display(void);

	void RecordError(const char *MediaName, const char *action, int error /* = 0 */);

	int play(const char *url);
	void stop(void);

private:
	HWND wnd;
	bool bPause;
	bool bStop;

	SDL_Thread *Read_tid, *Display_tid;
	SDL_cond *continue_read_thread;

	AVFormatContext *ic;
	AVCodecContext  *pCodecCtx;
	SwsContext *img_convert_ctx;
	AVPacket *packet;
	
	SDL_Window *screen;
	SDL_Renderer *renderer;
	
	int video_stream;
	AVStream *video_st;
	PacketQueue videoq;

	int audio_stream;
	AVStream *audio_st;
};

