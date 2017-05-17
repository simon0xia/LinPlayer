#include "PlayCore.h"
#include "Log.h"
#include <signal.h>
#include <QDebug>

extern AVPacket flush_pkt;

CPlayCore::CPlayCore(HWND w)
:wnd(w), bPause(false), bStop(false), bPlaying(false)
{
	ic = NULL;
	pCodecCtx = NULL;
	img_convert_ctx = NULL;
	packet = NULL;
	screen = NULL;
	renderer = NULL;

	video_disable = audio_disable = false;
	subtitle_disable = true;
}

CPlayCore::~CPlayCore()
{
	avcodec_close(pCodecCtx);
	if (ic != NULL)
		avformat_close_input(&ic);

	av_free_packet(packet);
	sws_freeContext(img_convert_ctx);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(screen);

	packet_queue_destroy(&videoq);
	packet_queue_destroy(&audioq);
	packet_queue_destroy(&subtitleq);

	//SDL_Quit();
}

int CPlayCore::decode_interrupt_cb(void *ctx)
{
	CPlayCore *p = (CPlayCore *)ctx;
	return p->bStop;
}

void CPlayCore::sigterm_handler(int sig)
{
	LogIns.FlashLog("sigterm_handler, ID:%d", sig);
	exit(123);
}

int CPlayCore::InitEnv(void)
{
	av_register_all();
	avformat_network_init();

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		LogIns.FlashLog("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}

	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);

	signal(SIGINT, sigterm_handler); /* Interrupt (ANSI).    */
	signal(SIGTERM, sigterm_handler); /* Termination (ANSI).  */

	return 0;
}

void CPlayCore::RecordError(const char *MediaName, const char *action, int error = 0)
{
	char errbuff[100];
	av_make_error_string(errbuff, 100, error);
	LogIns.FlashLog("%s %s. error info:%s\n", MediaName, action, errbuff);
}

int CPlayCore::beginRead(void *ptr)
{
	CPlayCore *p = (CPlayCore *)ptr;
	p->thread_Read();
	return 0;
}

