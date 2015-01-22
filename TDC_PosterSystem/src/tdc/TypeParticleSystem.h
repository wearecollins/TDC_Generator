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

class TypeParticleSystem : public lab::ParticleSystem, protected ofThread
{
public:
    
    typedef map<unsigned long, lab::Particle*>::iterator               TypeIterator;
    
    ~TypeParticleSystem();
    
    void setup( string directory = "meshes" );
    void update();
    void draw();
    void mouseMoved( int x, int y );
    void kinectMoved( ofPoint & p );
    
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
    
    // set colors
    void setColor( ofFloatColor color, float variance = 0.0 );
    
    // data object
    Data dataObject;
    
    // the all important mesh map
    map< string, TargetMesh> meshes;
    
    // set which mesh we're rendering
    void setMesh( string name );
    
    // interaction
    CameraManager camera;
    
    // density is 0-1, wipes out some of the mesh
    float density;
    
    // return home 0-1
    float home;
    
    float kinectWeight;
    
    void explode();
    
    float pointSize;
    float pointRandomization;
    
    // ?
    double scale;
    
protected:
    ofShader pointShader;
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
    ofVec2f     lastMouse, lastKinect;
    float       lastMass, lastKinectMass;
    
    
    // movement stuff
    MovementType moveType;
    ofVec2f      gravity;
};