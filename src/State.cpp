#include "State.h"

void getThrustPositions(int numPlayers, float xPos[3], float yPos[3]){
    const float factor = 0.65;
    xPos[0] = 0.0f;
    yPos[0] = -0.5;
    
    xPos[1] = -0.4330127018922192;//-0.8f;
    yPos[1] = 0.25;
    
    xPos[2] = 0.4330127018922192;//0.8f;
    yPos[2] = 0.25;
    
    for(int i = 0; i < 3; i++){
        xPos[i] *= factor;
        yPos[i] *= factor;
    }
    
}
