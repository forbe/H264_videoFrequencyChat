package com.jni.bitmap;

/**
 * rgb数据填充到bitmap对象中
 * 
 * @author kTian
 *
 */
public class AndroidBitmapInfo {
	
	static {
		System.loadLibrary("bitmap");
	}
	
	public static native int fillRgb565(byte[] dataRgb, int length, Object bitmap);

}
