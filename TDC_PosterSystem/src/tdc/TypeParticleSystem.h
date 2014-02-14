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
#include "TypeOutline.h"
#include "TypeParticle.h"

#include "ofxLabFlexParticleSystem.h"

// behaviorz
#include "behaviors/Flocking.h"
#include "behaviors/Noise.h"
#include "behaviors/Warp.h"
#include "behaviors/BumpMap.h"

// this is dumb
struct QuickVertex
{
    ofVec2f pos;
    int     index;
};

class TypeParticleSystem : public ofxLabFlexParticleSystem, protected ofThread
{
public:
    
    typedef map<unsigned long, ofxLabFlexParticle*>::iterator               TypeIterator;
    
    enum GridType {
        GRID_POINTS = 0,
        GRID_OUTLINE,
        GRID_NEGATIVE
    };
    
    enum DrawMode {
        DRAW_NULL   = -1,
        DRAW_POINTS = 0,
        DRAW_LINES,
        DRAW_LINES_RANDOMIZED,
        DRAW_LINES_ARBITARY,
        DRAW_SHAPES
    };
    
    enum MovementType {
        MOVE_NONE = 0,
        MOVE_NOISE,
        MOVE_WARP,
        MOVE_FLOCK,
        MOVE_BUMP,
        MOVE_GRAVITY
    };
    
    ~TypeParticleSystem();
    
    void setup( string file );
    void update();
    void draw();
    void mouseMoved( int x, int y );
    
    // draw mode
    DrawMode getDrawMode();
    void setDrawMode( DrawMode mode );
    string getDrawModeString();
    
    // behavior
    Behavior * getCurrentBehavior();
    void setBehavior( MovementType type );
    
    // grid vs outline
    void setUseGrid( bool useGrid );
    
protected:
    string      svgFile;
    // threadzzz
    void threadedFunction();
    
    // special lookup by letter
    vector<vector <QuickVertex> > letterGridParticles, letterOutlineParticles;
    Container   _particlesGrid, _particlesOutline;
    bool        bMeshIsUpdated;
    int         meshUpdatingFrames;
    
    bool        bNeedToChangeMesh;
    bool        bUseGrid;
    
    map<DrawMode, map<GridType,ofMesh> > meshes;
    void        buildMeshes();
    void        buildMesh(DrawMode mode, GridType type );
    
    DrawMode    drawMode, lastDrawMode;
    TypeOutline outline;
    TypeGrid    grid;
    TypeMesh *  currentTypeMesh;
    ofMesh *    currentMesh;
    ofMesh *    currentMeshBuffer;
    
    Behavior *  currentBehavior;
    map<MovementType, Behavior *> behaviors;
    
    vector<vector <QuickVertex> > * currentLetterParticles;
    
    ofMesh      gridMesh, bufferGridMesh;
    ofMesh      outlineMesh, bufferOutlineMesh;
    TypeIterator it;
    ofVec2f     lastMouse;
    float       lastMass;
    
    // movement stuff
    MovementType moveType;
    ofVec2f      gravity;
};