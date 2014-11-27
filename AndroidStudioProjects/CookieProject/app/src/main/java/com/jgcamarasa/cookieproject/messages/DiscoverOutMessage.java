package com.jgcamarasa.cookieproject.messages;

import com.jgcamarasa.cookieproject.MsgType;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Created by castor on 21/10/14.
 */
public class DiscoverOutMessage implements IOutMessage {

    @Override
    public MsgType getType() {
        return MsgType.MSG_CLIENT_DISCOVER;
    }

    @Override
    public byte[] getBytes() {
        ByteBuffer bb = ByteBuffer.allocate(1);
        bb.put((byte) getType().ordinal());
        bb.order(ByteOrder.LITTLE_ENDIAN);
        return bb.array();
    }
}
