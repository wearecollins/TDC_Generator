//
//  TypeParticleSystem.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/6/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "ofMain.h"

// system
#include "TDCConstants.h"
#include "TypeParticle.h"
#include "TargetMesh.h"

#include "ofxLabFlexParticleSystem.h"

// data set
#include "Data.h"

// behaviorz
#include "behaviors/CameraWarp.h"
#include "behaviors/Flocking.h"
#include "behaviors/Noise.h"
#include "behaviors/Warp.h"
#include "behaviors/BumpMap.h"

// interaction
#include "CameraManager.h"

class TypeParticleSystem : public ofxLabFlexParticleSystem, protected ofThread
{
public:
    
    typedef map<unsigned long, ofxLabFlexParticle*>::iterator               TypeIterator;
    
    ~TypeParticleSystem();
    
    void setup( string directory = "meshes" );
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
    Behavior *  getSettingsBehavior(); // behaviors[ MOVE_NONE ] is used to store/set settings
    
    // grid vs outline
    void setUseGrid( bool useGrid );
    
    double       scale;
    
    // set colors
    void setColor( ofFloatColor color, float variance = 0.0 );
    
    // data object
    Data        dataObject;
    
    void        setMesh( string name ){ currentMeshName = name; };
    
    // the all important
    map< string, TargetMesh> meshes;
    
protected:
    string      sourceDirectory;
    
    string currentMeshName;
    
    // threadzzz
    void threadedFunction();
    
    // special lookup by letter
    Container   _particlesGrid, _particlesOutline;
    bool        bMeshIsUpdated;
    int         meshUpdatingFrames;
    
    // flags to update mesh
    bool        bNeedToChangeMesh;
    bool        bUseGrid;
    bool        bNeedToChangeColor;
    
    ofFloatColor     particleColor;
    float           colorVariance;
    
    // drawing / meshes
    
    DrawMode    drawMode, lastDrawMode;
    TypeOutline outline;
    
    ofMesh *    currentMesh;
    ofMesh *    currentMeshBuffer;
    
    Behavior *  currentBehavior;
    map<MovementType, Behavior *> behaviors;
    
    TypeIterator it;
    ofVec2f     lastMouse;
    float       lastMass;
    
    // interaction
    CameraManager camera;
    
    // movement stuff
    MovementType moveType;
    ofVec2f      gravity;
};