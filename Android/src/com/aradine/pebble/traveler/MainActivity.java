package com.aradine.pebble.traveler;

import java.util.UUID;

import com.getpebble.android.kit.PebbleKit;

import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class MainActivity extends Activity {
	public static final String LOG_TAG = "MainActivity";
	
	private static UUID pebbleAppId = null;
	private int transactionId = 0;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		pebbleAppId = UUID.fromString(this.getString(R.string.pebble_app_id));
		PebbleKit.isWatchConnected(this.getApplicationContext());

		long start_id = (System.currentTimeMillis() & 0x00FFFFFF) << 2;
		transactionId = (int)start_id;
		
		Button b = (Button)findViewById(R.id.button1);
		b.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				LineInfo i = new LineInfo("22", "TQB", 8, "WAT", 9);
				sendMessage(i);
			}
		});
	}
	
	private void sendMessage(LineInfo lineInfo) {
		Log.d(LOG_TAG, "Attempting to send message with transactionId: " + transactionId + ", message: " + lineInfo.toString());
		if (PebbleKit.isWatchConnected(this.getApplicationContext())) {
			Log.d(LOG_TAG, "Sending message with transactionId: " + transactionId + ", message: " + lineInfo.toString());
			PebbleKit.sendDataToPebbleWithTransactionId(getApplicationContext(), pebbleAppId, lineInfo.toPebbleDictionary(), transactionId);
			transactionId++;
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	protected void onPause() {
		super.onPause();
		// TODO Unregister BroadcastReceivers
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		// TODO Register BroadcastReceivers
	}

}
