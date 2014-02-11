//
//  TypeParticle.cpp
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/6/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include "TypeParticle.h"

//-------------------------------------------------------------------------------------------
TypeParticle::TypeParticle() : ofxLabFlexParticle()
{
    radius = 1;
    offsetX = ofRandom(10000);
    offsetY = ofRandom(10000);
    bSet = false;
};

//-------------------------------------------------------------------------------------------
TypeParticle::TypeParticle( ofVec2f seedPos ) : ofxLabFlexParticle( seedPos )
{
    radius = 1;
    seedPosition = seedPos;
    offsetX = ofRandom(10000);
    offsetY = ofRandom(10000);
    bSet = false;
}

//-------------------------------------------------------------------------------------------
void TypeParticle::update()
{
    if (!bSet){
        bSet = true;
        offsetX = ofRandom(10000);
        offsetY = ofRandom(10000);
    }
    ofxLabFlexParticle::update();
    velocity = velocity * .99 + (seedPosition - *this ) * .01;
}

//-------------------------------------------------------------------------------------------
void TypeParticle::draw()
{
    ofEllipse( *this, radius, radius );
}
