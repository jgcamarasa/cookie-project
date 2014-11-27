package com.jgcamarasa.cookieproject.messages;

import com.jgcamarasa.cookieproject.MsgType;

/**
 * Created by castor on 21/10/14.
 */
public interface IOutMessage {
    public MsgType getType();
    public byte[] getBytes();
}
