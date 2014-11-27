package com.jgcamarasa.cookieproject.messages;

import com.jgcamarasa.cookieproject.MsgType;

/**
 * Created by castor on 21/10/14.
 */
public class ServerExitInMessage implements IInMessage {


    public void setFromBytes(byte[] data){
        int msgType = (int)data[0];
        assert (msgType == MsgType.MSG_SERVER_EXIT.ordinal());


    }

}
