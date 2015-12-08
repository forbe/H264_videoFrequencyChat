#include <jni.h>

#include "rtpmanage.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * RTP发送器初始化
 */
JNIEXPORT jint Java_com_jni_rtp_RTPSession_initRtp(JNIEnv *env, jobject jobj, jstring ip, jint destport, jint baseport, jdouble framerate);

/**
 * RTP发送销毁
 */
JNIEXPORT void Java_com_jni_rtp_RTPSession_uninitRtp(JNIEnv *env, jobject jobj);

/**
 * RTP发送
 */
JNIEXPORT jint Java_com_jni_rtp_RTPSession_sendRtp(JNIEnv *env, jobject jobj, jbyteArray h264data, jint length);

/**
 * RTP接收
 */
JNIEXPORT void Java_com_jni_rtp_RTPSession_receiveRtp(JNIEnv *env, jobject jobj);

/**
 * RTP组包好的数据读取
 */
JNIEXPORT int Java_com_jni_rtp_RTPSession_rendRtp(JNIEnv *env, jobject jobj, jbyteArray h264data);

#ifdef __cplusplus
}
#endif
