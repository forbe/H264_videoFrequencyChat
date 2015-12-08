#include <jni.h>
#include <math.h>
#include <android/log.h>

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>


#ifdef __cplusplus
extern "C" {
#endif

/********************************** 编码 **********************************************/
jint Java_com_jni_ffmpeg_X264Encoder_initEncoder264(JNIEnv *, jobject, jint w, jint h,
		jlong bitrate, jint fps);

jint Java_com_jni_ffmpeg_X264Encoder_encoder264(JNIEnv *, jobject, jbyteArray yuv,
		jbyteArray h264);

jint Java_com_jni_ffmpeg_X264Encoder_closeEncoder264(JNIEnv *, jobject);


#ifdef __cplusplus
}
#endif

