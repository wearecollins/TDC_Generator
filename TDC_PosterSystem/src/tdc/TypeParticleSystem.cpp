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
        ofMesh gridMesh = grid.getMesh();
        
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
            letterParticles.push_back(vector<QuickVertex>());
        }
        
        // build 10000 particles
        for (int i=0; i<10000; i++){
            int index = (int) ofRandom(0, gridMesh.getNumVertices());
            while ( grid.isOccupied(index)){
                index = (int) ofRandom(0, gridMesh.getNumVertices());
            }
            int letter = grid.getLetterByParticle( index );
            
            QuickVertex qv;
            qv.pos      = gridMesh.getVertex(index);
            qv.index    = i;
            
            letterParticles[letter].push_back(qv);
            
            grid.occupyIndex(index);
            TypeParticle * t = new TypeParticle(gridMesh.getVertex(index));
            t->velocity = ofVec2f(ofRandom(-100,100));
            t->mass = 50.0f;
            t->damping  = .9f;
            
            addParticle(t);
            bufferMesh.addVertex(gridMesh.getVertex(index));
            ofColor localColor = color;
            color.setHue( color.getHue() + sin(i) * 50.0f );
            bufferMesh.addColor( localColor );
        }
        
        lastDrawMode = DRAW_NULL;
        drawMode = DRAW_POINTS;
        bIsSetup = true;
    }
    
    while (isThreadRunning()){
        lock();
        ofxLabFlexParticleSystem::update();
        
        bMeshIsUpdated = false;
        
        // update mesh based on positionsIterator it;
        int i=0;
        for( it = _particles.begin(); it != _particles.end(); ++it )
        {
            bufferMesh.setVertex(i, *it->second);
            i++;
        }
        
        if ( lastDrawMode != drawMode ){
            int i=0;
            vector<int> attach;
            
            switch (drawMode) {
                case DRAW_LINES:
                    bufferMesh.setMode(OF_PRIMITIVE_LINES);
                    
                    bufferMesh.clearIndices();
                    // method 1: randomize
                    /*for( it = _particles.begin(); it != _particles.end(); ++it )
                     {
                     bufferMesh.addIndex(i);
                     Iterator it2;
                     int j=0;
                     for( it2 = _particles.begin(); it2 != _particles.end(); ++it2 )
                     {
                     if ( it2 != it ){
                     if ( it->second->distance( *it2->second ) < 50 ){
                     bufferMesh.addIndex(j);
                     break;
                     }
                     }
                     j++;
                     }
                     i++;
                     }*/
                    
                    
                    // method 2: attach to anywhere inside letter
                    
                    for (int j=0; j<letterParticles.size(); j++){
                        QuickVertex qv = letterParticles[j][ofRandom(letterParticles[j].size())];
                        attach.push_back(qv.index);
                    }
                    
                    for( it = _particles.begin(); it != _particles.end(); ++it )
                    {
                        int letterIndex = grid.getLetterByParticle(grid.getParticleIndex(((TypeParticle*)it->second)->getStart()));
                        bufferMesh.addIndex(attach[letterIndex]);
                        bufferMesh.addIndex(i);
                        
                        i++;
                    }
                    
                    break;
                    
                case DRAW_LINES_RANDOMIZED:
                    bufferMesh.setMode(OF_PRIMITIVE_LINES);
                    
                    bufferMesh.clearIndices();
                    
                {
                    vector<vector <QuickVertex> >  tempLetters;
                    for ( int j=0; j<letterParticles.size(); j++){
                        tempLetters.push_back( vector<QuickVertex>() );
                        for ( int k=0; k<letterParticles[j].size(); k++ ){
                            tempLetters[j].push_back( letterParticles[j][k]);
                        }
                    }
                    
                    for( it = _particles.begin(); it != _particles.end(); ++it )
                    {
                        int letterIndex = grid.getLetterByParticle(grid.getParticleIndex(((TypeParticle*)it->second)->getStart()));
                        bufferMesh.addIndex( i );
                        
                        int index = (int) ofRandom(tempLetters[letterIndex].size());
                        QuickVertex pt = tempLetters[letterIndex][index];
                        while (pt.pos == *it->second) {
                            index = (int) ofRandom(tempLetters[letterIndex].size());
                            pt = tempLetters[letterIndex][index];
                        }
                        
                        bufferMesh.addIndex(pt.index);
                        tempLetters[letterIndex].erase(tempLetters[letterIndex].begin() + index);
                        
                        i++;
                    }
                }
                    break;
                    
                case DRAW_LINES_ARBITARY:
                    bufferMesh.setMode(OF_PRIMITIVE_LINES);
                    
                    bufferMesh.clearIndices();
                    
                {
                    int lpIndex = ofRandom(letterParticles.size());
                    QuickVertex qv = letterParticles[lpIndex][ofRandom(letterParticles[lpIndex].size())];
                    int attachIndex = qv.index;
                    ofColor color = bufferMesh.getColor(attachIndex);
                    color.a = 0;
                    bufferMesh.setColor(attachIndex, color);
                    
                    for( it = _particles.begin(); it != _particles.end(); ++it )
                    {
                        int letterIndex = grid.getLetterByParticle(grid.getParticleIndex(((TypeParticle*)it->second)->getStart()));
                        bufferMesh.addIndex(attachIndex);
                        
                        int index = (int) ofRandom(letterParticles[letterIndex].size());
                        QuickVertex pt = letterParticles[letterIndex][index];
                        while (pt.pos == *it->second) {
                            index = (int) ofRandom(letterParticles[letterIndex].size());
                            pt = letterParticles[letterIndex][index];
                        }
                        
                        bufferMesh.addIndex(pt.index);
                        
                        i++;
                    }
                }
                    break;
                    
                    
                case DRAW_POINTS:
                    bufferMesh.setMode(OF_PRIMITIVE_POINTS);
                    bufferMesh.clearIndices();
                    for( it = _particles.begin(); it != _particles.end(); ++it )
                    {
                        bufferMesh.addIndex(i);
                        i++;
                    }
                    break;
                    
                case DRAW_SHAPES:
                    bufferMesh.setMode(OF_PRIMITIVE_TRIANGLES);
                    // to-do: triangulation!
                    bufferMesh.clearIndices();
                    
                    // method 1: randomize
                    for( it = _particles.begin(); it != _particles.end(); ++it )
                    {
                        bufferMesh.addIndex(i);
                        Iterator it2;
                        int j=0;
                        for( it2 = _particles.begin(); it2 != _particles.end(); ++it2 )
                        {
                            if ( it2 != it ){
                                if ( it->second->distance( *it2->second ) < 10 ){
                                    bufferMesh.addIndex(j);
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
                                    bufferMesh.addIndex(j);
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
    drawMesh = bufferMesh;
    unlock();
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::draw()
{
    drawMesh.draw();
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
void TypeParticleSystem::mouseMoved( int x, int y ){
    if (!bMeshIsUpdated) return;
    
    ofVec3f mp = ofVec3f(x,y,0);
    float mouseMass = (lastMass * .9 + mp.distance(lastMouse) * 2.0f * .8);
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