//
//  TypeParticleSystem.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/6/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "ofMain.h"
#include "TypeGrid.h"
#include "TypeParticle.h"

#include "ofxLabFlexParticleSystem.h"

// this is dumb
struct QuickVertex
{
    ofVec2f pos;
    int     index;
};

class TypeParticleSystem : public ofxLabFlexParticleSystem, protected ofThread
{
public:
    
    enum DrawMode {
        DRAW_NULL   = -1,
        DRAW_POINTS = 0,
        DRAW_LINES,
        DRAW_LINES_RANDOMIZED,
        DRAW_LINES_ARBITARY,
        DRAW_SHAPES
    };
    
    ~TypeParticleSystem();
    
    void setup( string file );
    void update();
    void draw();
    void mouseMoved( int x, int y );
    
    void setDrawMode( DrawMode mode );
    
protected:
    string      svgFile;
    // threadzzz
    void threadedFunction();
    
    // special lookup by letter
    vector<vector <QuickVertex> > letterParticles;
    bool        bMeshIsUpdated;
    int         meshUpdatingFrames;
    
    DrawMode    drawMode, lastDrawMode;
    TypeGrid    grid;
    ofMesh      drawMesh, bufferMesh;
    ofxLabFlexParticleSystem::Iterator it;
    ofVec2f     lastMouse;
    float       lastMass;
};