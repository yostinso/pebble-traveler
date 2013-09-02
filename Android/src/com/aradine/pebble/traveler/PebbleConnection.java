package com.aradine.pebble.traveler;

import static com.getpebble.android.kit.Constants.TRANSACTION_ID;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.UUID;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;

import com.getpebble.android.kit.PebbleKit;

public class PebbleConnection {
	public static final String LOG_TAG = PebbleConnection.class.getName();
	public static final int MAX_ATTEMPTS = 5;
	private static int transactionId = 0;
	private Context context;
	private UUID pebbleAppId;
	private PebbleReceiver pebbleReceiver;
	private boolean connected;
	private OutstandingTransactions transactions = new OutstandingTransactions();
	
	private Thread messageRunner = null;
	private MessageRunnable messageRunnable = null;
	
	private enum PebbleIntentType {
		CONNECTED, DISCONNECTED, ACKED, NACKED
	};
	private class MessageRunnable implements Runnable {
		private boolean dying = false;
		@Override
		public void run() {
			LineMessage msg;
			int transId = -1;
			while (true) {
				Log.d(LOG_TAG, "MessageRunnable checking for new messages...");
				synchronized (transactions) {
					List<Integer> keys = new ArrayList<Integer>(transactions.keySet());
					msg = null;
					if (keys.size() > 0) {
						Collections.sort(keys);
						for (int i = 0; i < keys.size(); i++) {
							if (!transactions.get(keys.get(i)).isSending) {
								transId = keys.get(i);
								msg = transactions.get(transId);
							}
						}
					}
				}
				if (msg != null) {
					Log.d(LOG_TAG, "MessageRunnable sending message with id " + msg.getMessageId() + ", transactionId: " + transId);
					msg.isSending = true;
					PebbleKit.sendDataToPebbleWithTransactionId(context, pebbleAppId, msg.lineInfo.toPebbleDictionary(), transId);
				}
				
				Log.d(LOG_TAG, "MessageRunnable sleeping...");
				try {
					Thread.sleep(1000);
				} catch (InterruptedException e) {}
				if (dying) { break; }
			}
		}

		public void die() {
			this.dying = true;
		}
		public boolean isDying() {
			return this.dying;
		}
	}
	
	private class PebbleReceiver extends BroadcastReceiver {
		private Context context;
		
		public PebbleReceiver(Context applicationContext) {
			this.context = applicationContext;
			this.register();
		}

		@Override
		public void onReceive(Context context, Intent intent) {
			Log.d(LOG_TAG, PebbleReceiver.class.getName() + " receiving intent " + intent.getAction());
			PebbleIntentType t = getPebbleIntentTypeFor(intent.getAction());
			final int transactionId;
			switch (t) {
			case CONNECTED:
				connected = true;
				break;
			case DISCONNECTED:
				connected = false;
				break;
			case ACKED:
	            transactionId = intent.getIntExtra(TRANSACTION_ID, -1);
	            if (transactionId != -1)
	            	handleAck(transactionId);
	            break;
			case NACKED:
	            transactionId = intent.getIntExtra(TRANSACTION_ID, -1);
	            if (transactionId != -1)
	            	handleNack(transactionId);
	            break;
			default:
				Log.w(LOG_TAG, "Unexpected intent received: " + intent.getAction());
			}
			
		}
		private PebbleIntentType getPebbleIntentTypeFor(String action) {
			if (com.getpebble.android.kit.Constants.INTENT_PEBBLE_CONNECTED.equals(action)) {
				return PebbleIntentType.CONNECTED;
			} else if (com.getpebble.android.kit.Constants.INTENT_PEBBLE_DISCONNECTED.equals(action)) {
				return PebbleIntentType.DISCONNECTED;
			} else if (com.getpebble.android.kit.Constants.INTENT_APP_RECEIVE_ACK.equals(action)) {
				return PebbleIntentType.ACKED;
			} else if (com.getpebble.android.kit.Constants.INTENT_APP_RECEIVE_NACK.equals(action)) {
				return PebbleIntentType.NACKED;
			} else {
				return null;
			}
		}
		public void register() {
			Log.d(LOG_TAG, "Registering receivers");
			PebbleKit.registerPebbleConnectedReceiver(context, this);
			PebbleKit.registerPebbleDisconnectedReceiver(context, this);
			// Register ACK and NACK the hard way
			IntentFilter ackFilter = new IntentFilter(com.getpebble.android.kit.Constants.INTENT_APP_RECEIVE_NACK);
			context.registerReceiver(this, ackFilter);
			IntentFilter nackFilter = new IntentFilter(com.getpebble.android.kit.Constants.INTENT_APP_RECEIVE_ACK);
			context.registerReceiver(this, nackFilter);
		}
		public void unregister() {
			Log.d(LOG_TAG, "Unregistering receivers");
			context.unregisterReceiver(this);
		}
	}

