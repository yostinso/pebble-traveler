package com.aradine.pebble.traveler;

public class LineMessage {
	public LineInfo lineInfo;
	public int attempts = 0;
	private int messageId;
	public boolean isSending = false;
	public LineMessage(LineInfo lineInfo, int messageId) {
		this.lineInfo = lineInfo;
		this.messageId = messageId;
	}
	public int getMessageId() {
		return messageId;
	}
}
