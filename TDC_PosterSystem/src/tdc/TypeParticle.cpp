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
        offsetZ = ofRandom(10000);
        seedPosition.z = ofRandom(-.01, .01);
    }
    ofxLabFlexParticle::update();
    velocity = velocity * .99 + (seedPosition - *this ) * .01;
    z = z * .99;
    if ( z > 100 ){
        z = 100;
    } else if ( z < -100 ){
        z = -100;
    }
}

//-------------------------------------------------------------------------------------------
void TypeParticle::draw()
{
    ofEllipse( *this, radius, radius );
}


//-------------------------------------------------------------------------------------------
TypeParticle& TypeParticle::operator=( const TypeParticle& p ){
    ofxLabFlexParticle::operator=(p);
    offsetX=p.offsetX;
    offsetY=p.offsetY;
    offsetZ=p.offsetZ;
    index=p.index;
    
    return *this;
}