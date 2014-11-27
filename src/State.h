#ifndef STATE_H
#define STATE_H

#include <Box2D/Box2D.h>
#include <vector>

using namespace std;

#define MAX_PLAYERS 3

struct State {
    b2Body* player;
    
    vector<b2Body*> ground;

    int playerStates[MAX_PLAYERS];
    int numPlayers;
    float secondsFromLastMsg[MAX_PLAYERS];
};


#endif
