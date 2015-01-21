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
        frame = 0;
    }
    
    void updateAll( lab::ParticleSystem::Container * c ){
        frame++;
    }
    
    void update( TypeParticle * p ){
        (*p) += ofVec3f( ofSignedNoise(p->getSeedPosition().x + p->getOffsetX() + (getTime() * timeFactor)) * intensity.x,
                        ofSignedNoise(p->getSeedPosition().y + p->getOffsetY() + (getTime() * timeFactor)) * intensity.y,
                        ofSignedNoise(p->getSeedPosition().z + p->getOffsetZ() + (getTime() * timeFactor)) * (intensity.z - .5));
    }
    
    float getTime(){
        return frame;
        // return ofGetElapsedTimeMillis();
    }
    
    float frame;
};
