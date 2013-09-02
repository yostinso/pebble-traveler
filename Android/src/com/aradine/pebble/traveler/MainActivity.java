package com.aradine.pebble.traveler;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class MainActivity extends Activity {
	public static final String LOG_TAG = "MainActivity";
	private PebbleConnection pebbleConn;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		pebbleConn = new PebbleConnection(getApplicationContext());
		
		Button b = (Button)findViewById(R.id.button1);
		b.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				LineInfo i = new LineInfo("22", "TQB", 8, "WAT", 9);
				pebbleConn.sendMessage(i);
			}
		});
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
		pebbleConn.unregister();
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		pebbleConn.register();
	}

}