int CPlayCore::thread_Read(void)
{
	qDebug() << __FUNCTION__;

	SDL_mutex *wait_mutex = NULL;
	int ret = 0;
	int eof = 0;
	AVPacket pkt1, *pkt = &pkt1;

	while (!bStop)	{
// 		if (is->seek_req){
// 			SeekFunction(ctrl);
// 			is->seek_req = 0;
// 			eof = 0;
// 		}
		
		// if the queue are full, no need to read more 
		if (videoq.size + audioq.size + subtitleq.size > MAX_QUEUE_SIZE
			|| ((videoq.nb_packets > MIN_FRAMES || video_stream < 0 || videoq.abort_request
					|| (video_st->disposition & AV_DISPOSITION_ATTACHED_PIC))
				&& (audioq.nb_packets > MIN_FRAMES || audio_stream < 0 || audioq.abort_request)
				&& (subtitleq.nb_packets > MIN_FRAMES || subtitle_stream < 0 || subtitleq.abort_request))){
			// wait 10 ms 
			SDL_LockMutex(wait_mutex);
			SDL_CondWaitTimeout(continue_read_thread, wait_mutex, 10);
			SDL_UnlockMutex(wait_mutex);
			continue;
		}
		ret = av_read_frame(ic, pkt);
		if (ret < 0) {
			if (ret == AVERROR_EOF || avio_feof(ic->pb) && !eof){
				if (video_stream >= 0)
					packet_queue_put_nullpacket(&videoq, video_stream);
				if (audio_stream >= 0)
					packet_queue_put_nullpacket(&audioq, audio_stream);
				if (subtitle_stream >= 0)
					packet_queue_put_nullpacket(&subtitleq, subtitle_stream);
				eof = 1;
			}
			if (ic->pb && ic->pb->error)
				break;
			SDL_LockMutex(wait_mutex);
			SDL_CondWaitTimeout(continue_read_thread, wait_mutex, 10);
			SDL_UnlockMutex(wait_mutex);
			continue;
		} else {
			eof = 0;
		}
		
		if (pkt->stream_index == video_stream
				&& !(video_st->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
			packet_queue_put(&videoq, pkt);
		} else if (pkt->stream_index == audio_stream) {
			packet_queue_put(&audioq, pkt);
		} else if (pkt->stream_index == subtitle_stream) {
			packet_queue_put(&subtitleq, pkt);
		} else {
			av_free_packet(pkt);
		}
	}
	bPlaying = false;
	return 1;
}

int CPlayCore::beginDisplay(void *ptr)
{
	CPlayCore *p = (CPlayCore *)ptr;
	p->thread_Display();
	return 0;
}

int CPlayCore::thread_Display(void)
{
	qDebug() << __FUNCTION__;

	renderer = SDL_CreateRenderer(screen, -1, 0);
	if (renderer == nullptr)   {
		LogIns.FlashLog("SDL_CreateRenderer - exiting:%s\n", SDL_GetError());
		return -1;
	}

	int got_picture, fCount,res;
	double frame_delay;
	AVFrame *pFrame, *pFrameYUV;
	uint8_t *out_buffer = NULL;
	SDL_Texture *sdlTexture = NULL;
	AVPacket pkt = { 0 };

	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();

	double basicDelay = av_q2d(pCodecCtx->time_base) * pCodecCtx->ticks_per_frame;
	
	//注意：因为out_buffer会组合到pFrameYUV中，释放pFrameYUV会连带释放！！后面若单独释放可能会出错。
	out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	sdlTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_POINT, NULL, NULL, NULL);

	INT64 drift, NextPts, PrePts, Delay;
	NextPts = PrePts = video_st->start_time;
	fCount = 0;
	while (!bStop)
	{
		bPlaying = true;	//监测信号量
		if (bPause) {
			SDL_Delay(1000);
			continue;
		}

		av_free_packet(&pkt);
		if (packet_queue_get(&videoq, &pkt, 1, 0) < 0)
			break;

		if (pkt.data == flush_pkt.data) {
			avcodec_flush_buffers(video_st->codec);
			continue;
		}

		res = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &pkt);
		if ( res < 0) {
			RecordError("NormalMedia", "avcodec_decode_video2", res);
			continue;
		}

		if (got_picture)	{
			sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
			SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
			SDL_RenderPresent(renderer);

// 			if (pFrame->pkt_pts != AV_NOPTS_VALUE) {
// 				ctrl->mo->Time_R = INT64((pFrame->pkt_pts - is->video_st->start_time) * 1000 * av_q2d(is->video_st->time_base));
// 			}
// 			else {
// 				ctrl->mo->Time_R = is->video_clock;
// 			}
// 			if (is->video_start_pos <= 0)	{
// 				is->video_start_pos = pkt.pos;
// 			}
// 			is->video_current_pos = pkt.pos;
		}

		// update video clock for next frame 
		if (pFrame->pkt_pts != AV_NOPTS_VALUE) {
			drift = pFrame->pkt_pts - NextPts;
			NextPts = (pFrame->pkt_pts - PrePts) + pFrame->pkt_pts;
			Delay = NextPts - pFrame->pkt_pts - drift;
			if (Delay < 0)
				Delay = 0;
			frame_delay = Delay * 1000 * av_q2d(video_st->time_base);		//N * AV_TIME_BASE / 1000 == N *1000,单位ms
			PrePts = pFrame->pkt_pts;
		}
		else {
			frame_delay = basicDelay + pFrame->repeat_pict * (basicDelay * 0.5);
			frame_delay *= 1000;
		}
		frame_delay = FFMIN(frame_delay, 200);
//		video_clock += (INT64)frame_delay;					//累加延时用于显示已播放时长及某些duration不可知文件的seek功能。
//		if ((ctrl->speed.num / ctrl->speed.den) >= 4) {
// 			//大于4倍（含）速度播放时，直接隔帧跳过。
// 			++fCount;
// 			if (fCount % (ctrl->speed.num / ctrl->speed.den) != 0)	{
// 				continue;
// 			}
// 			fCount = 0;
// 		}
// 		else {
// 			//其他速度播放，更改延时。
// 			frame_delay *= 1.0 * ctrl->speed.den / ctrl->speed.num;
// 		}
		if (video_st->codec->codec_id == AV_CODEC_ID_H264)
			frame_delay = FFMAX(frame_delay, 10);
		SDL_Delay(frame_delay);
	}
	bPlaying = false;
	return 1;
}

int CPlayCore::beginEvent(void *ptr)
{
	return 0;
	CPlayCore *p = (CPlayCore *)ptr;
	p->thread_Event();
	return 0;
}

