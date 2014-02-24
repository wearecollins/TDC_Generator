//
//  TypeParticleSystem.cpp
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/6/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include "TypeParticleSystem.h"

ofMesh test;

//-------------------------------------------------------------------------------------------
TypeParticleSystem::~TypeParticleSystem(){
    waitForThread();
    stopThread();
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::setup( string directory ){
    
    // setup ourselves based on window size
    setupSquare(ofVec2f(ofGetWidth(), ofGetHeight()));
    setOption(ofxLabFlexParticleSystem::HORIZONTAL_WRAP, false);
    setOption(ofxLabFlexParticleSystem::VERTICAL_WRAP, false);
    setOption( ofxLabFlexParticleSystem::DETECT_COLLISIONS, false);
    
    sourceDirectory = directory;
    
    // mouse interaction
    lastMass = 10.0f;
    meshUpdatingFrames = 0;
    bNeedToChangeMesh = false;
    bUseGrid = true;
    scale = 1.0f;
    
    // behavior
    
    behaviors[ MOVE_NONE ] = new Behavior();
    behaviors[ MOVE_NOISE ] = new Noise();
    behaviors[ MOVE_WARP ] = new Warp();
    behaviors[ MOVE_FLOCK ] = NULL;
    behaviors[ MOVE_BUMP ] = new BumpMap();
    behaviors[ MOVE_PUSH ] = new CameraWarp();
    
    camera.setup();
    behaviors[ MOVE_BUMP ]->setup(NULL);
    behaviors[ MOVE_BUMP ]->camera = &camera;
    behaviors[ MOVE_PUSH ]->setup(NULL);
    behaviors[ MOVE_PUSH ]->camera = &camera;
    
    // setup data object
    dataObject.setup();
    
    //MOVE_GRAVITY,
    
    // null till we builds them
    currentMesh = new ofMesh();
    
    // containers
    currentMeshBuffer = new ofMesh();
    
    moveType = MOVE_FLOCK;
    
    currentBehavior = behaviors[ moveType ];
    gravity  = ofVec2f(0,4.0f);
    
    // load into names first
    ofDirectory meshDirectory;
    int numMeshes = meshDirectory.listDir(sourceDirectory);
    
    for ( int i=0; i<numMeshes; i++){
        meshes[ meshDirectory.getName(i)] = TargetMesh();
    }
    
    // lez do it
    startThread();
}

/********************************************************************************************
  __  __    _    ___ _   _   _____ _   _ ____  _____    _    ____
 |  \/  |  / \  |_ _| \ | | |_   _| | | |  _ \| ____|  / \  |  _ \
 | |\/| | / _ \  | ||  \| |   | | | |_| | |_) |  _|   / _ \ | | | |
 | |  | |/ ___ \ | || |\  |   | | |  _  |  _ <| |___ / ___ \| |_| |
 |_|  |_/_/   \_\___|_| \_|   |_| |_| |_|_| \_\_____/_/   \_\____/
 
 ********************************************************************************************/

void TypeParticleSystem::threadedFunction(){
    static bool bIsSetup = false;
    
    bMeshIsUpdated = false;
    
    if (!bIsSetup){
        
        // setup particles
        
        ofColor color(255);
        
        int numParticles = 7000;
        
        // build particles for grid
        for (int i=0; i<numParticles; i++){
            TypeParticle * t = new TypeParticle( ofVec3f(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), 0));
            t->velocity = ofVec3f(ofRandom(-100,100),0,0);
            t->mass = 50.0f;
            t->damping  = .9f;
            t->index = i;
            
            addParticle(t);
        }
        
        // NOTE: DELETED CONCEPT OF PARTICLESGRID + PARTICLES OUTLINE!!!!!
        
        lastDrawMode = DRAW_NULL;
        drawMode = DRAW_POINTS;
        
        ofDirectory meshDirectory;
        int numMeshes = meshDirectory.listDir(sourceDirectory);
        for (int i=0; i<numMeshes; i++){
            meshes[ meshDirectory.getName(i)].setup( meshDirectory.getPath(i), numParticles );
            cout << "LOADING "<<meshDirectory.getPath(i)<<endl;
        }
        
        currentMeshName = meshDirectory.getName(0);
        
        currentMeshBuffer->clear();
        currentMeshBuffer->append( meshes[currentMeshName].getMesh(0, drawMode, bUseGrid ? GRID_POINTS : GRID_OUTLINE) );
        
        bIsSetup = true;
    }
    
    while (isThreadRunning()){
        
        // some behaviors need everybody
        if ( currentBehavior != NULL ){
            currentBehavior->copySettings( behaviors[ MOVE_NONE ]);
            currentBehavior->updateAll(&_particles);
        }
        
        // FIRST!
        if ( bNeedToChangeMesh || lastDrawMode != drawMode ){
            
            lock();
            
//            if ( bUseGrid ){
//                _particles = _particlesGrid;
//            } else {
//                _particles = _particlesOutline;
//            }
            
            //TODO: THIS SHOULD BE JUST RESETTING TARGET VERTICES!!!!
            
            currentMeshBuffer->clear();
            currentMeshBuffer->append(  meshes[currentMeshName].getMesh(0, drawMode, bUseGrid ? GRID_POINTS : GRID_OUTLINE) );
            currentMeshBuffer->setMode( meshes[currentMeshName].getMesh(0, drawMode, bUseGrid ? GRID_POINTS : GRID_OUTLINE).getMode());
            
            for( it = _particles.begin(); it != _particles.end(); ++it )
            {
                TypeParticle * p = (TypeParticle*)(it->second);
                p->seedPosition = currentMeshBuffer->getVertex(p->index);
            }
            
            lastDrawMode = drawMode;
            
            if ( behaviors[ MOVE_FLOCK ]  != NULL ) ((Flocking*) behaviors[ MOVE_FLOCK ])->setLetters( &_particles );
            unlock();
        }
        
        // Colors
        if ( bNeedToChangeColor ){
            lock();
            ofFloatColor currentColor = particleColor;
            float currentVariance = colorVariance;
            
            for (int i=0; i<currentMeshBuffer->getNumColors(); i++){
                ofFloatColor localColor = currentColor;
                currentColor.setHue( localColor.getHue() + ofRandom(-currentVariance,currentVariance));
                currentMeshBuffer->setColor(i, localColor);
            }
            bNeedToChangeColor = false;
            if ( behaviors[ MOVE_FLOCK ] != NULL ) behaviors[ MOVE_FLOCK ]->copyMesh(currentMesh);
            unlock();
        }
        
        lock();
        ofxLabFlexParticleSystem::update();
        
        bMeshIsUpdated = false;
        
        // update mesh based on positionsIterator it;
        
        for( it = _particles.begin(); it != _particles.end(); ++it )
        {
            TypeParticle * p = (TypeParticle*)(it->second);
            
            if ( currentBehavior != NULL ){
                currentBehavior->update(p);
            }
            currentMeshBuffer->setVertex(p->index, *p);
        }
        
        bMeshIsUpdated = true;
        bNeedToChangeMesh = false;
        meshUpdatingFrames = 0;
        unlock();
        
        sleep(16.6667); // run at ~60fps
    }
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::update(){
    // leave the mesh alone until it's done
    if (!bMeshIsUpdated) return;
    
    lock();
    
    if ( currentMesh && currentMesh->getNumVertices() > 0 && behaviors[ MOVE_FLOCK ] == NULL){
        behaviors[ MOVE_FLOCK ] = new Flocking(&_particles);
        behaviors[ MOVE_FLOCK ]->copyMesh(currentMesh);
    }
    currentMesh->clear();
    currentMesh->append(*currentMeshBuffer);
    currentMesh->setMode(currentMeshBuffer->getMode());
    currentBehavior = behaviors[ moveType ];
    unlock();
    
    /*if ( bUseGrid ){
        currentMesh = &gridMesh;
    } else {
        currentMesh = &outlineMesh;
    }*/
    
    // update stuff based on grid
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::draw()
{
    if ( currentMesh && currentMesh->getNumVertices() > 0 ){
        
        if ( currentBehavior == NULL ){
            ofPushMatrix();
            ofTranslate(ofGetWidth()/2.0, ofGetHeight()/2.0);
            ofScale(scale, scale);
            ofTranslate(-ofGetWidth()/2.0, -ofGetHeight()/2.0);
            currentMesh->draw();
            ofPopMatrix();
        } else {
            ofSetColor(255);
            if ( currentMesh && currentMesh->getNumVertices() > 0 ){
                if ( currentBehavior->getName() != "flocking" ){
                    ofPushMatrix();
                    ofTranslate(ofGetWidth()/2.0, ofGetHeight()/2.0);
                    ofScale(scale, scale);
                    ofTranslate(-ofGetWidth()/2.0, -ofGetHeight()/2.0);
                }
                if ( currentBehavior != NULL ){
                    currentBehavior->scale = scale;
                    currentBehavior->beginDraw();
                }
                if ( currentBehavior->getName() != "flocking" ) currentMesh->draw();
                
                if ( currentBehavior != NULL ){
                    currentBehavior->endDraw();
                }
                if ( currentBehavior->getName() != "flocking" ){
                    ofPopMatrix();
                }
            }
        }
        
        if ( currentBehavior != NULL ){
            currentBehavior->draw();
        }
    }
    meshUpdatingFrames++;
    if ( meshUpdatingFrames > 10 ){
        ofSetColor(0,50);
        ofRect(0,0,ofGetWidth(), ofGetHeight());
        ofSetColor(255);
        string str = "LOADING";
        int n = abs(sin(ofGetElapsedTimeMillis() * .01)) * 4;
        for ( int i=0; i<n; i++){
            str += ".";
        }
        
        ofDrawBitmapString(str, ofGetWidth()/2.0, ofGetHeight()/2.0);
    }
}


//-------------------------------------------------------------------------------------------
DrawMode TypeParticleSystem::getDrawMode(){
    return drawMode;
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::setDrawMode( DrawMode mode )
{
    // ignoring if updating
    if (!bMeshIsUpdated) return;
    lastDrawMode = drawMode;
    drawMode = mode;
}

//-------------------------------------------------------------------------------------------
string TypeParticleSystem::getDrawModeString(){
    switch (drawMode) {
        case DRAW_NULL:
            return "NULL";
            break;
            
        case DRAW_POINTS:
            return "Points";
            break;
            
        case DRAW_LINES:
            return "Lines to letter point";
            break;
            
        case DRAW_LINES_RANDOMIZED:
            return "Lines Randomized";
            break;
            
        case DRAW_LINES_ARBITARY:
            return "Lines to point in space";
            break;
            
        case DRAW_SHAPES:
            return "Randomized shapes";
            break;
            
        default:
            return "Undefined";
            break;
    }
}

/********************************************************************************************
  ____  _____ _____ _____ ___ _   _  ____ ____
 / ___|| ____|_   _|_   _|_ _| \ | |/ ___/ ___|
 \___ \|  _|   | |   | |  | ||  \| | |  _\___ \
  ___) | |___  | |   | |  | || |\  | |_| |___) |
 |____/|_____| |_|   |_| |___|_| \_|\____|____/
 
 ********************************************************************************************/

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::setColor( ofFloatColor color, float variance ){
    particleColor = color;
    colorVariance = variance;
    
    bNeedToChangeColor = true;
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::setUseGrid( bool useGrid ){
    bool last = bUseGrid;
    bUseGrid = useGrid;
    if ( bUseGrid != last ){
        bNeedToChangeMesh = true;
    }
}

#pragma mark behaviors
/********************************************************************************************
  ____  ______ _    _     __      _______ ____  _____   _____
 |  _ \|  ____| |  | |   /\ \    / /_   _/ __ \|  __ \ / ____|
 | |_) | |__  | |__| |  /  \ \  / /  | || |  | | |__) | (___
 |  _ <|  __| |  __  | / /\ \ \/ /   | || |  | |  _  / \___ \
 | |_) | |____| |  | |/ ____ \  /   _| || |__| | | \ \ ____) |
 |____/|______|_|  |_/_/    \_\/   |_____\____/|_|  \_\_____/

********************************************************************************************/

//-------------------------------------------------------------------------------------------
Behavior * TypeParticleSystem::getCurrentBehavior(){
    return currentBehavior;
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::setBehavior( MovementType type ){
    moveType = type;
}

//-------------------------------------------------------------------------------------------
Behavior *  TypeParticleSystem::getSettingsBehavior(){
    return behaviors[ MOVE_NONE ];
}


#pragma mark interaction
/********************************************************************************************
 ___ _   _ _____ _____ ____      _    ____ _____ ___ ___  _   _
 |_ _| \ | |_   _| ____|  _ \    / \  / ___|_   _|_ _/ _ \| \ | |
  | ||  \| | | | |  _| | |_) |  / _ \| |     | |  | | | | |  \| |
  | || |\  | | | | |___|  _ <  / ___ \ |___  | |  | | |_| | |\  |
 |___|_| \_| |_| |_____|_| \_\/_/   \_\____| |_| |___\___/|_| \_|
 
 ********************************************************************************************/

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::mouseMoved( int x, int y ){
    if (!bMeshIsUpdated) return;
    
    ofVec3f mp = ofVec3f(x,y,0);
    float mouseMass = (lastMass * .9 + mp.distance(lastMouse) * .3);
    lastMouse = mp;
    lastMass = mouseMass;
    
    lock();
    if ( currentBehavior != NULL ){
        currentBehavior->setMouse( lastMouse );
    }
    
    for( it = _particles.begin(); it != _particles.end(); ++it )
    {
        TypeParticle * p = (TypeParticle *) it->second;
        float distance = ofDist(x, y, p->x, p->y);
        
        if( distance > 100 ) {
            continue;
        }
        
        // ofVec2f - ofVec2f
        ofVec3f diff = mp -( *p );
        
        diff.normalize();
        
        // http://www.grantchronicles.com/astro09.htm
        // this is a really lose interpretation.. like not very close
        float force = mouseMass * p->mass / MAX(1, distance);
        float accel = force / p->mass;
        
        p->acceleration += diff * accel;
    }
    unlock();
}