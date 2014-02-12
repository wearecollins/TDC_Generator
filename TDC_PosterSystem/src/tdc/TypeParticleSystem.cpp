//
//  TypeParticleSystem.cpp
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/6/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include "TypeParticleSystem.h"


//-------------------------------------------------------------------------------------------
TypeParticleSystem::~TypeParticleSystem(){
    waitForThread();
    stopThread();
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::setup( string file ){
    
    // setup ourselves based on window size
    setupSquare(ofVec2f(ofGetWidth(), ofGetHeight()));
    setOption(ofxLabFlexParticleSystem::HORIZONTAL_WRAP, false);
    setOption(ofxLabFlexParticleSystem::VERTICAL_WRAP, false);
    setOption( ofxLabFlexParticleSystem::DETECT_COLLISIONS, false);
    
    svgFile = file;
    
    // mouse interaction
    lastMass = 10.0f;
    meshUpdatingFrames = 0;
    bNeedToChangeMesh = false;
    bUseGrid = true;
    
    // behavior
    
    behaviors[ MOVE_NONE ] = NULL;
    behaviors[ MOVE_NOISE ] = new Noise();
    behaviors[ MOVE_WARP ] = new Warp();
    behaviors[ MOVE_FLOCK ] = NULL;
    
    //MOVE_FLOCK,
    //MOVE_GRAVITY,
    //MOVE_WARP
    
    // containers
    if ( bUseGrid ){
        currentTypeMesh = &grid;
        currentMesh = &gridMesh;
        currentMeshBuffer = &bufferGridMesh;
        currentLetterParticles = &letterGridParticles;
    } else {
        currentTypeMesh = &outline;
        currentMesh = &outlineMesh;
        currentMeshBuffer = &bufferOutlineMesh;
        currentLetterParticles = &letterOutlineParticles;
    }
    
    moveType = MOVE_NOISE;
    
    currentBehavior = behaviors[ moveType ];
    gravity  = ofVec2f(0,4.0f);
    
    // lez do it
    startThread();
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::threadedFunction(){
    static bool bIsSetup = false;
    
    bMeshIsUpdated = false;
    
    if (!bIsSetup){
        // setup grid
        grid.load(svgFile);
        outline.load(svgFile);
        
        ofMesh t_gridMesh     = grid.getMesh();
        ofMesh t_outlineMesh  = outline.getMesh();
        
        //drawMesh.setupIndicesAuto();
        
        ofColor color;
        color.setHue( ofMap( ofGetHours(), 0, 24, 0, 360.0f ));
        color.setSaturation(180.0f);
        
        int hour = ofGetHours();
        
#pragma mark TODO: ATTACH TO API!
        
        if ( hour < 9 || hour > 18 ){
            color.setBrightness(100);
        } else if ( hour > 9 && hour < 6 ){
            int diff = 12 - hour;
            color.setBrightness( ofMap(diff, 0, 6, 360, 100));
        }
        
        // set up letter containers
        for ( int i=0; i< grid.getNumLetters(); i++){
            letterGridParticles.push_back(vector<QuickVertex>());
            letterOutlineParticles.push_back(vector<QuickVertex>());
        }
        
        // build 10000 particles for grid
        for (int i=0; i<10000; i++){
            int index = (int) ofRandom(0, t_gridMesh.getNumVertices());
            while ( grid.isOccupied(index)){
                index = (int) ofRandom(0, t_gridMesh.getNumVertices());
            }
            int letter = grid.getLetterByParticle( index );
            
            QuickVertex qv;
            qv.pos      = t_gridMesh.getVertex(index);
            qv.index    = i;
            
            letterGridParticles[letter].push_back(qv);
            
            grid.occupyIndex(index);
            TypeParticle * t = new TypeParticle(t_gridMesh.getVertex(index));
            t->velocity = ofVec2f(ofRandom(-100,100));
            t->mass = 50.0f;
            t->damping  = .9f;
            
            addParticle(t);
            bufferGridMesh.addVertex(t_gridMesh.getVertex(index));
            ofColor localColor = color;
            color.setHue( color.getHue() + sin(i) * 50.0f );
            bufferGridMesh.addColor( localColor );
        }
        _particlesGrid = _particles;
        clear();
        
        // add 8000 particles for outline
        for (int i=0; i<7000; i++){
            int index = (int) ofRandom(0, t_outlineMesh.getNumVertices());
            while ( outline.isOccupied(index) && !outline.isFull() ){
                index = (int) ofRandom(0, t_outlineMesh.getNumVertices());
            }
            int letter = outline.getLetterByParticle( index );
            
            QuickVertex qv;
            qv.pos      = t_outlineMesh.getVertex(index);
            qv.index    = i;
            
            letterOutlineParticles[letter].push_back(qv);
            
            outline.occupyIndex(index);
            TypeParticle * t = new TypeParticle(t_outlineMesh.getVertex(index));
            t->velocity = ofVec2f(ofRandom(-100,100));
            t->mass = 50.0f;
            t->damping  = .9f;
            
            addParticle(t);
            bufferOutlineMesh.addVertex(t_outlineMesh.getVertex(index));
            ofColor localColor = color;
            color.setHue( color.getHue() + sin(i) * 50.0f );
            bufferOutlineMesh.addColor( localColor );
        }
        _particlesOutline = _particles;
        clear();
        
        lastDrawMode = DRAW_NULL;
        drawMode = DRAW_POINTS;
        bIsSetup = true;
    }
    
    while (isThreadRunning()){
        lock();
        
        // some behaviors need everybody
        if ( currentBehavior != NULL ){
            currentBehavior->updateAll(&_particles);
        }
        
        ofxLabFlexParticleSystem::update();
        
        bMeshIsUpdated = false;
        
        // FIRST!
        if ( bNeedToChangeMesh ){
            if ( bUseGrid ){
                currentTypeMesh = &grid;
                currentMeshBuffer = &bufferGridMesh;
                currentLetterParticles = &letterGridParticles;
                _particles = _particlesGrid;
            } else {
                currentTypeMesh = &outline;
                currentMeshBuffer = &bufferOutlineMesh;
                currentLetterParticles = &letterOutlineParticles;
                _particles = _particlesOutline;
            }
        }
        
        // update mesh based on positionsIterator it;
        int i=0;
        
        for( it = _particles.begin(); it != _particles.end(); ++it )
        {
            TypeParticle * p = (TypeParticle*)(it->second);
            
            if ( currentBehavior != NULL ){
                currentBehavior->update(p);
            }
            
            /*if( moveType == MOVE_NOISE ){
                
            } else if ( moveType == MOVE_FLOCK ){
                
            } else if ( moveType == MOVE_GRAVITY ){
                (*p).velocity += gravity;
            } else if ( moveType == MOVE_WARP ){
                //(*p) += ofVec2f( ofSignedNoise(p->seedPosition.x + ofGetElapsedTimef() * .1), ofSignedNoise(p->seedPosition.x + ofGetElapsedTimef() * .2) * 50.0f);
                //(*p) += ofVec2f( 0, sin(p->seedPosition.x + ofGetElapsedTimeMillis() * .2) * 10.0);
                (*p) += ofVec2f( 0, ofSignedNoise((p->seedPosition.x / ofGetWidth() * 10.0f) + ofGetElapsedTimeMillis() * .001) * 50.0);
            }*/
            
            currentMeshBuffer->setVertex(i, *it->second);
            i++;
        }
        
        if ( lastDrawMode != drawMode || bNeedToChangeMesh ){
            int i=0;
            vector<int> attach;
            
            switch (drawMode) {
                case DRAW_LINES:
                    currentMeshBuffer->setMode(OF_PRIMITIVE_LINES);
                    
                    currentMeshBuffer->clearIndices();
                    // method 1: randomize
                    if ( !bUseGrid ){
                        for (int j=0; j<currentLetterParticles->size(); j++){
                            for (int k=0; k<(*currentLetterParticles)[j].size(); k++){
                                currentMeshBuffer->addIndex((*currentLetterParticles)[j][k].index);
                                int ind = floor(ofRandom((*currentLetterParticles)[j].size()));
                                currentMeshBuffer->addIndex((*currentLetterParticles)[j][ind].index);
                            }
                        }
                    } else {
                        // method 2: attach to anywhere inside letter
                        
                        for (int j=0; j<currentLetterParticles->size(); j++){
                            QuickVertex qv = (*currentLetterParticles)[j][ofRandom((*currentLetterParticles)[j].size())];
                            attach.push_back(qv.index);
                        }
                        
                        for( it = _particles.begin(); it != _particles.end(); ++it )
                        {
                            int letterIndex = currentTypeMesh->getLetterByParticle(currentTypeMesh->getParticleIndex(((TypeParticle*)it->second)->getStart()));
                            currentMeshBuffer->addIndex(attach[letterIndex]);
                            currentMeshBuffer->addIndex(i);
                            
                            i++;
                        }
                    }
                    
                    break;
                    
                case DRAW_LINES_RANDOMIZED:
                    currentMeshBuffer->setMode(OF_PRIMITIVE_LINES);
                    
                    currentMeshBuffer->clearIndices();
                    
                {
                    vector<vector <QuickVertex> >  tempLetters;
                    for ( int j=0; j<currentLetterParticles->size(); j++){
                        tempLetters.push_back( vector<QuickVertex>() );
                        for ( int k=0; k<(*currentLetterParticles)[j].size(); k++ ){
                            tempLetters[j].push_back( (*currentLetterParticles)[j][k]);
                        }
                    }
                    
                    for( it = _particles.begin(); it != _particles.end(); ++it )
                    {
                        int letterIndex = currentTypeMesh->getLetterByParticle(currentTypeMesh->getParticleIndex(((TypeParticle*)it->second)->getStart()));
                        currentMeshBuffer->addIndex( i );
                        
                        int index = (int) ofRandom(tempLetters[letterIndex].size());
                        QuickVertex pt = tempLetters[letterIndex][index];
                        while (pt.pos == *it->second) {
                            index = (int) ofRandom(tempLetters[letterIndex].size());
                            pt = tempLetters[letterIndex][index];
                        }
                        
                        currentMeshBuffer->addIndex(pt.index);
                        tempLetters[letterIndex].erase(tempLetters[letterIndex].begin() + index);
                        
                        i++;
                    }
                }
                    break;
                    
                case DRAW_LINES_ARBITARY:
                    currentMeshBuffer->setMode(OF_PRIMITIVE_LINES);
                    
                    currentMeshBuffer->clearIndices();
                    
                {
                    int lpIndex = ofRandom(currentLetterParticles->size());
                    QuickVertex qv = (*currentLetterParticles)[lpIndex][ofRandom((*currentLetterParticles)[lpIndex].size())];
                    int attachIndex = qv.index;
                    ofColor color = currentMeshBuffer->getColor(attachIndex);
                    color.a = 0;
                    currentMeshBuffer->setColor(attachIndex, color);
                    
                    for( it = _particles.begin(); it != _particles.end(); ++it )
                    {
                        int letterIndex = currentTypeMesh->getLetterByParticle(currentTypeMesh->getParticleIndex(((TypeParticle*)it->second)->getStart()));
                        currentMeshBuffer->addIndex(attachIndex);
                        
                        int index = (int) ofRandom((*currentLetterParticles)[letterIndex].size());
                        QuickVertex pt = (*currentLetterParticles)[letterIndex][index];
                        while (pt.pos == *it->second) {
                            index = (int) ofRandom((*currentLetterParticles)[letterIndex].size());
                            pt = (*currentLetterParticles)[letterIndex][index];
                        }
                        
                        currentMeshBuffer->addIndex(pt.index);
                        
                        i++;
                    }
                }
                    break;
                    
                    
                case DRAW_POINTS:
                    currentMeshBuffer->setMode(OF_PRIMITIVE_POINTS);
                    currentMeshBuffer->clearIndices();
                    for( it = _particles.begin(); it != _particles.end(); ++it )
                    {
                        currentMeshBuffer->addIndex(i);
                        i++;
                    }
                    break;
                    
                case DRAW_SHAPES:
                    currentMeshBuffer->setMode(OF_PRIMITIVE_TRIANGLES);
                    // to-do: triangulation!
                    currentMeshBuffer->clearIndices();
                    
                    // method 1: randomize
                    for( it = _particles.begin(); it != _particles.end(); ++it )
                    {
                        currentMeshBuffer->addIndex(i);
                        Iterator it2;
                        int j=0;
                        for( it2 = _particles.begin(); it2 != _particles.end(); ++it2 )
                        {
                            if ( it2 != it ){
                                if ( it->second->distance( *it2->second ) < 10 ){
                                    currentMeshBuffer->addIndex(j);
                                    break;
                                }
                            }
                            j++;
                        }
                        
                        j=0;
                        
                        Iterator it3;
                        for( it3 = _particles.begin(); it3 != _particles.end(); ++it3 )
                        {
                            if ( it3 != it && it3 != it2 ){
                                if ( it->second->distance( *it3->second ) < 20 ){
                                    currentMeshBuffer->addIndex(j);
                                    break;
                                }
                            }
                            j++;
                        }
                        i++;
                    }
                    break;
                    
                default:
                    break;
            }
        }
        bMeshIsUpdated = true;
        bNeedToChangeMesh = false;
        meshUpdatingFrames = 0;
        lastDrawMode = drawMode;
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
    }
    gridMesh = bufferGridMesh;
    outlineMesh = bufferOutlineMesh;
    currentBehavior = behaviors[ moveType ];
    unlock();
    
    if ( bUseGrid ){
        currentMesh = &gridMesh;
    } else {
        currentMesh = &outlineMesh;
    }
    if ( currentMesh && currentMesh->getNumVertices() > 0 ){
        if ( currentBehavior != NULL ){
            currentBehavior->beginDraw();
        }
        //currentMesh->draw();
        
        if ( currentBehavior != NULL ){
            currentBehavior->endDraw();
        }
    }
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::draw()
{
    if ( currentMesh && currentMesh->getNumVertices() > 0 ){
        currentMesh->draw();
        if ( currentBehavior != NULL ){
            currentBehavior->draw();
        }
    }
    meshUpdatingFrames++;
    if ( meshUpdatingFrames > 3 ){
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

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::setUseGrid( bool useGrid ){
    bool last = bUseGrid;
    bUseGrid = useGrid;
    if ( bUseGrid != last ){
        bNeedToChangeMesh = true;
    }
}

//-------------------------------------------------------------------------------------------
Behavior * TypeParticleSystem::getCurrentBehavior(){
    return currentBehavior;
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::setBehavior( MovementType type ){
    moveType = type;
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::mouseMoved( int x, int y ){
    if (!bMeshIsUpdated) return;
    
    ofVec3f mp = ofVec3f(x,y,0);
    float mouseMass = (lastMass * .9 + mp.distance(lastMouse) * .8);
    lastMouse = mp;
    lastMass = mouseMass;
    
    lock();
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