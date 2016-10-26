package com.example.tomek.notepad;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

public class RecordService extends Service {
	RecordAct record = new RecordAct();

	@Override
	public void onCreate() {
		super.onCreate();
		System.out.println("service record");

		record.record();
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		record.stopRecord();
	}

	@Override
	public IBinder onBind(Intent intent) {
		// TODO: Return the communication channel to the service.
		throw new UnsupportedOperationException("Not yet implemented");
	}
}