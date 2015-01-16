#include "PlayCore.h"
#include "Log.h"

CPlayCore::CPlayCore(HWND w)
:wnd(w)
{
}


CPlayCore::~CPlayCore()
{
}

int CPlayCore::InitEnv(void)
{
//	avdevice_register_all();
	av_register_all();
	avformat_network_init();

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		LogIns.FlashLog("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}

	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);

	return 0;
}

void CPlayCore::RecordError(const char *MediaName, const char *action, int error = 0)
{
	char errbuff[100];
	av_make_error_string(errbuff, 100, error);
	fprintf_s(stderr, "%s %s. error info:%s\n", MediaName, action, errbuff);
}

int CPlayCore::play(const char *url)
{
	AVCodec         *pCodec;
	AVInputFormat *ifmt;
	int res, video_st;
	AVFormatContext *pFormatCtx = NULL;
	AVCodecContext  *pCodecCtx = NULL;
	SwsContext *img_convert_ctx = NULL;
	AVPacket *packet = NULL;
	AVFrame *pFrame = NULL, *pFrameYUV = NULL;
	SDL_Window *screen = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *texture = NULL;

	screen = SDL_CreateWindowFrom(wnd);
	if (screen == 0){
		printf_s("SDL_CreateWindow - exiting:%s\n", SDL_GetError());
		goto The_end;
	}

	renderer = SDL_CreateRenderer(screen, -1, 0);
	if (renderer == NULL)   {
		printf_s("SDL_CreateRenderer - exiting:%s\n", SDL_GetError());
		goto The_end;
	}

	pFormatCtx = avformat_alloc_context();

	res = avformat_open_input(&pFormatCtx, url, NULL, NULL);

	if (res != 0){
		RecordError("NormalMedia", "avformat_open_input", res);
		goto The_end;
	}

	res = avformat_find_stream_info(pFormatCtx, NULL);
	if (res < 0) {
		RecordError("NormalMedia", "avformat_find_stream_info", res);
		goto The_end;
	}

	video_st = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (video_st < 0) {
		RecordError("NormalMedia", "av_find_best_stream", video_st);
		goto The_end;
	}

	av_dump_format(pFormatCtx, video_st, pFormatCtx->filename, 0);

	pCodecCtx = pFormatCtx->streams[video_st]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		RecordError("NormalMedia", "avcodec_find_decoder");
		goto The_end;
	}

	res = avcodec_open2(pCodecCtx, pCodec, NULL);
	if (res < 0)    {
		RecordError("NormalMedia", "avcodec_open2", res);
		goto The_end;
	}

	int DispW = pCodecCtx->width;
	int DispH = pCodecCtx->height;

	packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();
	uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, DispW, DispH));
	if (packet == NULL || pFrame == NULL || pFrameYUV == NULL || out_buffer == NULL) {
		RecordError("NormalMedia", "av_frame_alloc");
		goto The_end;
	}
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, DispW, DispH);

	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, DispW, DispH, PIX_FMT_YUV420P, SWS_POINT, NULL, NULL, NULL);
	if (img_convert_ctx == NULL) {
		RecordError("NormalMedia", "sws_getContext");
		goto The_end;
	}

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, DispW, DispH);
	if (img_convert_ctx == NULL) {
		printf_s("SDL_CreateTexture - exiting:%s\n", SDL_GetError());
		goto The_end;
	}

	int n = 1000;
	int got_picture;
	bool bPaused = false;
	while (--n)
	{
		res = av_read_frame(pFormatCtx, packet);
		if (res < 0){
			RecordError("NormalMedia", "av_read_frame", res);
			continue;
		}

		if (packet->stream_index == video_st){
			res = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if (res < 0) {
				RecordError("NormalMedia", "avcodec_decode_video2", res);
				continue;
			}

			if (got_picture)    {
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
				SDL_UpdateTexture(texture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
				SDL_RenderClear(renderer);
				SDL_RenderCopy(renderer, texture, NULL, NULL);
				SDL_RenderPresent(renderer);
			}
			SDL_Delay(40);
		}
		av_free_packet(packet);
	}

The_end:
	avcodec_close(pCodecCtx);
	if (pFormatCtx != NULL)
		avformat_close_input(&pFormatCtx);

	av_free_packet(packet);
	av_frame_free(&pFrame);
	av_frame_free(&pFrameYUV);

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(screen);
	sws_freeContext(img_convert_ctx);
	return 1;
}