#pragma once

#include <windows.h>
#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}
#endif
#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"

//define
#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 5
#define AV_DISPOSITION_ATTACHED_PIC 0x0400

/* polls for possible required screen refresh at least this often, should be less than 1/fps */
#define REFRESH_RATE 0.01

typedef struct MyAVPacketList {
	AVPacket pkt;
	struct MyAVPacketList *next;
	int serial;
} MyAVPacketList;

typedef struct PacketQueue {
	MyAVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	int abort_request;
	int serial;
	SDL_mutex *mutex;
	SDL_cond *cond;
} PacketQueue;