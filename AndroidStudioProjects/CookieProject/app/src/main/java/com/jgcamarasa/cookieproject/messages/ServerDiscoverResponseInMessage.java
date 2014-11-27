package com.jgcamarasa.cookieproject.messages;

import com.jgcamarasa.cookieproject.MsgType;

/**
 * Created by castor on 21/10/14.
 */
public class ServerDiscoverResponseInMessage implements IInMessage {
    private int mPlayerId;
    private int mR, mG, mB;


    public void setFromBytes(byte[] data){
        int msgType = (int)data[0];
        assert (msgType == MsgType.MSG_SERVER_DISCOVER_RESPONSE.ordinal());

        mPlayerId = (int)data[1];
        mR = (int)data[2] & 0xFF;;
        mG = (int)data[3] & 0xFF;
        mB = (int)data[4] & 0xFF;
    }

    public int getPlayerId() {
        return mPlayerId;
    }

    public int getR() {
        return mR;
    }

    public int getG() {
        return mG;
    }

    public int getB() {
        return mB;
    }
}
