#include "encodeh264.h"

AVCodecContext *m_pVideoCodecCtx = NULL;
AVFrame *frame;
AVPacket pkt;
int width = 320;
int height = 240;
unsigned char *yuvSeparate[3];

void separateYUYVbuf(unsigned char *Srcdata, int width, int height,
		unsigned char *Y, unsigned char *U, unsigned char *V) {
	unsigned long int i = 0, j = 0, k = 0;
	unsigned long int size = width * height * 2;

	while (j < size) {
		Y[i] = Srcdata[j];
		i++;
		j = j + 2;

		if (!(j < size))
			break;

		Y[i] = Srcdata[j];
		i++;

		U[k] = Srcdata[j - 1];
		V[k] = Srcdata[j + 1];
		k++;
		j = j + 2;
	}
}

void separate_yuv_nv21(unsigned char *nv21, int width, int height,
		unsigned char *Y, unsigned char *U, unsigned char *V) {

	int i = 0, j = 0, k = 0;
	int ysize = width * height;
	int vsize = ysize / 4;

	memcpy(Y, nv21, ysize);

	while (j < vsize) {
		V[j] = nv21[ysize + i];
		i++;
		U[k] = nv21[ysize + i];
		i++;
		j++;
		k++;
	}

}

void separate_yuv_I420(unsigned char *I420, int width, int height,
		unsigned char *Y, unsigned char *U, unsigned char *V) {

	int ysize = width * height;
	int vsize = ysize / 4;

	memcpy(Y, I420, ysize);
	memcpy(U, I420 + ysize, vsize);
	memcpy(V, I420 + ysize + vsize, vsize);
}

int video_encode_init(int codec_id, AVCodecContext **c, AVFrame **frame,
		int w, int h, unsigned long int bitrate, int fps, int gop) {
	AVCodec *codec;
	int ret;

	/* register all the codecs */
	avcodec_register_all();

	/* find the mpeg1 video encoder */
	codec = avcodec_find_encoder(codec_id);
	if (!codec) {
//		__android_log_print(ANDROID_LOG_INFO, "ktian", "Codec not found");
		return -1;
	}

	(*c) = avcodec_alloc_context3(codec);
	if (!(*c)) {
//		__android_log_print(ANDROID_LOG_INFO, "ktian", "Could not allocate video codec context");
		return -1;
	}

	// 设置宽、高和编码格式
//	(*c)->codec_type = AVMEDIA_TYPE_VIDEO; //
	(*c)->codec_id = codec_id;
	(*c)->width = w;
	(*c)->height = h;

	// CBR（固定码率控制）的设置
	(*c)->bit_rate = bitrate;
	(*c)->rc_min_rate = bitrate;	//
	(*c)->rc_max_rate = bitrate;	//
	(*c)->bit_rate_tolerance = bitrate;	//
	(*c)->rc_buffer_size = bitrate;	//
	(*c)->rc_initial_buffer_occupancy = (*c)->rc_buffer_size * 3 / 4;	//

	// 其他设置
	(*c)->time_base = (AVRational) {1,fps};
	(*c)->gop_size = gop;
	(*c)->max_b_frames = 1;
	(*c)->b_frame_strategy = 0;
	(*c)->pix_fmt = AV_PIX_FMT_YUV420P;
	(*c)->ticks_per_frame = 2; //

	av_opt_set((*c)->priv_data, "preset", "superfast", 0);	// 编码速率设置
	//	av_opt_set((*c)->priv_data, "preset", "fast", 0);
	// av_opt_set((*c)->priv_data, "profile", "baseline", 0);
	// av_opt_set((*c)->priv_data, "preset", "slow", 0);
	av_opt_set((*c)->priv_data, "tune", "zerolatency", 0);	// 实时编码设置

	/* open it */
	if (avcodec_open2((*c), codec, NULL) < 0) {
//		__android_log_print(ANDROID_LOG_INFO, "ktian", "Could not open codec");
		return -1;
	}

	(*frame) = av_frame_alloc();
	if (!(*frame)) {
//		__android_log_print(ANDROID_LOG_INFO, "ktian", "Could not allocate video (*frame)");
		return -1;
	}

	(*frame)->format = (*c)->pix_fmt;
	(*frame)->width = (*c)->width;
	(*frame)->height = (*c)->height;
	(*frame)->pts = 0;

	yuvSeparate[0] = (unsigned char*)malloc(w*h);
	yuvSeparate[1] = (unsigned char*)malloc(w*h/2);
	yuvSeparate[2] = (unsigned char*)malloc(w*h/2);

	if ((NULL == yuvSeparate[0]) || (NULL == yuvSeparate[1])
			|| (NULL == yuvSeparate[2])) {
//		__android_log_print(ANDROID_LOG_INFO, "ktian", "yuvSeparate malloc fail!");
		return -1;
	}

	/* the image can be allocated by any means and av_image_alloc() is
	 * just the most convenient way if av_malloc() is to be used */
	ret = av_image_alloc((*frame)->data, (*frame)->linesize, (*c)->width,
			(*c)->height, (*c)->pix_fmt, 32);
	if (ret < 0) {
//		__android_log_print(ANDROID_LOG_INFO, "ktian", "Could not allocate raw picture buffer");
		return -1;
	}

	return 0;
}


