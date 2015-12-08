#include "android_bitmap_info.h"

/**
 * 将Bitmap对象中填充RGB565内容
 *
 *	dataRgb rgb565数组
 *	length 数组长度
 *	bitmap 被填充的对象
 */
JNIEXPORT jint Java_com_jni_bitmap_AndroidBitmapInfo_fillRgb565(JNIEnv *env, jobject jobj,
		jbyteArray dataRgb, jint length, jobject bitmap) {

	unsigned char *rgbBuf = (unsigned char *) env->GetByteArrayElements(dataRgb,
			NULL);

	AndroidBitmapInfo info;
	void* pixels;

	if (AndroidBitmap_getInfo(env, bitmap, &info) < 0) {
//		__android_log_print(ANDROID_LOG_INFO, "JNIMsg", "AndroidBitmap_getInfo error");
		return -1;
	}

	if (info.format != ANDROID_BITMAP_FORMAT_RGB_565) {
//		__android_log_print(ANDROID_LOG_INFO, "JNIMsg", "ANDROID_BITMAP_FORMAT_RGB_565 error");
		return -1;
	}

	if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) {
//		__android_log_print(ANDROID_LOG_INFO, "JNIMsg", "AndroidBitmap_lockPixels error");
		return -1;
	}

	memcpy(pixels, rgbBuf, length);

	env->ReleaseByteArrayElements(dataRgb, (jbyte*) rgbBuf, NULL);

	AndroidBitmap_unlockPixels(env, bitmap);
	return 0;
}

