package com.baylor.video;

import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.hardware.Camera;
import android.hardware.Camera.ErrorCallback;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;

import com.jni.bitmap.AndroidBitmapInfo;
import com.jni.ffmpeg.X264Decoder;
import com.jni.rtp.RTPSession;

public class VideoPlayer implements SurfaceHolder.Callback, ErrorCallback {

	private SurfaceView mSurfaceview = null;
	private SurfaceHolder mSurfacehold = null;

	private int mWidth = 0;
	private int mHeight = 0;

	private Bitmap mBitmap = null;

	private ThreadRtpReceive mThreadRtpReceive = null;
	private ThreadDrawImage mThreadDrawImage = null;
	private ThreadDecoder mThreadDecoder = null;
	
	private RTPSession mRtpSession;
	
//	private DatagramSocket mDatagramSocket;
	

	/**
	 * 开始显示
	 * 
	 * @param surfaceview
	 * @param width
	 * @param height
	 * @return
	 */
	@SuppressWarnings("deprecation")
	public int OpenDecoder(SurfaceView surfaceview, RTPSession rtpSession, int width, int height) {

		mRtpSession = rtpSession;
		
		mWidth = width;
		mHeight = height;

		mSurfaceview = surfaceview;
		mSurfacehold = mSurfaceview.getHolder();
		mSurfacehold.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
		mSurfacehold.addCallback((Callback) this);
		if (X264Decoder.initDecoder264(width, height) < 0) {
			return -1;
		}
		
		mThreadRtpReceive = new ThreadRtpReceive();
		mThreadRtpReceive.start();
		
		mThreadDecoder = new ThreadDecoder();
		mThreadDecoder.start();

		mThreadDrawImage = new ThreadDrawImage();
		mThreadDrawImage.start();

		return 0;
	}

	/**
	 * 关闭显示
	 */
	public void CloseDecoder() {
//		if (mDatagramSocket != null) {
//			mDatagramSocket.close();
//		}

		if (mThreadDecoder != null) {
			mThreadDecoder.stopThreadDecoder();
		}
		
		if (mThreadDrawImage != null) {
			mThreadDrawImage.stopThreadDrawImage();
		}
		
		if(mRtpSession != null) {
			mRtpSession.uninitRtp();
			mRtpSession = null;
		}

		X264Decoder.closeDecoder264();

	}

	@Override
	public void onError(int error, Camera camera) {
		// TODO Auto-generated method stub

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
		CloseDecoder();
	}
	
	
	/**
	 * RTP接收
	 * 
	 * @author kTian
	 *
	 */
	protected class ThreadRtpReceive extends Thread {
		
		@Override
		public void run() {
			if(mRtpSession != null) {
				mRtpSession.receiveRtp();
			}
		}
		
	}

	/**
	 * 解码H264
	 * 
	 * @author kTian
	 * 
	 */
	protected class ThreadDecoder extends Thread {

		public void stopThreadDecoder() {
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

//			byte buf[] = new byte[mWidth * mHeight * 3];
//			DatagramPacket dPacket = new DatagramPacket(buf, buf.length);
//
//			Log.d("ktian", "UDP循环接收--------------");
//			// 开始接收UDP
//			try {
//				mDatagramSocket = new DatagramSocket(8080); // 监听指定的端口号
//				while (!this.isInterrupted()) {
//					mDatagramSocket.receive(dPacket); // 阻塞语句
//
//					int size = X264Decoder.decoder264(dPacket.getData(),
//							dPacket.getLength());
//					Log.d("ktian", "size -->" + size);
//				}
//			} catch (SocketException e) {
//				e.printStackTrace();
//			} catch (IOException e) {
//				e.printStackTrace();
//			} catch (Exception e) {
//				e.printStackTrace();
//			}
//			Log.d("ktian", "UDP接收退出--------------");
			
			byte buf[] = new byte[mWidth * mHeight * 3];
			
			while (!this.isInterrupted()) {
//				try {
//					Thread.sleep(15);
//				} catch (Exception e) {
//					e.printStackTrace();
//				}
				
				int size = mRtpSession.rendRtp(buf);
				
				if(size > 0) {
					X264Decoder.decoder264(buf, size);
				}
			}
			
		}
	}

	/**
	 * 界面显示
	 * 
	 * @author kTian
	 * 
	 */
	protected class ThreadDrawImage extends Thread {

		// RGB数据内存空间
		private byte[] dataRgb = new byte[mWidth * mHeight * 3];

		private Rect src;
		private Rect dst;

		public void stopThreadDrawImage() {
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
				try {
					Thread.sleep(15);
				} catch (Exception e) {
					e.printStackTrace();
				}
				// 读取RGB数据
				int length = X264Decoder.render(dataRgb);
				
				if (length <= 0) {
					continue;
				}

				if (mBitmap == null) {
					if (mWidth <= 0 || mHeight <= 0) {
						continue;
					}

					src = new Rect(0, 0, mWidth, mHeight);
					dst = new Rect(0, 0, mSurfaceview.getWidth(),
							mSurfaceview.getHeight());
					
					if (dst.width() == 0 || dst.height() == 0) {
						continue;
					}
					//改变显示方向，自己写的，有点问题，不能用，但大概是利用矩阵对图片进行旋转
//					 Bitmap bitmap = BitmapFactory.decodeByteArray(dataRgb, 0, dataRgb.length);
//					  Matrix matrix = new Matrix();
//                      matrix.postRotate(180);
//                      mBitmap= Bitmap.createBitmap(bitmap,
//                             0, 0, mWidth, mHeight, matrix, true);
//                      
					mBitmap = Bitmap.createBitmap(mWidth, mHeight,
							Config.RGB_565);
					
					if (mBitmap == null) {
						continue;
					}
				}

				// fill bitmap
				AndroidBitmapInfo.fillRgb565(dataRgb, length, mBitmap);

				Canvas canvas = mSurfacehold.lockCanvas();
				if (canvas == null) {
					continue;
				}
				canvas.drawBitmap(mBitmap, src, dst, null);
				mSurfacehold.unlockCanvasAndPost(canvas);
			}
		}
	}

}
