enum MsgType {
    MSG_SERVER_DISCOVER_RESPONSE,
    MSG_CLIENT_DISCOVER,
    MSG_CLIENT_MOTION_INPUT,
    MSG_CLIENT_TOUCH_INPUT,
    MSG_SERVER_EXIT
};

struct MsgServerDiscoverResponse {
    unsigned char msgType;
    unsigned char playerId;
    unsigned char r, g, b;
    
    MsgServerDiscoverResponse(){
        msgType = MSG_SERVER_DISCOVER_RESPONSE;
    }
}_msgServerDiscoverResponse;

struct MsgClientDiscover {
    char msgType;
    
    MsgClientDiscover(){
        msgType = MSG_CLIENT_DISCOVER;
    }
};


struct MsgClientMotionInput{
    unsigned char msgType;
    unsigned char playerId;
    float x;
    float y;
    float z;
};



struct MsgClientTouchInput{
    unsigned char msgType;
    unsigned char playerId;
    unsigned char state;
};
