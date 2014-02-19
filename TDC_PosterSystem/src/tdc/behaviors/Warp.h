//
//  Warp.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/12/14.
//
//

#pragma once

#include "Behavior.h"

// this should be namespaced like woah

class Warp : public Behavior {
public:
    
    Warp() : Behavior(){
        name = "warp";
        intensity.set( 10.0f, 50.0f );
    }
    
    void update( TypeParticle * p ){
        float multX = sin( ofGetElapsedTimeMillis() * mix / 100);
        float multY = sin( ofGetElapsedTimeMillis() * mix / 200);
        if ( intensity.x > intensity.y ){
            (*p) += ofVec2f( 0, ofSignedNoise((p->getSeedPosition().x / ofGetWidth() * (intensity.x + intensity.x* multX)) + ofGetElapsedTimeMillis() * timeFactor) * (intensity.y + intensity.y* multY));
        } else {
            (*p) += ofVec2f( ofSignedNoise((p->getSeedPosition().y / ofGetHeight() * (intensity.y + intensity.y* multY)) + ofGetElapsedTimeMillis() * timeFactor) * (intensity.x + intensity.x * multX), 0);
        }
    }
    
};