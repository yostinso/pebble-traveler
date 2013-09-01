package com.aradine.pebble.traveler.test;

import com.aradine.pebble.traveler.LineInfo;
import com.getpebble.android.kit.util.PebbleDictionary;

import junit.framework.TestCase;

public class TestLineInfo extends TestCase {
	protected void setUp() throws Exception {
		super.setUp();
	}

	protected void tearDown() throws Exception {
		super.tearDown();
	}

	public void testLineInfoStringStringIntStringInt() {
		LineInfo the22 = new LineInfo("22", "TQB", 8, "WAT", 9);
		assertEquals("Failed to set line",              "22",  the22.getLine());
		assertEquals("Failed to set inbound terminus",  "TQB", the22.getInbound().getTerminus());
		assertEquals("Failed to set inbound ETA",       8,     the22.getInbound().getEta());
		assertEquals("Failed to set outbound terminus", "WAT", the22.getOutbound().getTerminus());
		assertEquals("Failed to set outbound ETA",      9,     the22.getOutbound().getEta());
	}

	public void testToPebbleDictionaryInt() {
		LineInfo the22 = new LineInfo("22", "TQB", 8, "WAT", 9);
		PebbleDictionary dict = the22.toPebbleDictionary(0);
		byte[] expected = {
				'2', '2', '\0', '\0',
				'T', 'Q', 'B', '\0',
				0x8, 0x0, 0x0, 0x0,
				'W', 'A', 'T', '\0',
				0x9, 0x0, 0x0, 0x0
		};
		assertTrue("Couldn't find dictionary entry by key", dict.contains(0));
		byte[] bytes = dict.getBytes(0);
		assertEquals("Failed to generate 20-byte byte array", 20, bytes.length);
		for (int i = 0; i < bytes.length; i++) {
			assertEquals("Unexpected byte at position " + i, expected[i], bytes[i]);
		}
	}

}
