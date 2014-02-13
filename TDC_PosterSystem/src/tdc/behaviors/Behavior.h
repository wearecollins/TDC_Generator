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
    
    
    virtual void setMouse( ofVec2f mousePosition ){ mouse = mousePosition; };
    virtual void setup( ofxLabFlexParticleSystem::Container * c ){};
    
    virtual void update( TypeParticle * p ){};
    virtual void updateAll( ofxLabFlexParticleSystem::Container * c ){};
    
    // call before/after draw (duh)
    virtual void beginDraw(){}
    virtual void endDraw(){}
    virtual void draw(){}
    
    // settings
    ofVec2f intensity;
    double   timeFactor;
    float    mix;
    
protected:
    
    string name;
    ofVec2f mouse;
};
