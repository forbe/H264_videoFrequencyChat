#include <math.h>
#include <string.h>
#include <jni.h>
#include <android/log.h>

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"

#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/parseutils.h"


#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint Java_com_jni_ffmpeg_X264Decoder_initDecoder264(JNIEnv *, jobject, jint w, jint h);

JNIEXPORT jint Java_com_jni_ffmpeg_X264Decoder_decoder264(JNIEnv *, jobject, jbyteArray data, jint size);

JNIEXPORT jint Java_com_jni_ffmpeg_X264Decoder_render(JNIEnv *, jobject, jbyteArray data);

JNIEXPORT jint Java_com_jni_ffmpeg_X264Decoder_closeDecoder264(JNIEnv *, jobject);

//JNIEXPORT jint Java_com_jni_X264Decoder_renderBitmap(JNIEnv *, jobject, jobject bitmap);

#ifdef __cplusplus
}
#endif

