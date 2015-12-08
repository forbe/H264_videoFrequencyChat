#include "decodeh264.h"

AVCodec *m_pVideoCodec;
AVCodecContext *m_pVideoCodecCtx = NULL;

AVFrame *m_pFrameYuv;
AVFrame  *m_pFrameRgb;
AVPacket avpkt;

int m_nWidth, m_nHeight;

unsigned char *m_pRgbBuf;
int m_pRgbSize = 0;
int dataflag = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct SwsContext *sws_ctx;

int initDecoder264(int w, int h) {

	m_nWidth = w;
	m_nHeight = h;

	//解码和编码只需调用一次
	avcodec_register_all();
	m_pFrameYuv = av_frame_alloc();
	av_init_packet(&avpkt);

	m_pVideoCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!m_pVideoCodec) {
		return -1;
	}

	m_pVideoCodecCtx = avcodec_alloc_context3(m_pVideoCodec);
	if (!m_pVideoCodecCtx) {
		return -1;
	}

	if (m_pVideoCodec->capabilities & CODEC_CAP_TRUNCATED) {
		m_pVideoCodecCtx->flags |= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */
	}

//	m_pVideoCodecCtx->codec_id = codec_id;
//	m_pVideoCodecCtx->width = w;
//	m_pVideoCodecCtx->height = h;

	// CBR（固定码率控制）的设置
//	m_pVideoCodecCtx->bit_rate = bitrate;
//	m_pVideoCodecCtx->rc_min_rate = bitrate;	//
//	m_pVideoCodecCtx->rc_max_rate = bitrate;	//
//	m_pVideoCodecCtx->bit_rate_tolerance = bitrate;	//
//	m_pVideoCodecCtx->rc_buffer_size = bitrate;	//
//	m_pVideoCodecCtx->rc_initial_buffer_occupancy = m_pVideoCodecCtx->rc_buffer_size * 3 / 4;	//

	// 其他设置
	m_pVideoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	m_pVideoCodecCtx->ticks_per_frame = 2; //

	av_opt_set(m_pVideoCodecCtx->priv_data, "preset", "superfast", 0);	// 速率设置
	//	av_opt_set((*c)->priv_data, "preset", "fast", 0);
	// av_opt_set((*c)->priv_data, "profile", "baseline", 0);
	// av_opt_set((*c)->priv_data, "preset", "slow", 0);
	av_opt_set(m_pVideoCodecCtx->priv_data, "tune", "zerolatency", 0);	// 实时设置

	/* open it */
	if (avcodec_open2(m_pVideoCodecCtx, m_pVideoCodec, NULL) < 0) {
		return -1;
	}

	pthread_mutex_init(&mutex, NULL);

	return 0;
}

int CheckSws() {
	if (m_pVideoCodecCtx->width == 0 || m_pVideoCodecCtx->height == 0
			|| m_pVideoCodecCtx->pix_fmt == AV_PIX_FMT_NONE) {
		return -4;
	}

	if (m_pFrameRgb == NULL) {
		m_pFrameRgb = av_frame_alloc();
		if (m_pFrameRgb == NULL) {
			return -5;
		}
	}

	if (m_pRgbBuf == NULL) {
		m_pRgbBuf = malloc(m_nWidth * m_nHeight * 3);
		if (m_pRgbBuf == NULL) {
			return -6;
		}
	}

	if (sws_ctx == NULL) {
		sws_ctx = sws_getContext(m_nWidth, m_nHeight, m_pVideoCodecCtx->pix_fmt,
				m_nWidth, m_nHeight, PIX_FMT_RGB565, SWS_BILINEAR, NULL, NULL,
				NULL);

		if (sws_ctx == NULL) {
			return -7;
		}

		if (avpicture_fill((AVPicture*) m_pFrameRgb, m_pRgbBuf, PIX_FMT_RGB565,
				m_nWidth, m_nHeight) < 0) {
			return -8;
		}
	}
	return 0;
}

int decoder264(unsigned char *data, int size) {

	avpkt.data = data;
	avpkt.size = size;

	if (avpkt.size <= 0) {
		return -1;
	}

	int len, got_picture = 0;

	len = avcodec_decode_video2(m_pVideoCodecCtx, m_pFrameYuv, &got_picture, &avpkt);

	if (len < 0) {
		return -2;
	}

	if (got_picture) {

		if (CheckSws() < 0) {
			return -3;
		}

		pthread_mutex_lock(&mutex);
		sws_scale(sws_ctx, (const uint8_t * const *) m_pFrameYuv->data,
				m_pFrameYuv->linesize, 0, m_nHeight, m_pFrameRgb->data,
				m_pFrameRgb->linesize);
		m_pRgbSize = m_pFrameRgb->linesize[0] * m_nHeight;
		pthread_mutex_unlock(&mutex);

		dataflag = 1;
	}

	if (avpkt.data) {
		avpkt.size -= len;
		avpkt.data += len;
	}

	return got_picture;
}

int closeDecoder264() {
	avcodec_close(m_pVideoCodecCtx);
	av_free(m_pVideoCodecCtx);
	av_frame_free(&m_pFrameYuv);
	av_frame_free(&m_pFrameRgb);
	av_free_packet(&avpkt);
	free(m_pRgbBuf);

	pthread_mutex_destroy(&mutex);

	return 0;
}

int Render(unsigned char *data) {
	if (dataflag == 0) {
		return -1;
	}

	/*互斥锁上锁*/
	pthread_mutex_lock(&mutex);
	memcpy(data, m_pRgbBuf, m_pRgbSize);
	/*互斥锁接锁*/
	pthread_mutex_unlock(&mutex);

	dataflag = 0;

	return m_pRgbSize;
}

/**
 * 初始化解码器
 *
 * w 解码H264的帧宽
 * h 解码H264的帧高
 */
JNIEXPORT jint Java_com_jni_ffmpeg_X264Decoder_initDecoder264(JNIEnv * env,
		jobject jobj, jint w, jint h) {
	return initDecoder264(w, h);
}

/**
 * 解码器解码数据
 *
 * data H264数组
 * size 数组长度
 */
JNIEXPORT jint Java_com_jni_ffmpeg_X264Decoder_decoder264(JNIEnv *env, jobject jobj,
		jbyteArray data, jint size) {

	unsigned char array[size * 2];
	(*env)->GetByteArrayRegion(env, data, 0, size, array);
	int ret = decoder264(array, size);
	return ret;
}

/**
 * 释放解码器
 */
JNIEXPORT jint Java_com_jni_ffmpeg_X264Decoder_closeDecoder264(JNIEnv * env,
		jobject jobj) {
	return closeDecoder264();
}

/**
 * 读取将H264解码后的RGB565数值
 */
JNIEXPORT jint Java_com_jni_ffmpeg_X264Decoder_render(JNIEnv *env, jobject jobj,
		jbyteArray data) {
	unsigned char *dataBuf = (*env)->GetByteArrayElements(env, data, 0);

	int size = Render(dataBuf);

	(*env)->ReleaseByteArrayElements(env, data, dataBuf, 0);

	return size;
}
