package com.baylor.video;

import com.jni.rtp.RTPInfo;
import com.jni.rtp.RTPSession;

import android.util.Log;
import android.view.SurfaceView;

public class VideoManage {
	
	private SurfaceView mSufacerRecoder;
	private SurfaceView mSufacerPlay;
	
	private boolean isRecoder = false;
	private boolean isPlayer = false;
	
	private VideoRecoder mVideoRecoder;
	private VideoPlayer mVideoPlayer;
	
	private RTPSession mRtpSession;
	
	public VideoManage(SurfaceView SurfaceView1, SurfaceView SurfaceView2, boolean isRecoder, boolean isPlayer){
		mSufacerRecoder = SurfaceView1;
		mSufacerPlay = SurfaceView2;
		
		this.isRecoder = isRecoder;
		this.isPlayer = isPlayer;
	}
	
	public void open() {
		
		// 初始化RTPSession
		if(mRtpSession != null) {
			mRtpSession.uninitRtp();
			mRtpSession = null;
		}
		
		mRtpSession = new RTPSession();
		mRtpSession.initRtp(RTPInfo.IP, RTPInfo.DEST_PORT, RTPInfo.PORT_BASE, RTPInfo.FRAME_RATE);
		
		// 开始Recoder
		if(isRecoder && mSufacerRecoder != null) {
			if (mVideoRecoder != null) {
				mVideoRecoder.CloseEncoder();
				mVideoRecoder = null;
			}
			
//			mSufacerRecoder.setZOrderMediaOverlay(true);
//			mSufacerRecoder.getHolder().setFormat(PixelFormat.TRANSPARENT);
			
			mVideoRecoder = new VideoRecoder();
			int ret = mVideoRecoder.OpenEncoder(mSufacerRecoder, mRtpSession, VideoInfo.WIDTH, VideoInfo.HEIGHT, VideoInfo.HEIGHT_BITRATE, VideoInfo.FPS);
			if (ret < 0) {
				Log.d("ktian", "ffmpeg 软编接口调用失败");
			} else {
				Log.d("ktian", "ffmpeg 软编接口调用成功");
			}
		}
		
		// 开始Player
		if(isPlayer && mSufacerPlay != null) {
			if(mVideoPlayer != null) {
				mVideoPlayer.CloseDecoder();
				mVideoPlayer = null;
			}
			
//			mSufacerPlay.setZOrderMediaOverlay(true);
//			mSufacerPlay.getHolder().setFormat(PixelFormat.TRANSPARENT);
			
			mVideoPlayer = new VideoPlayer();
			int ret = mVideoPlayer.OpenDecoder(mSufacerPlay, mRtpSession, VideoInfo.WIDTH, VideoInfo.HEIGHT);
			if (ret < 0) {
				Log.d("ktian", "ffmpeg 软解码接口调用失败");
			} else {
				Log.d("ktian", "ffmpeg 软解码接口调用成功");
			}
		}
	}
	
	public void close() {
		if (mVideoRecoder != null) {
			mVideoRecoder.CloseEncoder();
			mVideoRecoder = null;
		}
		
		if(mVideoPlayer != null) {
			mVideoPlayer.CloseDecoder();
			mVideoPlayer = null;
		}
		
		if(mRtpSession != null) {
			mRtpSession.uninitRtp();
			mRtpSession = null;
		}
	}

}
