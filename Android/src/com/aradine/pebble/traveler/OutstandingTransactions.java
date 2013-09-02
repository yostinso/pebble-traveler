package com.aradine.pebble.traveler;

import java.util.LinkedHashMap;

import android.util.Log;

public class OutstandingTransactions extends LinkedHashMap<Integer, LineMessage> {
		private static final long serialVersionUID = -6439648137691363525L;
	private static final int MAX_SIZE = 10;
	@Override
	protected boolean removeEldestEntry(java.util.Map.Entry<Integer, LineMessage> eldest) {
		if (this.size() > MAX_SIZE) {
			Log.w(PebbleConnection.LOG_TAG, "Dropping old outgoing message: " + eldest.getValue().getMessageId());
			return true;
		} else {
			return false;
		}
	}
}
