#include <jni.h>

#include <pthread.h>
#include <android/bitmap.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 将Bitmap对象中填充RGB565内容
 *
 *	dataRgb rgb565数组
 *	length 数组长度
 *	bitmap 被填充的对象
 */
JNIEXPORT jint Java_com_jni_bitmap_AndroidBitmapInfo_fillRgb565(JNIEnv *env, jobject jobj,
		jbyteArray dataRgb, jint length, jobject bitmap);


#ifdef __cplusplus
}
#endif
