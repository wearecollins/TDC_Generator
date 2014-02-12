//
//  Noise.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/12/14.
//
//

#pragma once

#include "Behavior.h"

// this should be namespaced like woah

class Noise : public Behavior {
public:
    
    Noise() : Behavior(){
        name = "noise";
    }
    
    void update( TypeParticle * p ){
        (*p) += ofVec2f( ofSignedNoise(p->getSeedPosition().x + p->getOffsetX() + (ofGetElapsedTimeMillis() * timeFactor)) * intensity.x, ofSignedNoise(p->getSeedPosition().y + p->getOffsetY() + (ofGetElapsedTimeMillis() * timeFactor)) * intensity.y);
    }
    
};
