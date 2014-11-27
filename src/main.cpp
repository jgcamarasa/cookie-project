#include "GLES2/gl2.h"

#ifndef DESKTOP
#include "bcm_host.h"
#endif

#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include "Messages.h"
#include "Renderer.h"
#include "State.h"
#include <time.h>
#include <Box2D/Box2D.h>



const double OMEGA = 1000.0/60.0; // 

using namespace std;

pthread_mutex_t stateMutex;
State state;
b2World *m_world;
Renderer renderer;

void error(char *msg)
{
  perror(msg);
  exit(1);
}

void debugMem(unsigned char *mem, int size){
    int i;
    printf("mem size : %zu bytes\n",size);
    for(i=0;i<size;i++)
    printf("%02x ",mem[i]);
}

timespec diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

void* inputFunc(void *threadId);
void getThrustPositions(int numPlayers, float xPos[3], float yPos[3]);

void createGround(float x, float y, float w, float h, float angle = 0){
    const float32 k_restitution = 0.4f;
    
    b2Body* ground;
    b2BodyDef bd;
    bd.position.Set(x, y);
    bd.angle = 0.0174532925*angle;
    bd.type = b2_staticBody;

    ground = m_world->CreateBody(&bd);

    b2PolygonShape shape;
    shape.SetAsBox(w, h);

    b2FixtureDef sd;
    sd.shape = &shape;
    sd.density = 1.0f;
    sd.restitution = k_restitution;

    ground->CreateFixture(&sd);
    state.ground.push_back(ground);
}

void initState(){
    const float32 k_restitution = 0.8f;
    createGround(0.0f, -18.0f, 20.0, 10.0);
    createGround(0.0f, 18.0f, 20.0, 10.0);
    createGround(-15.0f*renderer.getRatio(), 0.0f, 10.0, 10.0);
    createGround(15.0f*renderer.getRatio(), 0.0f, 10.0, 10.0);
    createGround(0.0f, -11.0f, 4.0, 4.0, 10);
    createGround(3.0f, -10.0f, 4.0, 4.0, -15);
    createGround(-2.0f, -11.0f, 4.0, 4.0, 20);
    createGround(6.0f, -12.0f, 4.0, 4.0, 30);
    createGround(-7.0f, -12.0f, 4.0, 4.0, -20);
    createGround(12.0f, -11.0f, 4.0, 4.0, 10);
    createGround(17.0f, -10.0f, 4.0, 4.0, -15);
    createGround(-15.0f, -11.0f, 4.0, 4.0, 0);
    createGround(20.0f, -12.0f, 4.0, 4.0, 30);
    createGround(-12.4f, -10.0f,3.0, 3.0, 20);
    
    createGround(-11.0f, -12.0f, 0.5, 15.0, -5);
    createGround(-7.0f, 12.0f, 0.6, 15.0, 5);
    createGround(-0.0f, -16.0f, 0.5, 15.0, 0);
    createGround(-0.0f, 17.0f, 0.5, 15.0, 0);

    
    b2Body* player;
    {
        b2BodyDef bd2;
        bd2.position.Set(-15.0f, -6.0f);
        bd2.type = b2_dynamicBody;
        bd2.linearDamping = 0.2f;
        bd2.angularDamping = 0.5f;

        player = m_world->CreateBody(&bd2);

        b2PolygonShape shape2;
        //shape2.SetAsBox(1.0f, 1.0f);
        b2Vec2 vertices[3];
        vertices[0].Set(+0.0f, +1.0f);
        vertices[1].Set(-0.866f, -0.5f);
        vertices[2].Set(+0.866f, -0.5f);

        int32 count = 3;


        shape2.Set(vertices, count);

        b2FixtureDef sd2;
        sd2.shape = &shape2;
        sd2.density = 5.0f;
        sd2.restitution = k_restitution;

        player->CreateFixture(&sd2);
        
    }
    
    
    state.player = player;

    
    state.numPlayers = 0;
    for(int i = 0; i < MAX_PLAYERS; i++){
        state.playerStates[i] = 0;
        state.secondsFromLastMsg[i] = 0.0f;
    }
    
    
}

int main(int argc, char *argv[])
{
#ifndef DESKTOP
    bcm_host_init();
#endif
    
    // Input
    pthread_t thread;
    int error = pthread_create(&thread, NULL, inputFunc, (void*)0);
    

    m_world = new b2World(b2Vec2(0,-5));

    renderer.init();
    
    timespec act, ant;

    clock_gettime(CLOCK_REALTIME, &ant);
    clock_gettime(CLOCK_REALTIME, &act);
    
    double ms=0; //leftovers
    
    

    
    initState();
    
    while (1) {
        clock_gettime(CLOCK_REALTIME, &act);
        
        timespec dif = diff(ant, act);
   
        double milliseconds = dif.tv_nsec/1000000.0;
        if ( milliseconds > 250 ){
            milliseconds = 250;
        }
        
        ant.tv_sec = act.tv_sec;
        ant.tv_nsec = act.tv_nsec;


        int ticks = (milliseconds+ms)/(OMEGA);
        ms = (milliseconds+ms)-ticks*(OMEGA);
        //printf("Sec: %d, Nsec: %ld\n", dif.tv_sec, dif.tv_nsec);
        //printf("Milliseconds:%f, Ticks:%d, ms: %f\n", milliseconds, ticks, ms);

        float xPos[3], yPos[3];
        getThrustPositions(state.numPlayers, xPos, yPos);
        float angs[3] = {10*0.0174532925*0, -10*0.0174532925, 0};
        pthread_mutex_lock(&stateMutex);
        {
            if(state.playerStates[0]){
                //state.playerStates[2] = 1;
            }else{
                //state.playerStates[2]= 0;
            }
            for(int i = 0; i < MAX_PLAYERS; i++){
                if(state.playerStates[i] == 1){ 
                    b2Body *playerBody = state.player;
                    b2Vec2 force;
                    force.y = cos(playerBody->GetAngle()+4.1887902048*i/*angs[i]*/)*30;
                    force.x = -sin(playerBody->GetAngle()+4.1887902048*i/*angs[i]*/)*30;
                    
                    
                    printf("(%f, %f)\n", playerBody->GetAngle(), playerBody->GetAngle()*57.2957795);
                    printf("(%f, %f)\n", playerBody->GetAngle(), playerBody->GetAngle()*57.2957795);
                    printf("(%f, %f)\n\n", force.x/30, force.y/30);

                    playerBody->ApplyForce(force, playerBody->GetWorldPoint(b2Vec2(xPos[i],yPos[i])), true);

                }
            }
        } 
        pthread_mutex_unlock(&stateMutex);
        
        for(int i=0;i<ticks;i++){
            m_world->Step(OMEGA/1000, 10, 8);
        }
        
        
        renderer.startDraw();
        pthread_mutex_lock(&stateMutex);
        {
            renderer.drawState(state);
        } 
        pthread_mutex_unlock(&stateMutex);
        renderer.endDraw();  
        
        

        
    }

    renderer.finalize();
   
    
    return 0;    
    
}
    
