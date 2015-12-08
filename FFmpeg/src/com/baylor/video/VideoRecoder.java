package com.baylor.video;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import com.jni.ffmpeg.X264Encoder;
import com.jni.rtp.RTPSession;

import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.Camera.ErrorCallback;
import android.hardware.Camera.PreviewCallback;
import android.os.Build;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.SurfaceHolder.Callback;

/**
 * 实时帧编码
 * 
 * @author kTian
 * 
 */
public class VideoRecoder implements PreviewCallback, ErrorCallback,
		SurfaceHolder.Callback {
	
	private Lock mLock = null;
	private Camera mCamera = null;
	private SurfaceHolder mSurfaceHolder = null;

	private EncoderThread mEncodeThread = null;
	private boolean isYuvFresh = false;
	private byte[] mCamerabuf = null;

	private byte[] mYuvbuf = null;
	private byte[] mH264buf = null;

	private int mWidth;
	private int mHeight;
	private int mPicturesize = 0;
	
//	private DatagramSocket mDatagramSocket;
	
	private RTPSession mRtpSession;
	
	/**
	 * 开始实时视频获取
	 * 
	 * @param width
	 * @param height
	 * @param bitrate
	 * @param fps
	 * @param surfaceview
	 * @return
	 */
	@SuppressWarnings("deprecation")
	public int OpenEncoder(SurfaceView surfaceview, RTPSession rtpSession, int width, int height, long bitrate, int fps) {
		
		mRtpSession = rtpSession;
		
		mLock = new ReentrantLock();
		mWidth = width;
		mHeight = height;

		mSurfaceHolder = surfaceview.getHolder(); // 绑定SurfaceView，取得SurfaceHolder对象
		mSurfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
		mSurfaceHolder.addCallback((Callback) this);

		// 宽、高非零
		if(mWidth <= 0 || mHeight <= 0) {
			return -1;
		}
		
		// 初始化编码器
		if (X264Encoder.initEncoder264(mWidth, mHeight, bitrate, fps) < 0) {
			return -1;
		}
		
		mPicturesize = mWidth * mHeight * 3 / 2;
		mCamerabuf = new byte[mPicturesize];
		mYuvbuf = new byte[mPicturesize];
		mH264buf = new byte[mPicturesize];
		
//		try {
//			mDatagramSocket = new DatagramSocket();
//		} catch (SocketException e) {
//			e.printStackTrace();
//		}

		if (openCamera() < 0) {
			return -1;
		}
		
		mEncodeThread = new EncoderThread();
		mEncodeThread.start();

		return 0;
	}
	
	/**
	 * 结束实时视频获取
	 */
	public void CloseEncoder() {
		
		if (mCamera != null) {
			closeCamera();
			mCamera = null;
		}
		
		if (mEncodeThread != null) {
			mEncodeThread.stopEncoderThread();
			mEncodeThread = null;
		}

		// 关闭编码器
		X264Encoder.closeEncoder264();
		
		// 关闭RTP发送器
		mRtpSession.uninitRtp();
		
//		mDatagramSocket.close();
//		mDatagramSocket = null;

	}

	// 打开Camera
	private int openCamera() {
		try {
			if (Build.VERSION.SDK_INT > 8) {
				int num = Camera.getNumberOfCameras();
				if (num < 1) {
					return -1;
				}
				// 打开后置摄像头(num-2), 前置摄像头(num-1)
				mCamera = Camera.open(num - 1);
			} else {
				mCamera = Camera.open();
			}
			
			
			mCamera.setPreviewDisplay(mSurfaceHolder);
			
			Camera.Parameters parameters = mCamera.getParameters();
			
			Log.d("ktian", "surfece - w == "+mSurfaceHolder.getSurfaceFrame().width());
			Log.d("ktian", "surfece - h == "+mSurfaceHolder.getSurfaceFrame().height());
			
			parameters.setPreviewSize(mWidth, mHeight);
			parameters.setPictureSize(mWidth, mHeight);
			parameters.setPreviewFormat(ImageFormat.NV21);

			mCamera.setParameters(parameters);
//			mCamera.setDisplayOrientation(90);
			mCamera.addCallbackBuffer(mCamerabuf);
			mCamera.setPreviewCallbackWithBuffer(this);
			mCamera.setErrorCallback(this);
			mCamera.startPreview();
		} catch (Exception e) {
			e.printStackTrace();
			return -1;
		}

		return 0;
	}

	// 关闭Camera
	private void closeCamera() {
		if (mCamera != null) {
			mCamera.setPreviewCallback(null); // ！！这个必须在前，不然退出出错
			mCamera.stopPreview();
			mCamera.release();
			mCamera = null;
		}
	}

	@Override
	public void onError(int error, Camera camera) {
		closeCamera();
		openCamera();
	}

	@Override
	public void onPreviewFrame(byte[] data, Camera camera) {
		mLock.lock();
		System.arraycopy(data, 0, mYuvbuf, 0, mPicturesize);
		isYuvFresh = true;
		mLock.unlock();
		
		mCamera.addCallbackBuffer(data);
	}

	/**
	 * 在线程中调用Native编码
	 * 
	 * @author kTian
	 * 
	 */
	protected class EncoderThread extends Thread {
		
		public void stopEncoderThread() {
			while (true) {
				if (this.isAlive()) {
					this.interrupt();
				} else {
					break;
				}
			}
		}

		@Override
		public void run() {

			while (!this.isInterrupted()) {
				mLock.lock();
				
				if (isYuvFresh == false) {
					mLock.unlock();
					continue;
				} else {
					isYuvFresh = false;
				}

				int h264Size = X264Encoder.encoder264(mYuvbuf, mH264buf);
				if (h264Size > 0 && mH264buf != null) {
					
//					Log.d("ktian", "UDP包大小-->"+h264Size);
//					try {
//						DatagramPacket datagramPacket = new DatagramPacket(mH264buf, 0, h264Size, InetAddress.getByName("192.168.1.85"), 9090);
//						mDatagramSocket.send(datagramPacket);
//					} catch (UnknownHostException e) {
//						e.printStackTrace();
//					} catch (IOException e) {
//						e.printStackTrace();
//					}
					
					// RTP发送
					mRtpSession.sendRtp(mH264buf, h264Size);
				}
				
				mLock.unlock();
			}
		}
		
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		// TODO Auto-generated method stub
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		// TODO Auto-generated method stub
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		// TODO Auto-generated method stub
		CloseEncoder();
	}

}
