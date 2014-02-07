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
};

//-------------------------------------------------------------------------------------------
TypeParticle::TypeParticle( ofVec2f seedPos ) : ofxLabFlexParticle( seedPos )
{
    radius = 1;
    seedPosition = seedPos;
}

//-------------------------------------------------------------------------------------------
void TypeParticle::update()
{
    ofxLabFlexParticle::update();
    velocity = velocity * .9 + (seedPosition - *this ) * .1;
}

//-------------------------------------------------------------------------------------------
void TypeParticle::draw()
{
    ofEllipse( *this, radius, radius );
}
