//
//  TypeParticle.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/6/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "ofxLabFlexParticle.h"

class TypeParticle : public ofxLabFlexParticle
{
    friend class TypeParticleSystem;
public:
    
    TypeParticle();
    
    TypeParticle( ofVec2f seedPositon );
    
    void update();
    void draw();
    ofVec2f getStart(){ return seedPosition; };
    
protected:
    
    ofVec2f seedPosition;
    int offsetX;
    int offsetY;
    bool bSet;
};

