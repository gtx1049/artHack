package com.example.tomek.notepad;

import android.content.Context;
import android.hardware.Camera;
import android.media.CamcorderProfile;
import android.media.MediaRecorder;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.TextView;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;

class CameraThread extends Thread {
	private MediaRecorder mediarecorder;// 录制视频的类private long
	private SurfaceHolder surfaceHolder;
	private SurfaceView surfaceview;// 显示视频的控件
	private Camera mCamera;
	private long recordTime;
	private boolean isRecording = false;
	private int hour = 0;
	private int minute = 0;
	private int second = 0;
	private Context context;
	private TextView time;
	private long startTime = Long.MIN_VALUE;
	private long endTime = Long.MIN_VALUE;
	private HashMap<String, String> map = new HashMap<String, String>();
	private static final String TAG = "SEDs508EG";
	public static final int MEDIA_TYPE_IMAGE = 1;
	public static final int MEDIA_TYPE_VIDEO = 2;

	// public RecordThread() {
	// backStop();
	// }

	public CameraThread(long recordTime, SurfaceView surfaceview, SurfaceHolder surfaceHolder, Context context,
						TextView time) {
		this.recordTime = recordTime;
		this.surfaceview = surfaceview;
		this.surfaceHolder = surfaceHolder;
		this.context = context;
		this.time = time;
	}

	@Override
	public void run() {
		/** * 开始录像 */
		startRecord();
		/** * 启动定时器，到规定时间recordTime后执行停止录像任务 */
		//Timer timer = new Timer();
		//timer.schedule(new TimerThread(), recordTime);
	}

	/** * 开始录像 */
	public void startRecord() {
		mediarecorder = new MediaRecorder();// 创建mediarecorder对象
		try {
			mCamera = Camera.open();
		} catch (Exception e) {
			// 打开摄像头错误
			e.printStackTrace();
			Log.i("info", "open camera fail");
			return;
		}
		mCamera.unlock();

		mediarecorder.setCamera(mCamera); // 设置录制视频源为Camera(相机)
		mediarecorder.setAudioSource(MediaRecorder.AudioSource.CAMCORDER);
		mediarecorder.setVideoSource(MediaRecorder.VideoSource.CAMERA); // 设置录制文件质量，格式，分辨率之类，这个全部包括了
		mediarecorder.setProfile(CamcorderProfile.get(CamcorderProfile.QUALITY_LOW));
		mediarecorder.setPreviewDisplay(surfaceHolder.getSurface()); // 设置视频文件输出的路径
		// mediarecorder.setOutputFile("/sdcard/sForm.3gp");
		mediarecorder.setOutputFile(getOutputMediaFile(MEDIA_TYPE_VIDEO).toString());
		try { // 准备录制
			mediarecorder.prepare(); // 开始录制
			mediarecorder.start();
			// time.setVisibility(View.VISIBLE);// 设置录制时间显示
		} catch (IllegalStateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	/** * 停止录制 */
	public void stopRecord() {
		System.out.println("--------------");

		if (mediarecorder != null) {
			// 停止录制
			mediarecorder.stop();
			// time.setText(format(hour) + ":" + format(minute) + ":"
			// + format(second));
			// 释放资源
			if (mCamera != null) {
				mCamera.lock();
				mCamera.stopPreview();
				mCamera.release();
				mCamera = null;
				System.out.println("camera is null");
			}
			mediarecorder.reset();
			mediarecorder.release();

			mediarecorder = null;

		}
	}

	private static File getOutputMediaFile(int type) {
		// 判断SDCard是否存在
		if (!Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState())) {
			Log.d(TAG, "SDCard不存在");
			return null;
		}

		// File mediaStorageDir = new
		// 如果期望图片在应用程序卸载后还存在，且能被其他应用程序共享，则此保存位置最合适
		// 如果不存在的话，则创建存储目录
		File mediaStorageDir = new File(
				Environment.getExternalStorageDirectory() + File.separator + "/MyXingCheCamera3GP/");
		if (!mediaStorageDir.exists()) {
			if (!mediaStorageDir.mkdir()) {
				Log.d(TAG, "failed to create directory");
				return null;
			}
		}
		// 创建媒体文件名
		String timestamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
		File mediaFile;
		if (type == MEDIA_TYPE_IMAGE) {
			mediaFile = new File(mediaStorageDir.getPath() + File.separator + "IMG_" + timestamp + ".jpg");
		} else if (type == MEDIA_TYPE_VIDEO) {
			System.out.println(mediaStorageDir.getPath() + File.separator + "VID_" + timestamp + ".3gp");
			mediaFile = new File(mediaStorageDir.getPath() + File.separator + "VID_" + timestamp + ".3gp");
		} else {
			Log.d(TAG, "文件类型有误");
			return null;
		}

		return mediaFile;
	}
}