int video_encode(AVCodecContext **c, AVFrame **frame, AVPacket pkt,
		unsigned char *yuvdata, int w, int h, unsigned char **h264data,
		unsigned int * h264size) {

	int i, got_output=0;

	separate_yuv_nv21(yuvdata, w, h, yuvSeparate[0],yuvSeparate[1],yuvSeparate[2]);
	/* encode 1 second of video */
	(*frame)->data[0] = yuvSeparate[0];
	(*frame)->data[1] = yuvSeparate[1];
	(*frame)->data[2] = yuvSeparate[2];

	av_init_packet(&pkt);
	pkt.data = NULL; // packet data will be allocated by the encoder
	pkt.size = 0;

	fflush(stdout);

	/* encode the image */
	avcodec_encode_video2((*c), &pkt, (*frame), &got_output);

//	if(ret == 0) {
		if (got_output) {
			memcpy((*h264data), pkt.data, pkt.size);
			*h264size = pkt.size;
		}
//	}

	av_free_packet(&pkt);

	(*frame)->pts++;
	return got_output;
}

int video_encode_uninit(AVCodecContext **c, AVFrame **frame) {
	avcodec_close((*c));
	av_free((*c));
	av_frame_free(frame);
	free(yuvSeparate[0]);
	free(yuvSeparate[1]);
	free(yuvSeparate[2]);

	return 0;
}


/**
 * 初始化编码器
 */
jint Java_com_jni_ffmpeg_X264Encoder_initEncoder264(JNIEnv * env, jobject jobj, jint w,
		jint h, jlong bitrate, jint fps) {
	width = w;
	height = h;
	if (video_encode_init(AV_CODEC_ID_H264, &m_pVideoCodecCtx, &frame, width, height, bitrate, fps, 10) == 0) {
		// video_encode_init success!
		return 0;
	}

	return -1;
}

/**
 * 帧编码
 */
jint Java_com_jni_ffmpeg_X264Encoder_encoder264(JNIEnv * env, jobject jobj,
		jbyteArray yuv, jbyteArray h264) {

	unsigned char *yuvBuf = (*env)->GetByteArrayElements(env, yuv, NULL);
	unsigned char *h264Buf = (*env)->GetByteArrayElements(env, h264, NULL);

	int h264size = 0;
	int ret = video_encode(&m_pVideoCodecCtx, &frame, pkt, yuvBuf, width, height, &h264Buf, &h264size);

	(*env)->ReleaseByteArrayElements(env, h264, h264Buf, 0);
	(*env)->ReleaseByteArrayElements(env, yuv, yuvBuf, 0);

	return h264size;
}

/**
 * 关闭编码器
 */
jint Java_com_jni_ffmpeg_X264Encoder_closeEncoder264(JNIEnv *env, jobject jobj) {
	return video_encode_uninit(&m_pVideoCodecCtx, &frame);
}
