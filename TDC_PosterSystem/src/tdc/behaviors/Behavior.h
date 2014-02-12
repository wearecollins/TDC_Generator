//
//  Behavior.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/12/14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLabFlexParticleSystem.h"

class TypeParticle;
class TypeParticleSystem;

class Behavior {
    
public:
    
    Behavior(){
        name = "undefined";
    }
    
    string getName(){
        return name;
    }
    
    virtual void update( TypeParticle * p ){};
    virtual void updateAll( ofxLabFlexParticleSystem::Container * c ){};
    
    // call before/after draw (duh)
    virtual void beginDraw(){}
    virtual void endDraw(){}
    
    // settings
    ofVec2f intensity;
    double   timeFactor;
    
protected:
    
    string name;
    
};
