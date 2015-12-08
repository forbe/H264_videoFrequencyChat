package com.jni.ffmpeg;

public class X264Encoder {

	static {
		System.loadLibrary("encoder");
	}

	/******************** h264编码 **************************/
	public static native int initEncoder264(int w, int h, long bitrate, int fps);

	public static native int encoder264(byte[] yuv, byte[] h264);

	public static native int closeEncoder264();

}
