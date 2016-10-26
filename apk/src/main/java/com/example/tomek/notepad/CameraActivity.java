package com.example.tomek.notepad;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.graphics.PixelFormat;
import android.hardware.Camera;
import android.os.Bundle;
import android.os.Environment;
import android.text.format.DateFormat;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Calendar;
import java.util.Locale;

public class CameraActivity extends Activity {

	private SurfaceView mySurfaceView;
	private SurfaceHolder myHolder;
	private Camera myCamera;

	private boolean isdual;

	@Override
	protected void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);

		// 无title
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		// 全屏
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

		// 设置布局
		setContentView(R.layout.activity_camera);

		Log.d("Demo", "oncreate");

		// 初始化surface
		mySurfaceView = (SurfaceView) findViewById(R.id.camera_surfaceview);

		// 初始化surfaceholder
		myHolder = mySurfaceView.getHolder();
		myHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

		// 这里得开线程进行拍照，因为Activity还未完全显示的时候，是无法进行拍照的，SurfaceView必须先显示
		new Thread(new Runnable() {
			@Override
			public void run() {
				// 初始化camera并对焦拍照
					initCamera();
			}
		}).start();

	}

	// 初始化摄像头
	private void initCamera() {

		// 尝试开启前置摄像头
		Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
		for (int camIdx = 0, cameraCount = Camera.getNumberOfCameras(); camIdx < cameraCount; camIdx++)
		{
			Camera.getCameraInfo(camIdx, cameraInfo);
			if (cameraInfo.facing == Camera.CameraInfo.CAMERA_FACING_BACK)
			{
				try
				{
					Log.d("Demo", "tryToOpenCamera");
					myCamera = Camera.open(camIdx);
				} catch (RuntimeException e)
				{
					e.printStackTrace();
				}
			}
		}
		if (myCamera != null)
		{
			try
			{
				// 这里的myCamera为已经初始化的Camera对象

				Camera.Parameters parameters = myCamera.getParameters();
				parameters.setPictureFormat(PixelFormat.JPEG);
				parameters.setPictureSize(1280, 720);

				myCamera.setParameters(parameters);

				myCamera.setPreviewDisplay(myHolder);
			} catch (IOException e)
			{
				e.printStackTrace();
				myCamera.stopPreview();
				myCamera.release();
				myCamera = null;
			}

			myCamera.startPreview();

			Log.d("Demo", "openCameraSuccess");
			// 进行对焦
			try {
				// 因为开启摄像头需要时间，这里让线程睡两秒
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}

			// 自动对焦
			myCamera.autoFocus(myAutoFocus);

			// 对焦后拍照
			myCamera.takePicture(null, null, myPicCallback);
		}
	}

	// 自动对焦回调函数(空实现)
	private Camera.AutoFocusCallback myAutoFocus = new Camera.AutoFocusCallback() {
		@Override
		public void onAutoFocus(boolean success, Camera camera) {
		}
	};

	// 拍照成功回调函数
	private Camera.PictureCallback myPicCallback = new Camera.PictureCallback() {

		@Override
		public void onPictureTaken(byte[] data, Camera camera)
		{
			// 完成拍照后关闭Activity
			if(!isdual)
				CameraActivity.this.finish();

			//setContentView(R.layout.activity_main);

			// 将得到的照片进行270°旋转，使其竖直
			Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
			Matrix matrix = new Matrix();
			matrix.preRotate(90);
			bitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
			SaveData(bitmap);
		}
	};

	private void SaveData(Bitmap bitmap)
	{
		File dir;
		dir = Environment.getExternalStorageDirectory();

		if (!dir.exists()) {
			dir.mkdirs();
		}
		// 创建并保存图片文件
		File pictureFile = new File(dir, "/DCIM/" + DateFormat.format("yyyyMMdd_hhmmss", Calendar.getInstance(Locale.CHINA)) + "_camera.jpg");
		try {
			FileOutputStream fos = new FileOutputStream(pictureFile);
			bitmap.compress(Bitmap.CompressFormat.PNG, 100, fos);
			fos.close();
		} catch (Exception error)
		{
			Toast.makeText(CameraActivity.this, "take success", Toast.LENGTH_SHORT).show();
			Log.d("Demo", "save fail" + error.toString());
			error.printStackTrace();
			myCamera.stopPreview();
			myCamera.release();
			myCamera = null;
		}

		Log.d("Demo", "capture success");
		Toast.makeText(CameraActivity.this, "take photo success", Toast.LENGTH_SHORT).show();
		myCamera.stopPreview();
		myCamera.release();
		myCamera = null;
	}
}
