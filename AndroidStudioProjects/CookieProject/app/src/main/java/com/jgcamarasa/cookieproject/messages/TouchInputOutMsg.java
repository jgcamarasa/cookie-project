package com.jgcamarasa.cookieproject.messages;

import com.jgcamarasa.cookieproject.MsgType;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Created by castor on 22/10/14.
 */
public class TouchInputOutMsg implements IOutMessage {
    int mTouchState;
    int mPlayerId;

    public TouchInputOutMsg(int playerId, int state){
        mTouchState = state;
        mPlayerId = playerId;
    }

    @Override
    public MsgType getType() {
        return MsgType.MSG_CLIENT_TOUCH_INPUT;
    }

    @Override
    public byte[] getBytes() {
        ByteBuffer bb = ByteBuffer.allocate(3);
        bb.put((byte) getType().ordinal());
        bb.put((byte) mPlayerId);
        bb.put((byte) mTouchState);
        bb.order(ByteOrder.LITTLE_ENDIAN);
        return bb.array();
    }
}