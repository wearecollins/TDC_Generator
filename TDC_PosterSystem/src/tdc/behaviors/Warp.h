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
        float multZ = sin( ofGetElapsedTimeMillis() * mix / 150);
        if ( intensity.x > intensity.y && intensity.x > intensity.z ){
            (*p) += ofVec3f( 0, ofSignedNoise((p->getSeedPosition().x / ofGetWidth() * (intensity.x + intensity.x* multX)) + ofGetElapsedTimeMillis() * timeFactor) * (intensity.y + intensity.y* multY), 0);
        } else if ( intensity.z > intensity.y && intensity.z > intensity.x ){
            (*p) += ofVec3f( 0, 0, ofSignedNoise((p->getSeedPosition().x / ofGetWidth() * (intensity.x + intensity.x* multX)) + ofGetElapsedTimeMillis() * timeFactor) * ((intensity.z - .5) + (intensity.z - .5) * multZ));
        } else {
            (*p) += ofVec3f( ofSignedNoise((p->getSeedPosition().y / ofGetHeight() * (intensity.y + intensity.y* multY)) + ofGetElapsedTimeMillis() * timeFactor) * (intensity.x + intensity.x * multX), 0, 0);
        }
    }
    
};