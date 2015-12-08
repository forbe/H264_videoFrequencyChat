package com.example.ffmpegdemo;

import com.baylor.video.VideoManage;

import android.app.Activity;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

public class MainActivity extends Activity {

	private SurfaceView mSurfaceView1;
	private SurfaceView mSurfaceView2;


	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		// 防止屏幕睡眠
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		Window win = getWindow();
		win.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		win.setFlags(WindowManager.LayoutParams.FLAG_FORCE_NOT_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FORCE_NOT_FULLSCREEN);
		setContentView(R.layout.activity_main);

		mSurfaceView1 = (SurfaceView) findViewById(R.id.surfaceview1);
		
		mSurfaceView2 = (SurfaceView) findViewById(R.id.surfaceview2);
		findViewById(R.id.btnStart).setOnClickListener(
				new View.OnClickListener() {

					@Override
					public void onClick(View arg0) {
						startVideoManage();
					}
				});

	}
	
	@Override
	protected void onPause() {
		closeVideoManage();
		super.onPause();
	}
	
	private VideoManage mVideoManage;
	
	private void startVideoManage() {
		mVideoManage = new VideoManage(mSurfaceView1, mSurfaceView2, true, true);
		mVideoManage.open();
	}
	
	private void closeVideoManage() {
		mVideoManage.close();
	}

}
