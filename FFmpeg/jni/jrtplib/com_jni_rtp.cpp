#include "com_jni_rtp.h"

H264RtpFun *h264RtpFun;

// 将jstring转成C++中的string
std::string jstring2str(JNIEnv* env, jstring jstr) {
	char* rtn = NULL;
	jclass clsstring = env->FindClass("java/lang/String");
	jstring strencode = env->NewStringUTF("GB2312");
	jmethodID mid = env->GetMethodID(clsstring, "getBytes",
			"(Ljava/lang/String;)[B");
	jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
	jsize alen = env->GetArrayLength(barr);
	jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
	if (alen > 0) {
		rtn = (char*) malloc(alen + 1);
		memcpy(rtn, ba, alen);
		rtn[alen] = 0;
	}
	env->ReleaseByteArrayElements(barr, ba, 0);
	std::string stemp(rtn);
	free(rtn);
	return stemp;
}

/**
 * RTP初始化
 */
JNIEXPORT jint Java_com_jni_rtp_RTPSession_initRtp(JNIEnv *env, jobject jobj, jstring ip,
		jint destport, jint portbase, jdouble framerate) {
	if(h264RtpFun == NULL) {
		h264RtpFun = new H264RtpFun();
	}
	string i = jstring2str(env, ip);
	return h264RtpFun->rtp_params_init(i, destport, portbase, framerate);
}

/**
 * RTP销毁
 */
JNIEXPORT void Java_com_jni_rtp_RTPSession_uninitRtp(JNIEnv *env, jobject jobj) {
	h264RtpFun->rtp_params_uninit();
	h264RtpFun = NULL;
}

/**
 * RTP发送
 */
JNIEXPORT jint Java_com_jni_rtp_RTPSession_sendRtp(JNIEnv *env, jobject jobj, jbyteArray h264data, jint length) {

	if(h264data == NULL || length <= 0) {
		return -2;
	}
	unsigned char h264Buf[length];
	env->GetByteArrayRegion(h264data, 0, length, (jbyte*) h264Buf);

	if(h264RtpFun != NULL) {
		h264RtpFun->rtp_send(h264Buf, length);
	}else{
		return -1;
	}

	return 0;
}

/**
 * RTP接收
 */
JNIEXPORT void Java_com_jni_rtp_RTPSession_receiveRtp(JNIEnv *env, jobject jobj) {
	h264RtpFun->rtp_receive();
}

/**
 * RTP组包好的H264码流读取
 */
JNIEXPORT int Java_com_jni_rtp_RTPSession_rendRtp(JNIEnv *env, jobject jobj, jbyteArray data) {
	unsigned char *dataBuf = (unsigned char *) env->GetByteArrayElements(data, NULL);

	int size = h264RtpFun->rtp_read(dataBuf);

	env->ReleaseByteArrayElements(data, (jbyte*) dataBuf, 0);

	return size;
}



