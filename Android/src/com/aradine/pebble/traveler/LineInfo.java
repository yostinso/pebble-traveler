package com.aradine.pebble.traveler;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import android.util.Log;

import com.getpebble.android.kit.util.PebbleDictionary;

public class LineInfo {
	
	/*	
	 * typedef struct {
	 *	  char terminus[4];
	 *	  uint32_t eta;
	 *	} TerminusInfo;
	 *
	 *	typedef struct {
	 *	  char line[4];
	 *	  TerminusInfo inbound;
	 *	  TerminusInfo outbound;
	 *	} LineInfo;
	 */

	private static final int DICT_LEN = 20;
	public static final int LINE_DICT_KEY = 0x00000101;
	public class TerminusInfo {
		private String terminus = ""; // char[4]
		private int eta; // uint32_t
		
		private void setTerminus(String terminus) {
			this.terminus = trim(terminus, 3);
		}
		public String getTerminus() {
			return terminus;
		}
		private void setEta(int eta) {
			this.eta = eta;
		}
		public int getEta() {
			return eta;
		}
	}
	
	private String line = ""; // char[4]
	private TerminusInfo inbound = new TerminusInfo();
	private TerminusInfo outbound = new TerminusInfo();
	
	public LineInfo(String line, String inboundTerminus, int inboundEta, String outboundTerminus, int outboundEta) {
		this.setLine(line);
		inbound.setTerminus(inboundTerminus);
		inbound.setEta(inboundEta);
		outbound.setTerminus(outboundTerminus);
		outbound.setEta(outboundEta);
	}
	public LineInfo(String line, String inboundTerminus, String outboundTerminus) {
		this(line, inboundTerminus, 0, outboundTerminus, 0);
	}
	
	public void setLine(String line) {
		this.line = trim(line, 3);
	}
	public String getLine() {
		return line;
	}
	public void setOutboundTerminus(String terminus) {
		outbound.setTerminus(terminus);
	}
	public void setOutboundEta(int eta) {
		outbound.setEta(eta);
	}
	public void setInboundTerminus(String terminus) {
		inbound.setTerminus(terminus);
	}
	public void setInboundEta(int eta) {
		inbound.setEta(eta);
	}
	public TerminusInfo getOutbound() {
		return outbound;
	}
	public TerminusInfo getInbound() {
		return inbound;
	}
	
	public PebbleDictionary toPebbleDictionary(int key) {
		PebbleDictionary d = new PebbleDictionary();
		// line,inb_term,inb_eta,out_term,out_eta
		// xxx.III.nnnnOOO.nnnn
		ByteBuffer b = ByteBuffer.allocate(DICT_LEN).order(ByteOrder.LITTLE_ENDIAN);
			
		b.position(0);
		try {
			b.put(line.getBytes("US-ASCII"));
		} catch (UnsupportedEncodingException e) {
			Log.e(MainActivity.LOG_TAG, "Unable to use US-ASCII for line name encoding; using default", e);
			b.put(outbound.terminus.getBytes());
		}

		b.position(4);
		try {
			b.put(inbound.terminus.getBytes("US-ASCII"));
		} catch (UnsupportedEncodingException e) {
			Log.e(MainActivity.LOG_TAG, "Unable to use US-ASCII for inbound terminus encoding; using default", e);
			b.put(outbound.terminus.getBytes());
		}
		b.position(8);
		b.putInt(inbound.eta);
			
		b.position(12);
		try {
			b.put(outbound.terminus.getBytes("US-ASCII"));
		} catch (UnsupportedEncodingException e) {
			Log.e(MainActivity.LOG_TAG, "Unable to use US-ASCII for outbound terminus encoding; using default", e);
			b.put(outbound.terminus.getBytes());
		}
		b.position(16);
		b.putInt(outbound.eta);
		
		d.addBytes(key, b.array());
		
		return d;
	}
	
	public PebbleDictionary toPebbleDictionary() {
		return toPebbleDictionary(LINE_DICT_KEY);
	}
	
	@Override
	public String toString() {
		return line +
				"[" + inbound.getTerminus() + "@" + inbound.getEta() + "]" +
				"[" + outbound.getTerminus() + "@" + outbound.getEta() + "]";
	}

	private String trim(String str, int len) {
		if (line == null) {
			return "";
		} else if (line.length() > len) {
			return line.substring(0, len);
		} else {
			return str;
		}
	}
}
