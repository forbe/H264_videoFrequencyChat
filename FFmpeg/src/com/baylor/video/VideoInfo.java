package com.baylor.video;

public class VideoInfo {
	
	public static final int WIDTH = 320;
	public static final int HEIGHT = 240;
	public static final int FPS = 20;
	
	public static final int VERY_LOW_BITRATE    = WIDTH * HEIGHT * 3 / 4;	// 极低码率
	public static final int LOW_BITRATE         = WIDTH * HEIGHT * 3 / 2;	// 低码率
	public static final int MIDDLE_BITRATE      = WIDTH * HEIGHT * 3;		// 中码率
	public static final int HEIGHT_BITRATE      = WIDTH * HEIGHT * 3 * 2;	// 高码率
	public static final int VERY_HEIGHT_BITRATE = WIDTH * HEIGHT * 3 * 4;	// 极高码率
	
}
