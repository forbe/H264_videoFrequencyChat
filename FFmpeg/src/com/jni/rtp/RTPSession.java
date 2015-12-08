package com.jni.rtp;

/**
 * RTP发送和接收操作类
 * 
 * @author kTian
 *
 */
public class RTPSession {
	
	static {
		System.loadLibrary("jrtp");
	}

	public native int initRtp(String ip, int destport, int portbase, double framerate);
	
	public native void uninitRtp();
	
	/********************************** RTP发送 ************************************/
	public native int sendRtp(byte[] h264, int length);

	
	/********************************** RTP接收 ************************************/
	public native void receiveRtp();
	
	public native int rendRtp(byte[] data);
}