	public PebbleConnection(Context applicationContext) {
		this.context = applicationContext;
		this.pebbleAppId = UUID.fromString(context.getString(R.string.pebble_app_id));
		
		connected = PebbleKit.isWatchConnected(context);
		pebbleReceiver = new PebbleReceiver(context);
		this.register();
	}
	
	public void register() {
		pebbleReceiver.register();
		synchronized (this) {
			if (messageRunner == null || !messageRunner.isAlive() || messageRunnable.isDying()) {
				messageRunnable = new MessageRunnable(); 
				messageRunner = new Thread(messageRunnable);
				messageRunner.start();
			}
		}
	}
	public void unregister() {
		pebbleReceiver.unregister();
		synchronized (this) {
			messageRunnable.die();
			messageRunner.interrupt();
		}
	}
	public boolean activateApp() {
		if (connected) {
			PebbleKit.startAppOnPebble(context, pebbleAppId);
			return true;
		} else {
			return false;
		}
	}
	private void queueMessage(LineMessage lineMessage) {
		synchronized (transactions) {
			int transId = lineMessage.attempts == 0 ? lineMessage.getMessageId() : nextTransactionId();
			transactions.put(transId, lineMessage);
		}
	}
	
	public boolean sendMessage(LineInfo lineInfo) {
		return sendMessage(new LineMessage(lineInfo, nextTransactionId()));
	}
	private boolean sendMessage(LineMessage msg) {
		Log.d(LOG_TAG, "Attempting to send message: " + msg.lineInfo.toString());
		if (connected) {
			Log.d(LOG_TAG, "Sending message with Id: " + msg.getMessageId() + ", message: " + msg.lineInfo.toString() + ", attempt " + msg.attempts);
			queueMessage(msg);
			return true;
		} else {
			Log.d(LOG_TAG, "Watch not connected!");
			return false;
		}
	}
	
	private void handleAck(int transactionId) {
		synchronized (transactions) {
			LineMessage msg = transactions.get(transactionId);
			if (msg == null) {
				Log.w(LOG_TAG, "Got ACK for unknown/expired transaction " + transactionId);
			} else {
				Log.d(LOG_TAG, "Got ACK for transaction " + transactionId + ", removing from outstanding");
				transactions.remove(transactionId);
			}
		}
	}
	
	private void handleNack(int transactionId) {
		LineMessage msg;
		synchronized (transactions) {
			msg = transactions.get(transactionId);
			if (msg == null) {
				Log.d(LOG_TAG, "Got NACK for unknown/expired transaction " + transactionId);
			} else {
				if (msg.attempts < MAX_ATTEMPTS) {
					Log.w(LOG_TAG, "Got NACK for transaction " + transactionId + "; " + msg.attempts + "attempts, trying again");
					msg.isSending = false;
					msg.attempts++;
				} else {
					Log.w(LOG_TAG, "Got NACK for transaction " + transactionId + "; " + msg.attempts + "attempts, failing");
					msg = null;
				}

				transactions.remove(transactionId);
			}
		}
		if (msg != null) {
			sendMessage(msg);
		}
	}
	
	private int nextTransactionId() {
		transactionId++;
		if (transactionId > 255) {
			transactionId = 0;
		}
		return transactionId;
	}
	
	// TODO: Try to keep track of local state as compared to what's on the watch
	// TODO: Need a clear() method on the watch and a relevant message
	// TODO: Figure out how to active the watch app
	// TODO: Need a ping() method on the watch and a relevant message which we can use to see if the app is running
}
