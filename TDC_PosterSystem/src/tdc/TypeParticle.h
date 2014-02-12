//
//  TypeParticle.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/6/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "ofxLabFlexParticle.h"

class Behavior;

class TypeParticle : public ofxLabFlexParticle
{
    friend class TypeParticleSystem;
    friend class Behavior;
public:
    
    TypeParticle();
    
    TypeParticle( ofVec2f seedPositon );
    
    void update();
    void draw();
    ofVec2f getStart(){ return seedPosition; };
    
    int getOffsetX(){ return offsetX; };
    int getOffsetY(){ return offsetY; };
    ofVec3f getSeedPosition(){ return seedPosition; };
    
protected:
    
    ofVec2f seedPosition;
    int offsetX;
    int offsetY;
    bool bSet;
};