int CPlayCore::thread_Event(void)
{
	qDebug() << __FUNCTION__;

	SDL_Event event;
	double remaining_time = 0.0;
	while (bStop)
	{
		SDL_PumpEvents();
		while (!SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
// 			if (!cursor_hidden && av_gettime_relative() - cursor_last_shown > CURSOR_HIDE_DELAY) {
// 				SDL_ShowCursor(0);
// 				cursor_hidden = 1;
// 			}
			if (remaining_time > 0.0)
				av_usleep((int64_t)(remaining_time * 1000000.0));
			remaining_time = REFRESH_RATE;
			if (!bPause)
				video_refresh(&remaining_time);
			SDL_PumpEvents();
		}
	}
	return 1;
}

void CPlayCore::video_refresh(double *remaining_time)
{
	UNREFERENCED_PARAMETER(remaining_time);

}

int CPlayCore::stream_video_open()
{
	AVCodec *pCodec;
	int res;

	packet_queue_start(&videoq);

	video_st = ic->streams[video_stream];
	pCodecCtx = video_st->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		RecordError("NormalMedia", "avcodec_find_decoder");
		return -1;
	}
	res = avcodec_open2(pCodecCtx, pCodec, NULL);
	if (res < 0)    {
		RecordError("NormalMedia", "avcodec_open2", res);
		return -1;
	}

	return 0;
}

int CPlayCore::stream_audio_open()
{
	packet_queue_start(&audioq);

	audio_st = ic->streams[audio_stream];

	return 0;
}

int CPlayCore::stream_subtitle_open()
{
	packet_queue_start(&subtitleq);
	
	return 0;
}

int CPlayCore::play(const char *url)
{	
	int res;

	screen = SDL_CreateWindowFrom(wnd);
	if (screen == nullptr){
		LogIns.FlashLog("SDL_CreateWindow - exiting:%s\n", SDL_GetError());
		return -1;
	}

	//SDL_DestoryWindow函数会隐藏窗口，为程序可重入，需要加ShowWindow调用。
	SDL_ShowWindow(screen);

	//为简化后续的处理，无论当前文件含有几种流，均初始化全部的队列。
	packet_queue_init(&audioq);
	packet_queue_init(&videoq);
	packet_queue_init(&subtitleq);

	av_init_packet(&flush_pkt);
	flush_pkt.data = (uint8_t *)&flush_pkt;

	ic = avformat_alloc_context();
	ic->interrupt_callback.callback = decode_interrupt_cb;
	ic->interrupt_callback.opaque = this;

	res = avformat_open_input(&ic, url, NULL, NULL);
	if (res != 0){
		RecordError("NormalMedia", "avformat_open_input", res);
		return -1;
	}
	av_format_inject_global_side_data(ic);

	res = avformat_find_stream_info(ic, NULL);
	if (res < 0) {
		RecordError("NormalMedia", "avformat_find_stream_info", res);
		return -1;
	}
	if (ic->pb)
		ic->pb->eof_reached = 0; // FIXME hack, ffplay maybe should not use avio_feof() to test for the end

	video_stream = audio_stream = subtitle_stream = -1;
	if (!video_disable) {
		video_stream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
		if (video_stream < 0){
			RecordError("NormalMedia", "avformat_find_stream_info", res);
			RaiseException(0xE0000001, 0, 0, 0);
		} else {
			stream_video_open();
		}
	}

	if (!audio_disable) {
		audio_stream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, video_stream, NULL, 0);
		if (audio_stream > 0){
			stream_audio_open();
		}
	}

	if (!video_disable && !subtitle_disable) {
		subtitle_stream = av_find_best_stream(ic, AVMEDIA_TYPE_SUBTITLE, -1,
										(audio_stream >= 0 ? audio_stream : video_stream), NULL, 0);
		if (subtitle_stream > 0) {
			stream_subtitle_open();
		}
	}

	if (video_stream < 0 && audio_stream < 0)
	{
		RecordError("NormalMedia", "Could not find any valid-stream.");
		return -1;
	}

	continue_read_thread = SDL_CreateCond();

	Read_tid = SDL_CreateThread(beginRead, "ReadThread", this);
	Display_tid = SDL_CreateThread(beginDisplay, "DispalyThread", this);
	Event_tid = SDL_CreateThread(beginEvent, "EventThread", this);
	return 1;
}

void CPlayCore::stop(void)
{
	bStop = !bStop;
	bPause = false;

	SDL_WaitThread(Read_tid, NULL);
	SDL_WaitThread(Display_tid, NULL);
}

void CPlayCore::pause(void)
{
	if (bPause)
	{
		bPause = false;
		av_read_play(ic);
	}
	else
	{
		bPause = true;
		av_read_pause(ic);
	}
}

int CPlayCore::setMute(bool Mute)
{
	bMute = Mute;

	return 0;
}