void* inputFunc(void *threadId){
    int id = (int)threadId;
    int sockfd;
    struct sockaddr_in servaddr,cliaddr;

    unsigned char mesg[1000];

    sockfd=socket(AF_INET,SOCK_DGRAM,0);

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(18213);
    bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    
    // Possible client msgs
    MsgClientTouchInput msgClientTouchInput;
    
    timespec act, ant;

    clock_gettime(CLOCK_REALTIME, &ant);
    clock_gettime(CLOCK_REALTIME, &act);

    for (;;)
    {
        //clock_gettime(CLOCK_REALTIME, &act);
        
        //timespec dif = diff(ant, act);
        
        double seconds = 0.0;//dif.tv_nsec/1000000000.0;
        
        //ant.tv_sec = act.tv_sec;
        //ant.tv_nsec = act.tv_nsec;
        
        socklen_t len = sizeof(cliaddr);
        int bytesReceived = recvfrom(sockfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&len);
        int bytesRead = 0;
        //debugMem(mesg, bytesReceived);
        
        pthread_mutex_lock(&stateMutex);
        {
            for(int i = 0; i < state.numPlayers; i++){
                state.secondsFromLastMsg[i] += seconds;
            }
        } 
        pthread_mutex_unlock(&stateMutex);
        
        
        while(bytesRead < bytesReceived){
            unsigned char msgType = mesg[0];
            switch(msgType){
                case MSG_CLIENT_DISCOVER:
                    _msgServerDiscoverResponse.playerId = state.numPlayers;
                    switch(_msgServerDiscoverResponse.playerId){
                        case 0:
                            _msgServerDiscoverResponse.r = 255;
                            _msgServerDiscoverResponse.g = 0;
                            _msgServerDiscoverResponse.b = 0;
                            break;
                        case 1:
                            _msgServerDiscoverResponse.r = 0;
                            _msgServerDiscoverResponse.g = 255;
                            _msgServerDiscoverResponse.b = 0;
                            break;
                        case 2:
                            _msgServerDiscoverResponse.r = 0;
                            _msgServerDiscoverResponse.g = 0;
                            _msgServerDiscoverResponse.b = 255;
                            break;
                        case 3:
                            _msgServerDiscoverResponse.r = 255;
                            _msgServerDiscoverResponse.g = 0;
                            _msgServerDiscoverResponse.b = 255;
                            break;
                        case 4:
                            _msgServerDiscoverResponse.r = 255;
                            _msgServerDiscoverResponse.g = 255;
                            _msgServerDiscoverResponse.b = 0;
                            break;
                    }
                    
                    //state.secondsFromLastMsg[_msgServerDiscoverResponse.playerId] = 0.0f;
                    
                    sendto(
                        sockfd,
                        &_msgServerDiscoverResponse,
                        sizeof(_msgServerDiscoverResponse),
                        0,
                        (struct sockaddr *)&cliaddr,
                        sizeof(cliaddr));
                    printf("Discover received. Responding!\n");
                    state.numPlayers++;
                    bytesRead += 1;
                    break;
                    
                case MSG_CLIENT_TOUCH_INPUT:
                    msgClientTouchInput.msgType = msgType;
                    msgClientTouchInput.playerId = mesg[1];
                    msgClientTouchInput.state = mesg[2];
                    pthread_mutex_lock(&stateMutex);
                    {
                        if(msgClientTouchInput.state == 0){
                            state.playerStates[msgClientTouchInput.playerId] = 0;
                        }else{
                            state.playerStates[msgClientTouchInput.playerId] = 1;
                        }
                        state.secondsFromLastMsg[msgClientTouchInput.playerId] = 0.0f;
                    } 
                    pthread_mutex_unlock(&stateMutex);
                   
                    bytesRead += 3;
                    
                    break;
            }
        }
        
        pthread_mutex_lock(&stateMutex);
        {
            for(int i = 0; i < state.numPlayers; i++){
                //printf("%f\n",state.secondsFromLastMsg[i]);
                if(state.secondsFromLastMsg[i] >= 3.0f){
                    
                }
            }
        } 
        pthread_mutex_unlock(&stateMutex);
        
    }
    
    pthread_exit(NULL);
}
