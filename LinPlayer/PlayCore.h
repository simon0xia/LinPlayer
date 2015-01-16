#pragma once

#include <windows.h>

//using owner main, not SDL_main
#define SDL_MAIN_HANDLED

#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}
#endif
#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"

class CPlayCore
{
public:
	CPlayCore(HWND w);
	~CPlayCore();

	static int InitEnv(void);

	void RecordError(const char *MediaName, const char *action, int error /* = 0 */);

	int play(const char *url);

private:
	HWND wnd;

};

