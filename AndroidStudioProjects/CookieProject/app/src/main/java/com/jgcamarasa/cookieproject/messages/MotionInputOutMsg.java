package com.jgcamarasa.cookieproject.messages;

import com.jgcamarasa.cookieproject.MsgType;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Created by castor on 22/10/14.
 */
public class MotionInputOutMsg implements IOutMessage {
    float mX, mY, mZ;
    int mPlayerId;

    public MotionInputOutMsg(int playerId, float x, float y, float z){
        mX = x;
        mY = y;
        mZ = z;
        mPlayerId = playerId;
    }

    @Override
    public MsgType getType() {
        return MsgType.MSG_CLIENT_MOTION_INPUT;
    }

    @Override
    public byte[] getBytes() {
        ByteBuffer bb = ByteBuffer.allocate(4+4*3).order(ByteOrder.LITTLE_ENDIAN);
        bb.put((byte) getType().ordinal());
        bb.put((byte) mPlayerId);
        bb.putFloat(mX).putFloat(mY).putFloat(mZ);
        return bb.array();
    }
}